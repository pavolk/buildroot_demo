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
 * @file    MyBconAdapterLogging.c
 *
 * @brief   Implementation of BCON adapter logging
 *
 * @author  Dennis Voss
 *
 * @date    19.05.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <bconadapter/BconAdapterDefines.h>
#include <bconadapter/BconAdapterApi.h>

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/param.h>
#include "MyBconAdapterLogging.h"

// global function pointer for log function
static BconTraceFunc g_pTraceFunc = NULL;


// Write log output using the log function set with SetExternalLogFunction
EXTERN_C void BCON_ADAPTER_CDECL LogOutput(BconAdapterTraceLevel level, const char *pFormat, ...)
{
    if (g_pTraceFunc == NULL)
    {
        return;
    }

    va_list argptr;
    va_start(argptr, pFormat);

    BconTraceFunc pTraceFunc = g_pTraceFunc;
    if (pTraceFunc != NULL)
    {
        pTraceFunc(level, pFormat, argptr);
    }

    va_end(argptr);
}


// Set log function pointer, handed down in BconAdapterInit()
EXTERN_C void SetExternalLogFunction(BconTraceFunc pFunc)
{
    g_pTraceFunc = pFunc;
}


EXTERN_C BCONSTATUS BconStatusFromErrno(int error)
{
    // Check for success code
    if (error == 0)
    {
        return BCON_OK;
    }

    // Convert to BCON custom error code.
    return BCON_E_CUSTOM | (error & 0x7fff);
}


EXTERN_C int BconStatusToErrno(BCONSTATUS status)
{
    // Check for success code
    if (status == BCON_OK)
    {
        return 0;
    }

    // Convert to POSIX error code.
    return (int)(status & 0x7fff);
}


BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterGetStatusMessage(BCONSTATUS statusToDecode, char *pBuffer, size_t bufferSize, size_t *pRequiredSize)
{
    char errorMessageBuffer[2048];
    size_t errorMessageLength = 0;

    // Convert to POSIX errno
    int errnum = BconStatusToErrno(statusToDecode);

    // Clear buffer and get message for given errno
    memset(&errorMessageBuffer[0], 0, sizeof(errorMessageBuffer));

    errno = 0;
#ifdef _GNU_SOURCE
    char* msg = strerror_r(errnum, &errorMessageBuffer[0], sizeof(errorMessageBuffer) - 1)
    BCON_UNUSED(msg);
#else
    int ret = strerror_r(errnum, &errorMessageBuffer[0], sizeof(errorMessageBuffer) - 1);
    BCON_UNUSED(ret);
#endif
    if (errno == EINVAL)
    {
        return BCON_E_NOT_FOUND;
    }

    // Get length of error message and
    errorMessageLength = strlen(&errorMessageBuffer[0]);

    // Check if requiredSize parameter was set
    if (pRequiredSize != NULL)
    {
        *pRequiredSize = errorMessageLength + 1;
    }

    // Check that buffer is present and has enough size
    if ((pBuffer != NULL) && (bufferSize != 0))
    {
        // Copy message buffer, ensure that buffer has terminating zero
        strncpy(pBuffer, &errorMessageBuffer[0], MIN(bufferSize, errorMessageLength));
        pBuffer[bufferSize - 1] = '\0';

    }

    return BCON_OK;
}
