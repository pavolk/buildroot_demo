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
 * @file    MyBconAdapterZynqStream.cpp
 *
 * @brief   Implementation of the BCON adapter stream interface
 *
 * @author  Andreas Gau
 *
 * @date    07.11.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <bconadapter/BconAdapterDefines.h>
#include <bconadapter/BconAdapterStream.h>

#include "MyBconAdapterZynqSimpleMutex.h"
#include "MyBconAdapterZynqMemoryFunc.h"
#include "MyBconAdapterZynqStream.h"
#include "MyBconAdapterLogging.h"

#include <vector>
#include <list>
#include <map>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>

#include <sys/ioctl.h>

// Older linux/videodev2.h may not have defined V4L2_COLORSPACE_DEFAULT
#ifndef V4L2_COLORSPACE_DEFAULT
    #define V4L2_COLORSPACE_DEFAULT 0
#endif

#define IndexToBufferHandle(idx) \
    reinterpret_cast<BconAdapterBufferHandle>((idx) + 1U)

#define BufferHandleToIndex(hdl) \
    (reinterpret_cast<uintptr_t>(hdl) - 1U)


//////////////////////////////////////////////////////////////////////////////
static intptr_t g_nextStreamHandle = 1;
static std::map<intptr_t, StreamData*> g_theStreams;

//////////////////////////////////////////////////////////////////////////////
static BCONSTATUS BconAdapterStreamFlushBuffersImpl(StreamData* pStream, BCONSTATUS flushReturnStatus);

//////////////////////////////////////////////////////////////////////////////
///
static StreamData* LookupStream(BconAdapterStreamHandle handle)
{
    std::map<intptr_t, StreamData*>::iterator pos = g_theStreams.find((intptr_t)handle);
    if (pos == g_theStreams.end())
    {
        return NULL;
    }
    return pos->second;
}


//////////////////////////////////////////////////////////////////////////////
///
int xioctl(int fd, unsigned long int request, void *arg)
{
    int ret = 0;

    do
    {
        ret = ioctl(fd, request, arg);
    }
    while (-1 == ret && EINTR == errno);

    return ret;
}


