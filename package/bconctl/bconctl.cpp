/** ----------------------------------------------------------------------------
 *
 * Basler dart BCON for LVDS Development Kit
 * http://www.baslerweb.com
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2017-2018, Basler AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * -----------------------------------------------------------------------------
 *
 * @file    bconctl.cpp
 *
 * @brief   Implementation of bconctl
 *
 * @author  Björn Rennfanz
 *
 * @date    15.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <errno.h>
#include <basler/bconctl.h>

#include <cstdio>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>

#include "bconctlCommandLineParser.h"
#include "bconctlParameters.h"
#include "bconctlVersion.h"

using namespace Utils;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
void printHelp()
{
    CBconCtlCommandLineParser parser;
    std::string helpText;

    cout << "Usage:" << std::endl;
    cout << "  bconctl <command> <subcommand> [options]" << std::endl;
    cout << std::endl;
    cout << "Commands:" << std::endl;

    parser.ListAllCommands(helpText);
    cout << helpText << std::endl;

    cout << "Use 'bconctl <command> -h' to show help text for specific command." << std::endl;
    cout << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
//
void printVersionLine()
{
    cout << "bconctl, " << BCONCTL_VERSION_STRING << std::endl;
    cout << "Basler dart BCON for LVDS Development Kit Control Utility" << std::endl;
    cout << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
//
void throwRuntimeException(std::string ownErrorMessage, int result)
{
    // Get error message from "errno" and throw as runtime_error
    char const * strErrorMessage = strerror(errno);

    // Assemble error message
    std::stringstream stringStream;
    stringStream << ownErrorMessage << " (Result code: " << result << ", " << strErrorMessage << ").";

    // Throw a runtime exception
    throw std::runtime_error(stringStream.str());
}

///////////////////////////////////////////////////////////////////////////////
//
void throwInvalidArgumentException(std::string ownErrorMessage)
{
    // Throw a runtime exception
    throw std::invalid_argument(ownErrorMessage);
}

///////////////////////////////////////////////////////////////////////////////
//
void runLEDCmd(CBconCtlParameters parameters, bconctl_led_user_t slct_user_led)
{
    int result = 0;
    if (parameters.isOnCmd)
    {
        // Enable selected LEDx
        result = bconctl_board_led_on(slct_user_led);
        if (result < 0)
        {
            throwRuntimeException("Failed to set LED state", result);
        }
    }
    else if (parameters.isOffCmd)
    {
        // Disable selected LEDx
        result = bconctl_board_led_off(slct_user_led);
        if (result < 0)
        {
            throwRuntimeException("Failed to set LED state", result);
        }
    }
    else
    {
        // Get status of selected LEDx
        result = bconctl_board_led_status(slct_user_led);
        if (result < 0)
        {
            throwRuntimeException("Failed to get LED state", result);
        }

        // Generate output message
        if (!parameters.isQuiet)
        {
            cout << "User LED" << slct_user_led << " is currently " << (result > 0 ? "on" : "off") << "." << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void runI2cIdCmd(CBconCtlParameters parameters)
{
    int result = 0;
    if (parameters.hasI2cIdParams())
    {
        // Convert string to i2c_id
        char *pEnd;
        int i2c_id = static_cast<int>(strtol(parameters.i2cIdParams.c_str(), &pEnd, 10));
        if (*pEnd != 0)
        {
            throwInvalidArgumentException("Invalid parameter for 'i2c_id'");
        }

        result = bconctl_camera_address_select(i2c_id);
        if (result < 0)
        {
            throwRuntimeException("Failed to switch camera address", result);
        }
    }
    else
    {
        result = bconctl_camera_address_status();
        if (result < 0)
        {
            throwRuntimeException("Failed to get camera address", result);
        }

        // Generate output message
        if (!parameters.isQuiet)
        {
            cout << "The value of the address select pin (i2c_id) is " << result << "." << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void runPowerCmd(CBconCtlParameters parameters)
{
    int result = 0;
    if (parameters.isOnCmd)
    {
        // Enable selected LEDx
        result = bconctl_camera_power_on();
        if (result < 0)
        {
            throwRuntimeException("Failed to set camera power state", result);
        }
    }
    else if (parameters.isOffCmd)
    {
        // Disable selected LEDx
        result = bconctl_camera_power_off();
        if (result < 0)
        {
            throwRuntimeException("Failed to set camera power state", result);
        }
    }
    else
    {
        // Get status of selected LEDx
        result = bconctl_camera_power_status();
        if (result < 0)
        {
            throwRuntimeException("Failed to get camera power state", result);
        }

        // Generate output message
        if (!parameters.isQuiet)
        {
            cout << "Camera power is currently " << (result > 0 ? "on" : "off") << "." << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void runResetCmd(CBconCtlParameters parameters)
{
    // Reset all BCON cameras
    int result = bconctl_camera_reset();
    if (result < 0)
    {
        throwRuntimeException("Failed to reset camera", result);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void runTrggenCmd(CBconCtlParameters parameters)
{
    int result = 0;
    unsigned int period = 0, duration = 0;

    if (parameters.hasPulseParams())
    {
        if (2 != sscanf(parameters.pulseParams.c_str(), "%u:%u", &period, &duration))
        {
            throwInvalidArgumentException("Failed to decode pulse parameters");
        }
    }

    // Open trigger generator
    const bconctl_trggen_ctx_t *ctx = bconctl_trggen_open();
    if (!ctx)
    {
        throwRuntimeException("Failed to access trigger generator", -1);
    }

    if (parameters.hasPulseParams())
    {
        // Get maximum and minimum period
        const unsigned int period_min = bconctl_trggen_get_minimum_pulse_period_ms(ctx);
        const unsigned int period_max = bconctl_trggen_get_maximum_pulse_period_ms(ctx);

        // Check range of given period
        if (period < period_min || period > period_max)
        {
            // Close trigger generator
            bconctl_trggen_close(const_cast<bconctl_trggen_ctx_t *>(ctx));

            // Assemble error message
            std::stringstream stringStream;
            stringStream << "Requested period invalid - must be in range [" << period_min << ".." << period_max << "]";

            // Throw invalid argument exception
            throwInvalidArgumentException(stringStream.str());
        }

        // Check period is greater than duration
        if (duration >= period)
        {
            // Close trigger generator
            bconctl_trggen_close(const_cast<bconctl_trggen_ctx_t *>(ctx));

            // Throw invalid argument exception
            throwInvalidArgumentException("Pulse duration must be less than period");
        }

        result = bconctl_trggen_set_pulse(ctx, period, duration);
        if (result < 0)
        {
            // Close trigger generator
            bconctl_trggen_close(const_cast<bconctl_trggen_ctx_t *>(ctx));

            // Throw runtime exception
            throwRuntimeException("Failed to set pulse period/duration", result);
        }
    }

    if (parameters.isOnCmd)
    {
        // Start trigger generator
        result = bconctl_trggen_start(ctx);
        if (result < 0)
        {
            throwRuntimeException("Failed to start trigger generator", result);
        }
    }
    else if (parameters.isOffCmd)
    {
        // Stop trigger generator
        result = bconctl_trggen_stop(ctx);
        if (result < 0)
        {
            throwRuntimeException("Failed to stop trigger generator", result);
        }
    }
    else
    {
        // Get status of trigger generator
        result = bconctl_trggen_status(ctx);
        if (result < 0)
        {
            throwRuntimeException("Failed to get trigger generator status", result);
        }

        // Convert result to enabled/disabled string
        std::string triggerState((result > 0 ? "enabled" : "disabled"));

        // Get current timing parameters of trigger generator
        result = bconctl_trggen_get_pulse(ctx, &period, &duration);
        if (result < 0)
        {
            throwRuntimeException("Failed to get trigger generator parameters", result);
        }

        // Generate output message
        if (!parameters.isQuiet)
        {
            // Assemble console message
            std::stringstream stringStream;
            stringStream << "Trigger generation is " << triggerState << ", period is set to " << period << " ms with duration (signal high) of " << duration << " ms.";

            // Output message string
            cout << stringStream.str() << std::endl;
        }
    }

    // Close trigger generator
    bconctl_trggen_close(const_cast<bconctl_trggen_ctx_t *>(ctx));
}

///////////////////////////////////////////////////////////////////////////////
//
int main(int argc, const char* argv[])
{
    try
    {
        CBconCtlCommandLineParser parser;
        CBconCtlParameters parameters;

        // Parse and check args passed on command line
        bool argvParsed = parser.Parse(argc, argv);
        if (!parser.IsOption(BconCtlOption_NoLogo) && !parser.IsOption(BconCtlOption_Quiet))
        {
            printVersionLine();
        }

        if (!argvParsed)
        {
            // Invalid args
            const CBconCtlCommandLineParser::ErrorInfo& errorInfo = parser.GetErrorInfo();
            std::string errorText = "Error: " + errorInfo.errorDescription + " '" + errorInfo.errorArg +"'";
            cerr << errorText << std::endl;

            // If the command was parsed, print command help
            if (BconCtlErrorState_UnknownCommand != errorInfo.errorState && BconCtlCommand_Unknown != parser.GetCommand())
            {
                // Print how to do it right
                cout << "Usage:";

                std::string helpText;
                parser.GetHelpText(parser.GetCommand(), helpText);
                cout << helpText << std::endl;
            }
            else
            {
                // Print general help
                printHelp();
            }

            return 1;
        }

        // Common options
        parameters.isQuiet = parser.IsOption(BconCtlOption_Quiet);
        parameters.isHelp = parser.IsOption(BconCtlOption_Help);

        // Additional parameters
        parser.GetOptionParameter(BconCtlOption_Pulse, parameters.pulseParams);

        // Subcommand actions
        bool hasI2cIdCmd = parser.GetSubCommandParameter(BconCtlSubCommand_I2CId, parameters.i2cIdParams);
        parameters.isOnCmd = parser.IsSubCommand(BconCtlSubCommand_On);
        parameters.isOffCmd = parser.IsSubCommand(BconCtlSubCommand_Off);

        EBconCtlCommand command = parser.GetCommand();

        // Print OK at end of command
        bool printOk = parameters.isOnCmd
                    || parameters.isOffCmd
                    || parameters.hasI2cIdParams()
                    || parameters.hasPulseParams()
                    || BconCtlCommand_Reset == command;

        if (parameters.isHelp)
        {
            cout << "Usage:";

            std::string helpText;
            parser.GetHelpText(parser.GetCommand(), helpText);
            cout << helpText << std::endl;

            return 0;
        }

        switch (command)
        {
            case BconCtlCommand_Config:
            {
                if (hasI2cIdCmd)
                {
                    runI2cIdCmd(parameters);
                }
                else
                {
                    // Should have been checked by the parser
                    cerr << "Error: config command was called without subcommand." << std::endl;
                    if (!parameters.isQuiet)
                    {
                        // Print how to do it right
                        cout << "Usage:";

                        std::string helpText;
                        parser.GetHelpText(parser.GetCommand(), helpText);
                        cout << helpText << std::endl;
                    }

                    return 1;
                }
                break;
            }

            case BconCtlCommand_Led0:
            case BconCtlCommand_Led1:
            case BconCtlCommand_Led2:
            {
                // Convert BconCtlCommand_Ledx to bconctl_led_user_t
                bconctl_led_user_t slct_user_led = static_cast<bconctl_led_user_t>(parser.GetCommand() - BconCtlCommand_Led0);
                runLEDCmd(parameters, slct_user_led);
                break;
            }

            case BconCtlCommand_Power:
            {
                runPowerCmd(parameters);
                break;
            }

            case BconCtlCommand_Reset:
            {
                runResetCmd(parameters);
                break;
            }

            case BconCtlCommand_Trggen:
            {
                runTrggenCmd(parameters);
                break;
            }

            case BconCtlCommand_Version:
            {
                cout << "License BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>" << std::endl;
                cout << "Copyright (C) 2017 Basler AG" << std::endl;
                break;
            }

            case BconCtlCommand_Help:
                // fallback
            default:
            {
                printHelp();
            }
        }

        // Output acknowledge
        if (!parameters.isQuiet && printOk)
        {
            cout << "OK" << std::endl;
        }
    }
    catch(std::invalid_argument &e)
    {
        cerr <<  "Error: " << e.what() << "." << std::endl;
        return 1;
    }
    catch(std::exception &e)
    {
        cerr <<  "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        cerr << "An unknown exception occurred." << std::endl;
        return 1;
    }

    return 0;
}
