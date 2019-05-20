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
 * @file    MyBconAdapterI2CConnection.c
 *
 * @brief   Implementation of the I2C interface of the BCON adapter
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
#include <bconadapter/BconAdapterI2C.h>
#include "MyBconAdapterLogging.h"
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>

// Limit for string length
#define MAX_STRING_LENGTH  256

// Helper function to convert BconAdapterI2cBusHandle to file descriptor (int)
static int getFd(BconAdapterI2cBusHandle arg)
{
    return *((int*)(&arg));
}

//////////////////////////////////////////////////////////////////////////////
/// \brief Opens the I2C bus connection to a camera device.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterI2cOpenConnection(
    const char deviceId[], BconAdapterI2cBusHandle *phBus, uint32_t *pDeviceAddress)
{
    // Copy string for usage
    char devicePath[MAX_STRING_LENGTH] = { 0 };
    strcpy(devicePath, deviceId);

    // String is like "/dev/i2c-2:99", so find ':'
    char *separator = (char*)strchr(devicePath, ':');
    if (separator == NULL)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Device ID incomplete.");
        return BCON_E_NOT_FOUND;
    }

    // Parse device address
    int deviceAddress = atoi(separator + 1);
    if ((deviceAddress < 0) || (deviceAddress >= 128))
    {
        LogOutput(TRACE_LEVEL_ERROR, "Error parsing device address, only 7-bit address allowed.");
        return BCON_E_INVALID_PARAMETER;
    }

    // Open device
    separator[0] = '\0';
    int fd = open(devicePath, O_RDWR);
    if (fd < 0)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Could not open device '%s': %s.", devicePath, strerror(errno));
        return BCON_E_OPERATION_FAILED;
    }

    // Lock I2C bus device and check if device is already locked
    // indicating a non-exclusive and therefore forbidden access.
    // The lock is released when the descriptor has been closed.
    if (flock(fd, LOCK_EX | LOCK_NB) < 0)
    {
        if (errno == EWOULDBLOCK)
        {
            LogOutput(TRACE_LEVEL_ERROR, "Device '%s' is already in use.", devicePath);
        }
        else
        {
            LogOutput(TRACE_LEVEL_ERROR, "Could not lock device '%s': %s", devicePath, strerror(errno));
        }
        close(fd);
        return BCON_E_OPERATION_FAILED;
    }

    // Return value device address
    if (pDeviceAddress != NULL)
    {
        *pDeviceAddress = deviceAddress;
    }

    // Return value bus handle
    if (phBus != NULL)
    {
        *phBus = (BconAdapterI2cBusHandle)(long)fd;
    }

    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
/// \brief Closes the I2C bus connection to a camera device.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterI2cCloseConnection(
    BconAdapterI2cBusHandle hBus, uint32_t deviceAddress)
{
    (void)deviceAddress;

    // Close file descriptor
    int ret = close(getFd(hBus));
    if (ret != 0)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Could not close device.");
        return BCON_E_OPERATION_FAILED;
    }

    return BCON_OK;
}

//////////////////////////////////////////////////////////////////////////////
/// \brief Reads a block of data from I2C bus.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterI2cRead(
    BconAdapterI2cBusHandle hBus,
    uint32_t deviceAddress,
    void *pData,
    size_t sizeInBytes,
    size_t *pBytesRead,
    uint32_t timeout_ms)
{
    (void)timeout_ms;

    // Set target device address
    if (ioctl(getFd(hBus), I2C_SLAVE, deviceAddress) < 0)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Error setting target address.");
        return BCON_E_OPERATION_FAILED;
    }

    // Read from I2C
    int bytesRead = read(getFd(hBus), pData, sizeInBytes);
    if (bytesRead <= 0)
    {
        return BCON_E_READ_FAILED;
    }
    else
    {
        *pBytesRead = bytesRead;
        return BCON_OK;
    }
}

//////////////////////////////////////////////////////////////////////////////
/// \brief Writes a block of data on the I2C bus.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterI2cWrite(
    BconAdapterI2cBusHandle hBus,
    uint32_t deviceAddress,
    const void *pData,
    size_t sizeInBytes,
    uint32_t timeout_ms)
{
    (void)timeout_ms;

    // Set target device address
    if (ioctl(getFd(hBus), I2C_SLAVE, deviceAddress) < 0)
    {
        LogOutput(TRACE_LEVEL_ERROR, "Error setting target address.");
        return BCON_E_OPERATION_FAILED;
    }

    // Write to I2C
    // Note: The BCON camera uses clock stretching.
    // The used I2C master hardware must support clock stretching properly.
    int bytesWritten = write(getFd(hBus), pData, sizeInBytes);
    if (bytesWritten != sizeInBytes)
    {
        return BCON_E_WRITE_FAILED;
    }
    else
    {
        return BCON_OK;
    }
}