//////////////////////////////////////////////////////////////////////////////
///
static void *PumpThread(void *args)
{
    // Cast back to StreamDataPtr
    StreamData* pStream = static_cast<StreamData*>(args);

    // Wait until streaming on was called
    while (pStream && pStream->pumpThreadRun && !pStream->v4l2StreamingEnabled)
    {
        pthread_cond_wait(&pStream->v4l2StreamingStartedCond, pStream->lock.GetMutexPtr());
    }

    while (pStream && pStream->pumpThreadRun)
    {
        // Dequeue V4L2 buffer contents
        int ret = DequeueV4L2Buffer(pStream);
        if (ret < 0)
        {
            // We can not continue here, cancel pending buffers and break
            if (errno == EINVAL)
            {
                LogOutput(TRACE_LEVEL_INFORMATION, "DequeueV4L2Buffer aborted: status=%#X, errno=%#X", ret, errno);
            }
            else
            {
                LogOutput(TRACE_LEVEL_FATAL, "Unexpected result of DequeueV4L2Buffer status=%#X, errno=%#X", ret, errno);
            }
            BconAdapterStreamFlushBuffersImpl(pStream, BCON_E_BUFFER_INCOMPLETE);
            break;
        }

        {
            MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);
            if (pStream->streamBufferReadyCallback)
            {
                pStream->streamBufferReadyCallback(pStream->myHandle, pStream->outputQueue.size(), pStream->streamBufferReadyCallbackContext);
            }
        }
    }

    return NULL;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamCreate(const char *pDeviceID, BconAdapterStreamHandle *pHandle)
{
    //check is already created for device
    for (std::map<intptr_t, StreamData*>::const_iterator it = g_theStreams.begin(); it != g_theStreams.end(); ++it)
    {
        if (it->second->deviceID == pDeviceID)
        {
            *pHandle = (BconAdapterStreamHandle)it->first;
            return BCON_S_ALREADY_CREATED;
        }
    }

    //create new stream
    g_theStreams.insert(std::make_pair(g_nextStreamHandle, new StreamData((BconAdapterStreamHandle)g_nextStreamHandle)));
    *pHandle = (BconAdapterStreamHandle)g_nextStreamHandle;
    ++g_nextStreamHandle;

    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamDestroy(BconAdapterStreamHandle handle)
{
    std::map<intptr_t, StreamData*>::iterator pos = g_theStreams.find((intptr_t)handle);
    if (pos == g_theStreams.end())
    {
        return BCON_E_INVALID_HANDLE;
    }

    delete pos->second;
    g_theStreams.erase(pos);

    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
/// \brief Opens the stream from the device.

static int setup_v4l2(int fd)
{
    int idx = 0;
    int ret;

    // Switch to first input
    // Ignore error ENOTTY - device may not support this xioctl,
    // which is o.k.
    ret = xioctl(fd, VIDIOC_S_INPUT, &idx);
    return (ret == -1 && errno == ENOTTY) ? 0 : ret;
}

EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamOpen(BconAdapterStreamHandle handle)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        pStream->fdV4L2 = -1;
        pStream->fdVsrc = -1;

        // Get video device names from environment when present.
        const char* videoDevice = getenv("BCON_ADAPTER_V4L_DEVICE");
        if (videoDevice == NULL)
        {
            videoDevice = "/dev/video0";
        }
        const char* videoSubDevice = getenv("BCON_ADAPTER_V4L_SUB_DEVICE");
        if (videoSubDevice == NULL)
        {
            videoSubDevice = "/dev/v4l-subdev0";
        }

        // Open V4L2 capture device.
        LogOutput(TRACE_LEVEL_INFORMATION, "Attempting do open %s", videoDevice);
        const int fd_v4l = open(videoDevice, O_RDWR);
        if (fd_v4l < 0)
        {
            int openErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Failed to open %s", videoDevice);

            return BconStatusFromErrno(openErrno);
        }

        struct v4l2_capability caps;
        const int ret = xioctl(fd_v4l, VIDIOC_QUERYCAP, &caps);
        if (ret < 0)
        {
            int xioctlErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "VIDIOC_QUERYCAP failed - cannot determine device capabilities");
            close(fd_v4l);

            return BconStatusFromErrno(xioctlErrno);
        }

        static const uint32_t required_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
        if (required_caps != (caps.capabilities & required_caps))
        {
            LogOutput(TRACE_LEVEL_FATAL, "%s (%s) is not a capture device", videoDevice, caps.driver);
            close(fd_v4l);

            return BCON_E_OPERATION_FAILED;
        }

        if (setup_v4l2(fd_v4l) < 0)
        {
            int setupErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Required V4L2 capability not supported by driver");
            close(fd_v4l);

            return BconStatusFromErrno(setupErrno);
        }

        // Open vidsrc device.
        LogOutput(TRACE_LEVEL_INFORMATION, "Attempting do open %s", videoSubDevice);
        const int fd_vsrc = open(videoSubDevice, O_RDWR);
        if (fd_vsrc < 0)
        {
            int openErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Failed to open %s", videoSubDevice);
            close(fd_v4l);

            return BconStatusFromErrno(openErrno);
        }

        pStream->fdV4L2 = fd_v4l;
        pStream->fdVsrc = fd_vsrc;

        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamClose(BconAdapterStreamHandle handle)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        pStream->bufferContexts.clear();
        pStream->inputQueue.clear();
        pStream->outputQueue.clear();

        close(pStream->fdV4L2);
        pStream->fdV4L2 = -1;

        close(pStream->fdVsrc);
        pStream->fdVsrc = -1;

        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamGetProperty(BconAdapterStreamHandle handle, uint64_t /*address*/, void * /*pValue*/, size_t /*valueSize*/)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        return BCON_E_READ_FAILED;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamSetProperty(BconAdapterStreamHandle handle, uint64_t /*address*/, const void * /*pValue*/, size_t /*valueSize*/)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        return BCON_E_WRITE_FAILED;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
static BCONSTATUS BconAdapterStreamCalculateGrabberParams(uint32_t* grabberWidth, uint32_t cameraWidth, uint32_t pixelFormatPfncValue)
{
    LogOutput(TRACE_LEVEL_DEBUG, "--> BconAdapterStreamCalculateGrabberParams(grabberWidth=%u, cameraWidth=%u, pixelFormatPfncValue=%u)", *grabberWidth, cameraWidth, pixelFormatPfncValue);

    // Calculate the bits used per pixel, 48 bits is maximum
    uint32_t bitsPerPixel = (pixelFormatPfncValue >> 16) & 0xFF;
    if (bitsPerPixel > 48)
    {
        LogOutput(TRACE_LEVEL_DEBUG, "<-- BconAdapterStreamCalculateGrabberParams returns status=%#X, bitsPerPixel=%u, grabberWidth=%u", BCON_E_INVALID_PARAMETER, bitsPerPixel, *grabberWidth);
        return BCON_E_INVALID_PARAMETER;
    }

    // Convert and round up to bytes per pixel
    uint32_t bytesPerPixel = (bitsPerPixel + 4) / 8;

    // Calculate width needed by grabber based
    // on 1 byte per pixel (mono8 tunneling)
    *grabberWidth = cameraWidth * bytesPerPixel;

    LogOutput(TRACE_LEVEL_DEBUG, "<-- BconAdapterStreamCalculateGrabberParams returns status=%#X, bytesPerPixel=%u, grabberWidth=%u", BCON_OK, bytesPerPixel, *grabberWidth);
    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
//
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamConfigureStreamingParameters(
    BconAdapterStreamHandle handle,
    uint32_t width,
    uint32_t height,
    uint32_t pixelFormatPfncValue,
    uint32_t clockFrequency,
    uint32_t pixelsPerClockCycle,
    size_t* payloadSizeOut)
{
    BCON_UNUSED(clockFrequency);

    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        // Get correct grabber width for current pixel format
        uint32_t grabberWidth = 0;
        BCONSTATUS status = BconAdapterStreamCalculateGrabberParams(&grabberWidth, width, pixelFormatPfncValue);
        if (BCON_ERROR(status))
        {
            LogOutput(TRACE_LEVEL_FATAL, "Failed to calculate grabber parameters.");
            return status;
        }

        // configure video device
        struct v4l2_format fmt;
        memset(&fmt, 0, sizeof fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        int ret = xioctl(pStream->fdV4L2, VIDIOC_G_FMT, &fmt);
        if (ret < 0)
        {
            int xioctlErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Failed to get capture image format.");

            return BconStatusFromErrno(xioctlErrno);
        }

        fmt.fmt.pix.width = grabberWidth;
        fmt.fmt.pix.height = height + TrailerExtraLines;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        fmt.fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT;
        fmt.fmt.pix.bytesperline = 0;
        ret = xioctl(pStream->fdV4L2, VIDIOC_S_FMT, &fmt); //TODO: tap geometry is missing
        if (ret < 0)
        {
            int xioctlErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Failed to set capture image format.");

            return BconStatusFromErrno(xioctlErrno);
        }

        // We don't expect adjustments from video device!
        if (fmt.fmt.pix.width != grabberWidth)
        {
            LogOutput(TRACE_LEVEL_WARNING, "Video driver adjusted width=%u to %u.", grabberWidth, fmt.fmt.pix.width);
        }
        if (fmt.fmt.pix.height != height + TrailerExtraLines)
        {
            LogOutput(TRACE_LEVEL_WARNING, "Video driver adjusted height=%u to %u.", height, fmt.fmt.pix.height);
        }

        // configure VSRC device
        struct v4l2_subdev_format subdev_fmt;
        memset(&subdev_fmt, 0, sizeof subdev_fmt);
        subdev_fmt.pad = 0;    // only one input pad for video source
        subdev_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        subdev_fmt.format.width = grabberWidth;
        subdev_fmt.format.height = height + TrailerExtraLines;
        subdev_fmt.format.code = MEDIA_BUS_FMT_Y8_1X8;     // use fixed 8 bit
        subdev_fmt.format.field = V4L2_FIELD_NONE;
        subdev_fmt.format.colorspace = V4L2_COLORSPACE_DEFAULT;

        ret = ioctl(pStream->fdVsrc, VIDIOC_SUBDEV_S_FMT, &subdev_fmt);
        if (ret < 0)
        {
            int xioctlErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Failed to set VSRC image format.");

            return BconStatusFromErrno(xioctlErrno);
        }

        // We don't expect adjustments from VSRC device!
        if (subdev_fmt.format.width != grabberWidth)
        {
            LogOutput(TRACE_LEVEL_WARNING, "VSRC driver adjusted width=%u to %u.", grabberWidth, subdev_fmt.format.width);
        }
        if (subdev_fmt.format.height != height + TrailerExtraLines)
        {
            LogOutput(TRACE_LEVEL_WARNING, "VSRC driver adjusted height=%u to %u.", height, subdev_fmt.format.height);
        }

        // save stream parameters for grab result
        pStream->width = width;
        pStream->height = height;
        pStream->pixelFormatPfncValue = pixelFormatPfncValue;
        pStream->payloadSize = fmt.fmt.pix.sizeimage - (TrailerExtraLines * grabberWidth);
        pStream->pixelsPerClockCycle = pixelsPerClockCycle;

        *payloadSizeOut = pStream->payloadSize;

        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamPrepareGrab(BconAdapterStreamHandle handle, size_t maxNumBuffer, size_t /*maxBufferSize*/)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        pStream->v4l2StreamingEnabled = false;
        pStream->bufferContexts.clear();

        // Request V4L2 buffers
        int ret = PrepareV4L2Buffer(pStream, maxNumBuffer);
        if (ret < 0)
        {
            return BconStatusFromErrno(errno);
        }

        pStream->bufferContexts.resize(pStream->nBuffers);
        pStream->inputQueue.clear();
        pStream->outputQueue.clear();

        for (std::vector<BufferContextData>::iterator it = pStream->bufferContexts.begin(); it != pStream->bufferContexts.end(); ++it)
        {
            it->Clear();
        }

        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamRegisterBuffer(BconAdapterStreamHandle handle, void *pBuffer, size_t bufferSize, BconAdapterBufferHandle *phBuffer)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        //check already registered
        for (std::vector<BufferContextData>::const_iterator it = pStream->bufferContexts.begin(); it != pStream->bufferContexts.end(); ++it)
        {
            if (it->pBuffer == pBuffer)
            {
                return BCON_E_BUFFER_ALREADY_REGISTERED;
            }
        }

        //find free buffer context
        for (std::vector<BufferContextData>::iterator it = pStream->bufferContexts.begin(); it != pStream->bufferContexts.end(); ++it)
        {
            if (it->pBuffer == NULL)
            {
                it->pBuffer = pBuffer;
                it->bufferSize = bufferSize;
                *phBuffer = IndexToBufferHandle(it - pStream->bufferContexts.begin());
                LogOutput(TRACE_LEVEL_INFORMATION, "Registered buffer @ %p, size = %lu, hdl = %p", pBuffer, bufferSize, *phBuffer);
                return BCON_OK;
            }
        }

        return BCON_E_OPERATION_FAILED; // More buffers than requested
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamFlushBuffers(BconAdapterStreamHandle handle)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        // Call internal implementation
        return BconAdapterStreamFlushBuffersImpl(pStream, BCON_E_BUFFER_CANCELLED);
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
static BCONSTATUS BconAdapterStreamFlushBuffersImpl(StreamData* pStream, BCONSTATUS flushReturnStatus)
{
    MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);

    while (!pStream->inputQueue.empty())
    {
        pStream->inputQueue.front()->status = flushReturnStatus;
        pStream->outputQueue.push_back(pStream->inputQueue.front());
        pStream->inputQueue.pop_front();
    }

    if (pStream->streamBufferReadyCallback && !pStream->outputQueue.empty())
    {
        pStream->streamBufferReadyCallback(pStream->myHandle, pStream->outputQueue.size(), pStream->streamBufferReadyCallbackContext);
    }

    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamStartStreaming(BconAdapterStreamHandle handle)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        pStream->pumpThreadRun = true;
        int ret = ::pthread_create(&pStream->pumpThread, NULL, &PumpThread, pStream);
        if (ret < 0)
        {
            int pthreadCreateErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Failed to start pump thread.");

            return BconStatusFromErrno(pthreadCreateErrno);
        }

        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamStopStreaming(BconAdapterStreamHandle handle)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        pStream->pumpThreadRun = false;
        if (pStream->pumpThread)
        {
            {
                MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);

                // turn off v4l stream to leave blocking DequeueV4L2Buffer() call in PumpThread()
                if (pStream->v4l2StreamingEnabled)
                {
                    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    int ret = xioctl(pStream->fdV4L2, VIDIOC_STREAMOFF, &type);
                    if (ret < 0)
                    {
                        int xioctlErrno = errno;
                        LogOutput(TRACE_LEVEL_FATAL, "Failed to stop streaming.");

                        return BconStatusFromErrno(xioctlErrno);
                    }

                    pStream->v4l2StreamingEnabled = false;
                }
            }

            ::pthread_join(pStream->pumpThread, NULL);
        }

        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
/// \brief Queues a registered buffer for grabbing.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamQueueBuffer(BconAdapterStreamHandle handle, BconAdapterBufferHandle hBuffer, const void* context)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        const size_t index = BufferHandleToIndex(hBuffer);
        if (index >= pStream->nBuffers)
        {
            return BCON_OK; // ignore extra buffers
        }
        if (!pStream->bufferContexts[index].pBuffer)
        {
            return BCON_E_INVALID_HANDLE;
        }
        if (pStream->bufferContexts[index].queued)
        {
            return BCON_E_BUFFER_ALREADY_QUEUED;
        }

        {
            MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);
            pStream->bufferContexts[index].userContext = context;
            pStream->bufferContexts[index].OnBeforeQueue();

            pStream->inputQueue.push_back(&pStream->bufferContexts[index]);

            // Allocate V4L2 buffer
            int ret = QueueV4L2Buffer(pStream, index);
            if (ret < 0)
            {
                return BconStatusFromErrno(errno);
            }

            // Turn on v4l streaming, requires at least one buffer according to V4L2 doc
            if (!pStream->v4l2StreamingEnabled)
            {
                int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                int ret = xioctl(pStream->fdV4L2, VIDIOC_STREAMON, &type);
                if (ret < 0)
                {
                    int xioctlErrno = errno;
                    LogOutput(TRACE_LEVEL_FATAL, "Failed to start streaming.");

                    return BconStatusFromErrno(xioctlErrno);
                }

                // Signal pump thread that streaming was enabled
                pStream->v4l2StreamingEnabled = true;
                pthread_cond_signal(&pStream->v4l2StreamingStartedCond);
            }

            return BCON_OK;
        }
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
/// \brief Retrieve a grab result.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamRetrieveResult(BconAdapterStreamHandle handle, BconGrabResult *pGrabResult, size_t *pNumBuffersLeft)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream == NULL)
    {
        return BCON_E_INVALID_HANDLE;
    }

    if (pGrabResult == NULL)
    {
        return BCON_E_INVALID_PARAMETER;
    }

    {
        MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);
        if (pStream->outputQueue.empty())
        {
            return BCON_E_NO_BUFFER_AVAILABLE;
        }

        // Copy parameters
        pGrabResult->hBuffer = IndexToBufferHandle(pStream->outputQueue.front() - &pStream->bufferContexts[0]);
        pGrabResult->pBuffer = pStream->outputQueue.front()->pBuffer;
        pGrabResult->userContext = pStream->outputQueue.front()->userContext;
        pGrabResult->payloadType = 0x0001; // Uncompressed image data
        pGrabResult->blockId = pStream->outputQueue.front()->trailer.blockId;
        pGrabResult->timeStamp = pStream->outputQueue.front()->trailer.timeStamp;
        pGrabResult->pixelFormat = pStream->outputQueue.front()->trailer.pixelFormat;
        pGrabResult->sizeX = pStream->width;
        pGrabResult->sizeY = pStream->height;
        pGrabResult->offsetX = 0;
        pGrabResult->offsetY = 0;
        pGrabResult->paddingX = 0;
        pGrabResult->paddingY = 0;
        pGrabResult->payloadSize = pStream->payloadSize;
        pGrabResult->errorCode = pStream->outputQueue.front()->status;

        pStream->outputQueue.front()->OnDeQueue();
        pStream->outputQueue.pop_front();

        if (pStream->streamBufferReadyCallback)
        {
            pStream->streamBufferReadyCallback(pStream->myHandle, pStream->outputQueue.size(), pStream->streamBufferReadyCallbackContext);
        }

        if (pNumBuffersLeft)
        {
            *pNumBuffersLeft = pStream->outputQueue.size();
        }
    }

    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
/// \brief Deregisters a buffer.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamDeregisterBuffer(BconAdapterStreamHandle handle, BconAdapterBufferHandle hBuffer, void** ppBuffer)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        const size_t index = BufferHandleToIndex(hBuffer);
        if (index >= pStream->bufferContexts.size())
        {
            return BCON_E_INVALID_HANDLE;
        }

        if (!pStream->bufferContexts[index].pBuffer)
        {
            return BCON_E_INVALID_HANDLE;
        }

        if (pStream->bufferContexts[index].queued)
        {
            return BCON_E_BUFFER_STILL_QUEUED;
        }

        {
            if (ppBuffer)
            {
                *ppBuffer = pStream->bufferContexts[index].pBuffer;
            }
            pStream->bufferContexts[index].Clear();
            return BCON_OK;
        }
    }

    return BCON_E_INVALID_HANDLE;
}


