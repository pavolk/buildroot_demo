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
 * @file    bconctlCommandLineParser.h
 *
 * @brief   Header for bconctl command line parser
 *
 * @author  Björn Rennfanz
 *
 * @date    14.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#pragma once

#include <string>
#include <vector>

#include <string.h>

// Own macro definitions
#define UTILS_COUNTOF(arr) (sizeof(arr)/sizeof(*arr))
#define UTILS_UNUSED(unused_var) ((void)(unused_var))

namespace Utils
{
    enum EBconCtlCommand
    {
        BconCtlCommand_Unknown = 0, //BconCtlCommand_Unknown must be 0 because it marks not set entries
        BconCtlCommand_Version,
        BconCtlCommand_Help,
        BconCtlCommand_Config,
        BconCtlCommand_Led0,
        BconCtlCommand_Led1,
        BconCtlCommand_Led2,
        BconCtlCommand_Power,
        BconCtlCommand_Reset,
        BconCtlCommand_Trggen,
        BconCtlCommand_NumCommands // must be last enum
    };

    enum EBconCtlSubCommand
    {
        BconCtlSubCommand_Unknown = 0, //BconCtlSubCommand_Unknown must be 0 because it marks not set entries
        BconCtlSubCommand_On,
        BconCtlSubCommand_Off,
        BconCtlSubCommand_I2CId,
        BconCtlSubCommand_NumCommands // must be last enum
    };

    enum EBconCtlOption
    {
        BconCtlOption_Unknown = 0, //BconCtlOption_Unknown must be 0 because it marks not set entries
        BconCtlOption_Help,
        BconCtlOption_NoLogo,
        BconCtlOption_Quiet,
        BconCtlOption_Version,
        BconCtlOption_Pulse,
        BconCtlOption_NumOptions // must be last enum
    };

    enum EBconCtlErrorState
    {
        BconCtlErrorState_OK = 0,
        BconCtlErrorState_OptionParamMissing,
        BconCtlErrorState_UnknownOption,
        BconCtlErrorState_UnknownCommand,
        BconCtlErrorState_UnknownSubCommand,
        BconCtlErrorState_NumStates // must be last enum
    };

    class CBconCtlCommandLineParser
    {
    public:
        ///////////////////////////////////////////////////////////////////////////
        //
        CBconCtlCommandLineParser()
        {
            Reset();
        }


        ///////////////////////////////////////////////////////////////////////////
        // parse and check args passed on command-line
        bool Parse(int argc, const char* argv[])
        {
            Reset();

            bool retVal = ParseCommandLine(argc, argv);
            retVal = retVal && EvaluateParsedData();

            return retVal;
        }

        ///////////////////////////////////////////////////////////////////////////
        //
        EBconCtlCommand GetCommand() const
        {
            return m_command;
        }

        ////////////////////////////////////////////////////////////
        // returns true if option is a parsed option;
        bool IsOption(EBconCtlOption option) const;

        ////////////////////////////////////////////////////////////
        // returns true and get the (maybe empty) parameter of option if option is a parsed option
        bool GetOptionParameter(EBconCtlOption option, std::string& parameter) const;

        ////////////////////////////////////////////////////////////
        // returns true if option is a parsed option;
        bool IsSubCommand(EBconCtlSubCommand subCommand) const;

        ////////////////////////////////////////////////////////////
        // returns true and get the (maybe empty) parameter of option if option is a parsed option
        bool GetSubCommandParameter(EBconCtlSubCommand subCommand, std::string& parameter) const;


        ////////////////////////////////////////////////////////////
        // error state:

        struct ErrorInfo
        {
            EBconCtlErrorState errorState;
            std::string errorDescription;
            std::string errorArg;
            bool set(EBconCtlErrorState state, const char* arg);
            void clear()
            {
                errorState = BconCtlErrorState_OK;
                errorDescription.clear();
                errorArg.clear();
            }
        };

        ///////////////////////////////////////////////////////////////////////////
        //
        const ErrorInfo& GetErrorInfo() const
        {
            return m_errorInfo;
        }

        ///////////////////////////////////////////////////////////////////////////
        //
        bool GetHelpText(EBconCtlCommand command, std::string& helptext) const;

        ///////////////////////////////////////////////////////////////////////////
        //
        void ListAllCommands(std::string& helptext);

        ////////////////////////////////////////////////////////////
        // helper function for comparing strings, case insensitive
        static bool IsEqualStringIgnoreCase(const char* pa, const char* pb)
        {
            if (strlen(pb) != strlen(pa))
            {
                return false;
            }

            for (unsigned int i = 0; i < strlen(pa); ++i)
            {
                if (tolower(pa[i]) != tolower(pb[i]))
                {
                    return false;
                }
            }

            return true;
        }

