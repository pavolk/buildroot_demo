/** ----------------------------------------------------------------------------
 *
 * Basler dart BCON for LVDS Development Kit
 * http://www.baslerweb.com
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2016-2018, Basler AG
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
 * @file    MyBconAdapterLogging.h
 *
 * @brief   Header of BCON adapter logging
 *
 * @author  Dennis Voss
 *
 * @date    19.05.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#pragma once

#include <bconadapter/BconAdapterDefines.h>
#include <bconadapter/BconAdapterTypes.h>


// Trace level constants
#define TRACE_LEVEL_FATAL       (BconAdapterTraceLevel_Critical)      ///< Abnormal exit or termination
#define TRACE_LEVEL_ERROR       (BconAdapterTraceLevel_Error)         ///< Severe errors that need logging
#define TRACE_LEVEL_WARNING     (BconAdapterTraceLevel_Warning)       ///< Warnings such as allocation failure
#define TRACE_LEVEL_INFORMATION (BconAdapterTraceLevel_Information)   ///< Includes non-error cases(e.g. function entry or exit logging)
#define TRACE_LEVEL_VERBOSE     (BconAdapterTraceLevel_Verbose)       ///< Detailed traces from intermediate steps
#define TRACE_LEVEL_DEBUG       (BconAdapterTraceLevel_Debug)         ///< Traces for debugging purposes


// Write to log output 
EXTERN_C void BCON_ADAPTER_CDECL LogOutput(BconAdapterTraceLevel level, const char *pFormat, ...);

// Set log function pointer
EXTERN_C void SetExternalLogFunction(BconTraceFunc pFunc);

// Converts V4L2 errors to BCON custom errors and back
EXTERN_C BCONSTATUS BconStatusFromErrno(int errnoError);
EXTERN_C int BconStatusToErrno(BCONSTATUS status);
