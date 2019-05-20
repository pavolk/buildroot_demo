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
 * @file    bconctlCommandLineParser.cpp
 *
 * @brief   Implementation of bconctl command line parser
 *
 * @author  Björn Rennfanz
 *
 * @date    14.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <stddef.h>
#include <limits.h>

#include <cassert>
#include <cstring>
#include <algorithm>

#include "bconctlCommandLineParser.h"

namespace Utils
{
    const char CBconCtlCommandLineParser::longPrefix[2] = { '-', '-' };
    const char CBconCtlCommandLineParser::shortPrefix[1] = { '-' };

    ///////////////////////////////////////////////////////////////////////////
    //
    const CBconCtlCommandLineParser::OptionDescription CBconCtlCommandLineParser::m_optionDescriptions[cNumberOfKnownOptions] =
    {
          // Long                Short   Enum                        NumAddlArgs     helpText
          { "help",             "h",    BconCtlOption_Help,         0,              "  -h | --help                     Show help text of command\n" }
        , { "no-logo",          NULL,   BconCtlOption_NoLogo,       0,              "  --no-logo                       Suppress display of program version banner\n" }
        , { "quiet",            "q",    BconCtlOption_Quiet,        0,              "  -q | --quiet                    Suppress console output\n" }
        , { "version",          NULL,   BconCtlOption_Version,      0,              "" } // "--version" is not a true option, but invokes as first and only argument the command "version"
        , { "pulse",            "p",    BconCtlOption_Pulse,        1,              "  -p | --pulse <period:duration>  Set the overall period and the signal high time (duration)\n"
                                                                                    "                                  of generated signal in milliseconds\n" }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    const CBconCtlCommandLineParser::SubCommandDescription CBconCtlCommandLineParser::m_subCommandDescriptions[cNumberOfKnownSubCommands] =
    {
          // subcommand name    Enum                                NumAddlArgs     helpText
          { "on",               BconCtlSubCommand_On,               0,              "  on                              Activates current command\n" }
        , { "off",              BconCtlSubCommand_Off,              0,              "  off                             Deactivates current command\n" }
        , { "i2c_id",           BconCtlSubCommand_I2CId,            1,              "  i2c_id                          Configures the i2c_id pin of the camera\n" }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    const CBconCtlCommandLineParser::CommandDescription CBconCtlCommandLineParser::m_commandDescriptions[cNumberOfKnownCommands] =
    {
        // command name
        // command enum
        // allowed subcommands
        // allowed options
        // help text

        {
            "version",
            BconCtlCommand_Version,
            { BconCtlSubCommand_Unknown },
            { BconCtlOption_Help },
            "\n"
            "  bconctl version [-h]\n"
            "\n"
            "Show version of bconctl.\n"
        }
        ,
        {
            "help",
            BconCtlCommand_Help,
            { BconCtlSubCommand_Unknown },
            { BconCtlOption_Help },
            "\n"
            "  bconctl help [-h]\n"
            "\n"
            "Show help text.\n"
        }
        ,
        {
            "config",
            BconCtlCommand_Config,
            { BconCtlSubCommand_I2CId },
            { BconCtlOption_Help, BconCtlOption_NoLogo, BconCtlOption_Quiet },
            "\n"
            "  bconctl config [i2c_id [0|1]] [-h] [--no-logo] [-q]\n"
            "\n"
            "Get or set a value for the config parameter.\n"
        }
        ,
        {
            "led0",
            BconCtlCommand_Led0,
            { BconCtlSubCommand_On, BconCtlSubCommand_Off },
            { BconCtlOption_Help, BconCtlOption_NoLogo, BconCtlOption_Quiet },
            "\n"
            "  bconctl led0 [on] [off] [-h] [--no-logo] [-q]\n"
            "\n"
            "Enable or disable LED 0 on carrier card.\n"
        }
        ,
        {
            "led1",
            BconCtlCommand_Led1,
            { BconCtlSubCommand_On, BconCtlSubCommand_Off },
            { BconCtlOption_Help, BconCtlOption_NoLogo, BconCtlOption_Quiet },
            "\n"
            "  bconctl led1 [on] [off] [-h] [--no-logo] [-q]\n"
            "\n"
            "Enable or disable LED 1 on carrier card.\n"
        }
        ,
        {
            "led2",
            BconCtlCommand_Led2,
            { BconCtlSubCommand_On, BconCtlSubCommand_Off },
            { BconCtlOption_Help, BconCtlOption_NoLogo, BconCtlOption_Quiet },
            "\n"
            "  bconctl led2 [on] [off] [-h] [--no-logo] [-q]\n"
            "\n"
            "Enable or disable LED 2 on carrier card.\n"
        }
        ,
        {
            "power",
            BconCtlCommand_Power,
            { BconCtlSubCommand_On, BconCtlSubCommand_Off },
            { BconCtlOption_Help, BconCtlOption_NoLogo, BconCtlOption_Quiet },
            "\n"
            "  bconctl power [on] [off] [-h] [--no-logo] [-q]\n"
            "\n"
            "Enable or disable the power supply for BCON camera.\n"
        }
        ,
        {
            "reset",
            BconCtlCommand_Reset,
            { BconCtlSubCommand_Unknown },
            { BconCtlOption_Help },
            "\n"
            "  bconctl reset [-h]\n"
            "\n"
            "Reset all BCON cameras by toggling address select pin.\n"
        }
        ,
        {
            "trggen",
            BconCtlCommand_Trggen,
            { BconCtlSubCommand_On, BconCtlSubCommand_Off },
            { BconCtlOption_Help, BconCtlOption_NoLogo, BconCtlOption_Quiet, BconCtlOption_Pulse },
            "\n"
            "  bconctl trggen [on] [off] [-p <period:duration>] [-h] [--no-logo] [-q]\n"
            "\n"
            "Enable or disable the trigger generator module.\n"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    const CBconCtlCommandLineParser::ErrorDescription CBconCtlCommandLineParser::m_errorDescriptions[cNumberOfErrorDescriptions]=
    {
          { BconCtlErrorState_OptionParamMissing,     "parameter missing for option" }
        , { BconCtlErrorState_UnknownOption,          "unknown option" }
        , { BconCtlErrorState_UnknownCommand,         "unknown command" }
        , { BconCtlErrorState_UnknownSubCommand,      "unknown subcommand" }
    };

    bool CBconCtlCommandLineParser::ErrorInfo::set(EBconCtlErrorState state, const char* arg)
    {
        if (BconCtlErrorState_OK == state)
        {
            assert(NULL == arg && "There is no valid error argument for error state BconCtlErrorState_OK.");

            // Reset everything
            clear();
            return true;
        }

        for (unsigned int j = 0; j < UTILS_COUNTOF(m_errorDescriptions); ++j)
        {
            if (state == m_errorDescriptions[j].enumVal)
            {
                clear();
                errorState = state;
                errorDescription = m_errorDescriptions[j].string;
                errorArg = arg;
                return true;
            }
        }

        assert(false && "missing description for error state");
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::ParseCommandLine(int argc, const char* argv[])
    {
        Reset();

        assert(BconCtlCommand_Unknown == m_command && "m_command should be reset to BconCtlCommand_Unknown before parsing starts.");

        if (1 == argc)
        {
            // No arguments to parse
            m_command = BconCtlCommand_Help;
            return true;
        }

        int i = 1;
        bool knownCommandFound = false;

        while (i < argc)
        {
            const char* currentArg = argv[i];
            size_t optionPrefixLength = 0;
            EBconCtlOptionForm optionForm = GetOptionForm(currentArg, optionPrefixLength);
            if (OptionForm_Short == optionForm || OptionForm_Long == optionForm)
            {
                assert(optionPrefixLength > 0 && "If the option form is valid, prefix length must be >0.");

                // We have an option
                // Which one?
                const char* optionNameBegin = currentArg + optionPrefixLength;
                bool knownOptionFound = false;
                for (unsigned int j = 0; j < UTILS_COUNTOF(m_optionDescriptions); ++j)
                {
                    const OptionDescription& currentOptionDescription = m_optionDescriptions[j];
                    assert(currentOptionDescription.enumVal != BconCtlOption_Unknown && "Invalid enumValue in m_optionDescriptions[j]");
                    if (optionPrefixLength == OptionForm_Short && NULL != currentOptionDescription.stringShort)
                    {
                        knownOptionFound = IsEqualStringIgnoreCase(optionNameBegin, currentOptionDescription.stringShort);
                    }
                    else if (optionPrefixLength == OptionForm_Long && NULL != currentOptionDescription.stringLong)
                    {
                        knownOptionFound = IsEqualStringIgnoreCase(optionNameBegin, currentOptionDescription.stringLong);
                    }

                    if (knownOptionFound)
                    {
                        assert(currentOptionDescription.additionalOptionParamsFollowing >= 0 && currentOptionDescription.additionalOptionParamsFollowing < 2 && "Currently, only 0 or 1 additional option parameters supported.");

                        Option option = { currentOptionDescription.enumVal, optionForm, std::string() }; // Explicit string init for improved readability

                        if (1 == currentOptionDescription.additionalOptionParamsFollowing) // Currently, only 0 or 1 additional option parameters supported
                        {
                            // Parse the next arg which must be the required option parameter
                            ++i;

                            if (i >= argc)
                            {
                                // Error: required option parameter missing
                                m_errorInfo.set(BconCtlErrorState_OptionParamMissing, currentArg);
                                return false;
                            }

                            option.parameter = argv[i];
                        }

                        // Check if option is already set
                        // Then overwrite it
                        bool optionOverwritten = false;
                        for (std::vector<Option>::iterator it = m_options.begin(); it != m_options.end(); ++it)
                        {
                            if (it->enumVal == option.enumVal)
                            {
                                *it = option;
                                optionOverwritten = true;
                                break;
                            }
                        }

                        if (!optionOverwritten)
                        {
                            m_options.push_back(option);
                        }

                        break;
                    }
                }

                if (!knownOptionFound)
                {
                    // Error: unknown option; stop parsing
                    m_errorInfo.set(BconCtlErrorState_UnknownOption, currentArg);
                    return false;
                }

            }
            // currentArg is not an option;
            // is it a command?
            else if (!knownCommandFound)
            {
                // Any non option at position 1 is a command
                m_command = GetBconCtlCommand(currentArg);
                if (BconCtlCommand_Unknown == m_command)
                {
                    // Error: unknown command; stop parsing
                    m_errorInfo.set(BconCtlErrorState_UnknownCommand, currentArg);
                    return false;
                }

                knownCommandFound = true;
            }
            // currentArg is not an option or command;
            // Is it a subcommand?
            else
            {
                bool knownSubCommandFound = false;
                for (unsigned int j = 0; j < UTILS_COUNTOF(m_subCommandDescriptions); ++j)
                {
                    const SubCommandDescription& currentSubCommandDescription = m_subCommandDescriptions[j];
                    assert(currentSubCommandDescription.enumVal != BconCtlSubCommand_Unknown && "Invalid enumValue in m_subCommandDescriptions[j]");

                    knownSubCommandFound = IsEqualStringIgnoreCase(currentArg, currentSubCommandDescription.string);

                    if (knownSubCommandFound)
                    {
                        assert(currentSubCommandDescription.additionalSubCommandParamsFollowing >= 0 && currentSubCommandDescription.additionalSubCommandParamsFollowing < 2 && "Currently, only 0 or 1 additional subcommand parameters supported.");

                        m_subCommand.enumVal = currentSubCommandDescription.enumVal;

                        if (1 == currentSubCommandDescription.additionalSubCommandParamsFollowing) // Currently, only 0 or 1 additional subcommand parameters supported
                        {
                            // Parse the next arg which must be the required subcommand parameter
                            ++i;

                            if (i < argc)
                            {
                                m_subCommand.parameter = argv[i];
                            }
                        }

                        break;
                    }
                }

                if (!knownSubCommandFound)
                {
                    // Error: unknown option; stop parsing
                    m_errorInfo.set(BconCtlErrorState_UnknownSubCommand, currentArg);
                    return false;
                }
            }

            ++i;
        }

        // Check for help or version option with no command
        if (BconCtlCommand_Unknown == m_command)
        {
            for (std::vector<Option>::iterator it = m_options.begin(); it != m_options.end(); ++it)
            {
                // --version at position 1 is equivalent to command version
                // -h or --help at position 1 is equivalent to command help
                if (BconCtlOption_Help == it->enumVal || BconCtlOption_Version == it->enumVal)
                {
                    m_command = (BconCtlOption_Version == it->enumVal) ? BconCtlCommand_Version : BconCtlCommand_Help;
                    it = m_options.erase(it);
                    break;
                }
            }
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::EvaluateParsedData()
    {
        // Check parsed data for consistency

        assert(BconCtlCommand_Unknown != m_command && "At this point we must have a known command.");

        // Find command description
        for (unsigned int j = 0; j < UTILS_COUNTOF(m_commandDescriptions); ++j)
        {
            const CommandDescription& currentCommandDescription = m_commandDescriptions[j];
            assert(currentCommandDescription.enumVal != BconCtlCommand_Unknown && "Invalid enumValue in m_commandDescriptions[j]");

            if (m_commandDescriptions[j].enumVal == m_command)
            {
                // Found command description

                // Check if set subcommands are valid for this command
                if (m_subCommand.enumVal != BconCtlSubCommand_Unknown)
                {
                    const EBconCtlSubCommand* pCurSubCommandEnumVal = std::find(currentCommandDescription.knownSubCommands, currentCommandDescription.knownSubCommands + UTILS_COUNTOF(currentCommandDescription.knownSubCommands), m_subCommand.enumVal);
                    if (pCurSubCommandEnumVal == currentCommandDescription.knownSubCommands + UTILS_COUNTOF(currentCommandDescription.knownSubCommands))
                    {
                            // Error: subcommand unknown to this command
                            std::string subCommandName;
                            bool subCommandNameOK = GetSubCommandName(m_subCommand, subCommandName);
                            assert(subCommandNameOK && "Getting subcommand string failed"); UTILS_UNUSED(subCommandNameOK);
                            m_errorInfo.set(BconCtlErrorState_UnknownSubCommand, subCommandName.c_str());
                            return false;
                    }
                }

                // Check if set options are valid for this command
                for (std::vector<Option>::iterator it = m_options.begin(); it != m_options.end(); ++it)
                {
                    const Option& currentOption = *it;
                    const EBconCtlOption* pCurOptionEnumVal = std::find(currentCommandDescription.knownCommandOptions, currentCommandDescription.knownCommandOptions + UTILS_COUNTOF(currentCommandDescription.knownCommandOptions), currentOption.enumVal);
                    if (pCurOptionEnumVal == currentCommandDescription.knownCommandOptions + UTILS_COUNTOF(currentCommandDescription.knownCommandOptions))
                    {
                        // Error: option unknown to this command
                        std::string optionWithPrefix;
                        bool optionWithPrefixOK = GetOptionWithPrefix(currentOption, optionWithPrefix);
                        assert(optionWithPrefixOK && "Getting option with prefix failed"); UTILS_UNUSED(optionWithPrefixOK);
                        m_errorInfo.set(BconCtlErrorState_UnknownOption, optionWithPrefix.c_str());
                        return false;
                    }
                }

                break;
            }
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::IsOption(EBconCtlOption option) const
    {
        for (std::vector<Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it)
        {
            if (option == it->enumVal)
            {
                return true;
            }
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::GetOptionParameter(EBconCtlOption option, std::string& parameter) const
    {
        // The parameter
        for (std::vector<Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it)
        {
            if (option == it->enumVal)
            {
                parameter = it->parameter;// may be empty
                return true;
            }
        }

        // Option is not in m_options
        return false;
    }

    ////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::IsSubCommand(EBconCtlSubCommand subCommand) const
    {
        if (subCommand == m_subCommand.enumVal)
        {
            return true;
        }

        return false;
    }

    ////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::GetSubCommandParameter(EBconCtlSubCommand subCommand, std::string& parameter) const
    {
        if (subCommand == m_subCommand.enumVal)
        {
            parameter = m_subCommand.parameter; // may be empty
            return true;
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    CBconCtlCommandLineParser::EBconCtlOptionForm CBconCtlCommandLineParser::GetOptionForm(const char* arg, size_t& prefixSize) const
    {
        size_t len = strlen(arg);
        if (len > UTILS_COUNTOF(longPrefix))
        {
            if (0 == strncmp(arg, longPrefix, UTILS_COUNTOF(longPrefix)))
            {
                prefixSize = UTILS_COUNTOF(longPrefix);
                return OptionForm_Long;
            }
        }

        if (len > UTILS_COUNTOF(shortPrefix))
        {
            if (0 == strncmp(arg, shortPrefix, UTILS_COUNTOF(shortPrefix)))
            {
                prefixSize = UTILS_COUNTOF(shortPrefix);
                return OptionForm_Short;
            }
        }

        prefixSize = 0;
        return OptionForm_None;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::GetOptionWithPrefix(const Option& option, std::string& optionWithPrefix) const
    {
        assert(option.enumVal != BconCtlOption_Unknown && "Cannot get the prefix for an unknown option.");
        assert(optionWithPrefix.empty() && "Sstring optionWithPrefix is not empty");

        for (unsigned int i = 0; i < UTILS_COUNTOF(m_optionDescriptions); ++i)
        {
            // Find the description to option
            if (option.enumVal == m_optionDescriptions[i].enumVal)
            {
                // Return prefix+string associated with form given by option

                switch (option.form)
                {
                    case OptionForm_Short:
                        {
                            optionWithPrefix = std::string(shortPrefix) + m_optionDescriptions[i].stringShort;
                            return true;
                        }
                    case OptionForm_Long:
                        {
                            optionWithPrefix = std::string(longPrefix) + m_optionDescriptions[i].stringLong;
                            return true;
                        }
                    default:
                        {
                            assert((option.form == OptionForm_Short || option.form == OptionForm_Long) && "Cannot get the prefix for an unknown option form.");
                            return false;
                        }
                }
            }
        }

        assert(false && "No optionDescription was found for option");
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::GetSubCommandName(const SubCommand& subCommand, std::string& subCommandName) const
    {
        assert(subCommand.enumVal != BconCtlSubCommand_Unknown && "Cannot get the name of an unknown subcommand.");
        assert(subCommandName.empty() && "String subCommandName is not empty");

        for (unsigned int i = 0; i < UTILS_COUNTOF(m_subCommandDescriptions); ++i)
        {
            // Find the description to option
            if (subCommand.enumVal == m_subCommandDescriptions[i].enumVal)
            {
                // Return string associated with form given by option
                subCommandName = m_subCommandDescriptions[i].string;
                return true;
            }
        }

        assert(false && "no subCommandDescription was found for subCommand");
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    EBconCtlCommand CBconCtlCommandLineParser::GetBconCtlCommand(const char* commandString) const
    {
        for (unsigned int j = 0; j < UTILS_COUNTOF(m_commandDescriptions); ++j)
        {
            const CommandDescription& currentCommandDescription = m_commandDescriptions[j];
            assert(currentCommandDescription.enumVal != BconCtlCommand_Unknown && "Invalid enumValue in m_commandDescriptions[j]");

            bool commandFound = IsEqualStringIgnoreCase(commandString, currentCommandDescription.string);
            if (commandFound)
            {
                return currentCommandDescription.enumVal;
            }
        }

        return BconCtlCommand_Unknown;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    bool CBconCtlCommandLineParser::GetHelpText(EBconCtlCommand command, std::string& helptext) const
    {
        assert((BconCtlCommand_Unknown != command && BconCtlCommand_NumCommands != command) && "There is no help text for an unknown command. ");

        // Find command description
        unsigned int i = 0;
        for (i = 0; i < UTILS_COUNTOF(m_commandDescriptions); ++i)
        {
            if (m_commandDescriptions[i].enumVal == command)
            {
                break;
            }
        }

        // Did we find a command description
        if (i == UTILS_COUNTOF(m_commandDescriptions))
        {
            assert(i != UTILS_COUNTOF(m_commandDescriptions) && "unknown command description");
            return false;
        }

        // Command description found
        const CommandDescription& currentCommandDescription = m_commandDescriptions[i];

        // Add help texts for known subcommands
        helptext = m_commandDescriptions[i].helpText;

        unsigned int l = 0;
        bool knownSubCommandFound = false;

        // Known subcommands are at the beginning of currentCommandDescription.knownSubCommands i.e. before BconCtlSubCommand_Unknown
        while (l < UTILS_COUNTOF(currentCommandDescription.knownSubCommands) && BconCtlSubCommand_Unknown != currentCommandDescription.knownSubCommands[l])
        {
            // Add option help text for currentCommandDescription.knownCommandOptions[j]
            for (unsigned int k = 0; k < UTILS_COUNTOF(m_subCommandDescriptions); ++k)
            {
                if (m_subCommandDescriptions[k].enumVal == currentCommandDescription.knownSubCommands[l])
                {
                    if (!knownSubCommandFound)
                    {
                        // Add help texts for known subcommands
                        helptext += "\nSubcommands:\n";
                        knownSubCommandFound = true;
                    }

                    helptext += m_subCommandDescriptions[k].helpText;
                    break;
                }
            }

            ++l;
        }

        unsigned int j = 0;
        bool knownOptionFound = false;

        // Known options are at the beginning of currentCommandDescription.knownCommandOptions i.e. before BconCtlOption_Unknown
        while (j < UTILS_COUNTOF(currentCommandDescription.knownCommandOptions) && BconCtlOption_Unknown != currentCommandDescription.knownCommandOptions[j])
        {
            // Add option help text for currentCommandDescription.knownCommandOptions[j]
            for (unsigned int k = 0; k < UTILS_COUNTOF(m_optionDescriptions); ++k)
            {
                if (m_optionDescriptions[k].enumVal == currentCommandDescription.knownCommandOptions[j])
                {
                    if (!knownOptionFound)
                    {
                        // Add help texts for known options
                        helptext += "\nOptions:\n";
                        knownOptionFound = true;
                    }

                    helptext += m_optionDescriptions[k].helpText;
                    break;
                }
            }

            ++j;
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    void CBconCtlCommandLineParser::ListAllCommands(std::string& helptext)
    {
        // Loop over all known commands
        unsigned int i = 0;
        for (i = 0; i < UTILS_COUNTOF(m_commandDescriptions); ++i)
        {
            // Add command to help line
            std::string helpLine("  ");
            helpLine += m_commandDescriptions[i].string;
            helptext += helpLine + "\n";
        }
    }
}