    private:
        ///////////////////////////////////////////////////////////////////////////
        //
        void Reset()
        {
            m_command = BconCtlCommand_Unknown;
            m_subCommand.enumVal = BconCtlSubCommand_Unknown;

            m_subCommand.parameter.clear();
            m_options.clear();
            m_errorInfo.clear();
        }

        ////////////////////////////////////////////////////////////
        // parse the command line arguments;
        // return false if an argument can't be interpreted
        bool ParseCommandLine(int argc, const char* argv[]);

        ////////////////////////////////////////////////////////////
        // evaluate the parsed data if they are consistent
        bool EvaluateParsedData();


        enum EBconCtlOptionForm
        {
            OptionForm_None
            , OptionForm_Short
            , OptionForm_Long
        };

        ////////////////////////////////////////////////////////////
        // the option prefixes for long and short form
        static const char longPrefix[2];
        static const char shortPrefix[1];

        ////////////////////////////////////////////////////////////
        // Get the form (e.g. short/long)
        EBconCtlOptionForm GetOptionForm(const char* arg, size_t& prefixSize) const;


        ////////////////////////////////////////////////////////////
        // get the option with its prefix as given on the command line
        struct Option; // forward declaration
        bool GetOptionWithPrefix(const Option& option, std::string& optionWithPrefix) const;

        ////////////////////////////////////////////////////////////
        // get the sub command as given on the command line
        struct SubCommand;
        bool GetSubCommandName(const SubCommand& subCommand, std::string& subCommandName) const;

        // get the command enum value given by the associated entry to commandString in commandDescriptions;
        // return BconCtlCommand_Unknown if there is no such enum value
        EBconCtlCommand GetBconCtlCommand(const char* commandString) const;

        ////////////////////////////////////////////////////////////
        // evaluation tables

        struct OptionDescription
        {
            const char* stringLong;
            const char* stringShort;
            EBconCtlOption enumVal;
            int additionalOptionParamsFollowing;
            std::string helpText;
        };

        static const int cNumberOfKnownOptions = BconCtlOption_NumOptions - 1;
        static const OptionDescription m_optionDescriptions[cNumberOfKnownOptions]; // do not describe BconCtlOption_Unknown

        struct SubCommandDescription
        {
            const char* string;
            EBconCtlSubCommand enumVal;

            int additionalSubCommandParamsFollowing;
            std::string helpText;
        };

        static const int cNumberOfKnownSubCommands = BconCtlSubCommand_NumCommands - 1;
        static const SubCommandDescription m_subCommandDescriptions[cNumberOfKnownSubCommands]; // do not describe BconCtlSubCommand_Unknown

        struct CommandDescription
        {
            const char* string;
            EBconCtlCommand enumVal;

            EBconCtlSubCommand knownSubCommands[cNumberOfKnownSubCommands]; // array of options known to the command which have to be at the
                                                                            // beginning of the array i.e. before the first BconCtlSubCommand_Unknown Entry

            EBconCtlOption knownCommandOptions[cNumberOfKnownOptions]; // array of options known to the command which have to be at the
                                                                       // beginning of the array i.e. before the first BconCtlOption_Unknown Entry
            std::string helpText;
        };

        static const int cNumberOfKnownCommands = BconCtlCommand_NumCommands - 1;
        static const CommandDescription m_commandDescriptions[cNumberOfKnownCommands]; // do not describe BconCtlCommand_Unknown

        ////////////////////////////////////////////////////////////
        // error description
        struct ErrorDescription
        {
            EBconCtlErrorState enumVal;
            const char* string;
        };

        static const int cNumberOfErrorDescriptions = BconCtlErrorState_NumStates - 1;
        static const ErrorDescription m_errorDescriptions[cNumberOfErrorDescriptions]; // do not describe BconCtlErrorState_OK

        ////////////////////////////////////////////////////////////
        // parsed data

        EBconCtlCommand m_command;

        // Struct to store found bconctl sub command
        struct SubCommand
        {
            EBconCtlSubCommand enumVal;
            std::string parameter; // following option parameter string if any, empty string otherwise
        };

        SubCommand m_subCommand;

        // Struct to store all found bconctl options
        struct Option
        {
            EBconCtlOption enumVal;
            EBconCtlOptionForm form;
            std::string parameter; // following option parameter string if any, empty string otherwise
        };

        std::vector<Option> m_options;
        ErrorInfo m_errorInfo;
    };
}