//////////////////////////////////////////////////////////////////////////////
/// \brief Finish grab session.
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamFinishGrab(BconAdapterStreamHandle handle)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream == NULL)
    {
        return BCON_E_INVALID_HANDLE;
    }

    // Finish grab needs flushed buffers
    BconAdapterStreamFlushBuffersImpl(pStream, BCON_E_BUFFER_CANCELLED);

    int ret = FreeV4L2Buffer(pStream);
    if (ret < 0)
    {
        return BconStatusFromErrno(errno);
    }

    return BCON_OK;
}


//////////////////////////////////////////////////////////////////////////////
///
EXTERN_C BCON_ADAPTER_API BCONSTATUS BCON_ADAPTER_CALL BconAdapterStreamRegisterCallback(BconAdapterStreamHandle handle, BconAdapterStreamBufferReadyCallback pCallback, void* context)
{
    StreamData* pStream = LookupStream(handle);
    if (pStream != NULL)
    {
        MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);
        if (pCallback != NULL)
        {
            pStream->streamBufferReadyCallback = pCallback;
            pStream->streamBufferReadyCallbackContext = context;
        }
        else
        {
            pStream->streamBufferReadyCallback = NULL;
            pStream->streamBufferReadyCallbackContext = NULL;
        }
        return BCON_OK;
    }

    return BCON_E_INVALID_HANDLE;
}
