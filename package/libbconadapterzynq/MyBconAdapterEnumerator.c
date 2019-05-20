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
 * @file    MyBconAdapterEnumerator.c
 *
 * @brief   Implementation of the enumeration interface of the BCON adapter
 *
 * @author  Gerald Kager
 *
 * @date    24.05.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <bconadapter/BconAdapterEnumerator.h>
#include <bconadapter/BconAdapterDefines.h>
#include "MyBconAdapterLogging.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>


///////////////////////////////////////////////////////////////////////////
/// \brief Start device discovery.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BconAdapterStartDiscovery(PFUNC_BCON_ADAPTER_DISCOVERY_CALLBACK callbackToBconAdapterUser, uintptr_t userCtx)
{
    BCONSTATUS returnCode = BCON_OK;

    // Get the I2C device configuration from environment variable BCON_ADAPTER_I2C_DEVICES
    //
    // Example for how to set BCON_ADAPTER_I2C_DEVICES for two devices: 
    //
    //     export BCON_ADAPTER_I2C_DEVICES="/dev/i2c-1:77 /dev/i2c-2:99"
    //
    // The two device identifiers /dev/i2c-1:77 and /dev/i2c-2:99 are separated by a blank.
    // The first device identifier /dev/i2c-1:77 consists of the I2C bus to open /dev/i2c-1  
    // and the device address 77, the bus and address parts being separated by a colon.
    // Analog for the second device identifier /dev/i2c-2:99.
    
    char *env_config = getenv("BCON_ADAPTER_I2C_DEVICES");

    if (env_config == NULL)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Error reading env.var. ");
        return BCON_E_OPERATION_FAILED;
    }

    // Duplicate string for usage
    char *config = strdup(env_config);
    if (config == NULL)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Error out of memory. ");
        return BCON_E_OPERATION_FAILED;
    }

    // Splitting the config string at the blanks yields the device identifier tokens.
    // Call the callback function for each token.
    char *token = strtok(config, " ");
    do
    {
        LogOutput(TRACE_LEVEL_INFORMATION, "Current Token is  _%s_", token);

        if (callbackToBconAdapterUser != NULL)
        {
            BCONSTATUS callbackToBconAdapterUserStatus = callbackToBconAdapterUser(token, (userCtx));
            if (!BCON_SUCCESS(callbackToBconAdapterUserStatus))
            {
                LogOutput(TRACE_LEVEL_ERROR, "Error calling BCON Adapter user callback in enumeration, status = 0x%08X", callbackToBconAdapterUserStatus);
                returnCode = callbackToBconAdapterUserStatus;
            }
        }

        token = strtok(NULL, " ");
    } 
    while (token != NULL);

    // free duplicate string created using strdup()
    free(config);

    return returnCode;
}

