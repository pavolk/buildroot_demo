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
 * @file    MyBconAdapterZynqMemoryMMAP.cpp
 *
 * @brief   Implementation of V4L2 functions using memory-mapped buffer mode
 *
 * @author  Björn Rennfanz
 *
 * @date    28.11.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <bconadapter/BconAdapterDefines.h>

#include "MyBconAdapterZynqSimpleMutex.h"
#include "MyBconAdapterZynqMemoryFunc.h"
#include "MyBconAdapterZynqStream.h"
#include "MyBconAdapterLogging.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <linux/videodev2.h>
#include <sys/mman.h>

//////////////////////////////////////////////////////////////////////////////
///
/*
The default memcpy of gcc didn't perform well enough.
This simple replacement works well for larger buffers.
See
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13544.html
https://aelseb.wordpress.com/2015/04/11/contiguous-memory-on-arm-and-cache-coherency/
for more info on this topic.
*/
void memcpy_neon(void *dst, const void *src, int size)
{
    if( (uintptr_t)src & 7 || (uintptr_t)dst & 7 ) {
        LogOutput(TRACE_LEVEL_WARNING, "Buffers are not 64bit aligned. Falling back to standard memcpy");
        memcpy(dst, src, size);
        return;
    }

    int rest = size & 127;
    int size_aligned = size - rest;

    if( size_aligned >= 128 ) {
        //copy in chunks of 16*8=128 Byte
        //no prefetching is done, as our buffer is propably uncached
        asm volatile (
        "NEONCopy:                          \n"
        "    VLDM %[src]!,{d0-d15}          \n"
        "    VSTM %[dst]!,{d0-d15}          \n"
        "    SUBS %[size],%[size],#0x80     \n"
        "    BGT NEONCopy                   \n"
        : [dst]"+r"(dst), [src]"+r"(src), [size]"+r"(size_aligned)
        :
        : "d0", "d1", "d2" , "d3" , "d4" , "d5" , "d6" , "d7",
          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "cc", "memory");
    }

    if(rest) {
        memcpy( (char*)dst + size_aligned, (char*)src + size_aligned, rest);
    }
}


//////////////////////////////////////////////////////////////////////////////
///
int DequeueV4L2Buffer(StreamData* pStream)
{
    struct v4l2_buffer v4l2Buf;
    memset(&v4l2Buf, 0, sizeof v4l2Buf);

    // Get buffer from V4L device, call is blocking
    v4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2Buf.memory = V4L2_MEMORY_MMAP;
    int ret = xioctl(pStream->fdV4L2, VIDIOC_DQBUF, &v4l2Buf);
    if (ret < 0)
    {
        int xioctlErrno = errno;
        LogOutput(TRACE_LEVEL_INFORMATION, "Dequeuing of buffer failed.");
        errno = xioctlErrno;

        return ret;
    }

    {
        MyBconAdapterZynq::CScopedSimpleLock lock(pStream->lock);
        if (!pStream->inputQueue.empty())
        {
            // Copy buffers from V4L2 to BCON stream buffer
            BufferContextData* pData = pStream->inputQueue.front();
            const uint8_t* src = static_cast<uint8_t *>(pStream->v4l2Buffers[v4l2Buf.index].start);
            const uint32_t bytesPerLine = pStream->payloadSize / pStream->height;
            struct ImageTrailer trailer = { 0 };
            // The status value of the tiler magic is located at the start of the 1. extra image line
            const uint8_t* pMagic = src + pStream->payloadSize;
            const uint8_t* pVersion = pMagic + sizeof trailer.magic;
            // The status value of the trailer is located at the start of the 2. extra image line
            const uint8_t* pStatus = src + pStream->payloadSize + bytesPerLine;
            const uint8_t* pPixelFormat = pStatus + sizeof trailer.status;
            // The block id value of the trailer is located at the start of the 3. extra image line
            const uint8_t* pBlockId = src + pStream->payloadSize + (2 * bytesPerLine);
            // The block stamp of the trailer is located at the start of the 4. extra image line
            const uint8_t* pTimeStamp = src + pStream->payloadSize + (3 * bytesPerLine);
            bool trailerOk = true;

            // Get trailer data ot of V4L2 buffer
            memcpy(&trailer.magic, pMagic, sizeof trailer.magic);
            if (trailer.magic != TrailerMagic)
            {
                trailerOk = false;
                LogOutput(TRACE_LEVEL_ERROR, "Unexpected trailer magic %08x instead of %08x.", trailer.magic, TrailerMagic);
            }
            else
            {
                memcpy(&trailer.version, pVersion, sizeof trailer.version);
                if (trailer.version != TrailerVersion)
                {
                    trailerOk = false;
                    LogOutput(TRACE_LEVEL_ERROR, "Unexpected trailer version %08x instead of %08x.", trailer.version, TrailerVersion);
                }
            }

            if (trailerOk)
            {
                memcpy(&trailer.status, pStatus, sizeof trailer.status);
                memcpy(&trailer.pixelFormat, pPixelFormat, sizeof trailer.pixelFormat);
                memcpy(&trailer.blockId, pBlockId, sizeof trailer.blockId);
                memcpy(&trailer.timeStamp, pTimeStamp, sizeof trailer.timeStamp);
            }
            else
            {
                // This image trailer data will be used to fill BconGrabResult
                trailer.status = BCON_E_BUFFER_INCOMPLETE;
                trailer.pixelFormat = -1;       // undefined
                trailer.blockId = UINT64_MAX;   // invalid
                trailer.timeStamp = 0;
            }

            // Copy only image payload without trailer data, using our own memcpy implementation
            // memcpy(pData->pBuffer, src, v4l2Buf.bytesused - (TrailerExtraLines * bytesPerLine));
            memcpy_neon(pData->pBuffer, src, v4l2Buf.bytesused - (TrailerExtraLines * bytesPerLine));

            pStream->inputQueue.pop_front();
            pData->queued = false;

            pStream->outputQueue.push_back(pData);
            pStream->outputQueue.back()->trailer = trailer;
            pStream->outputQueue.back()->status = trailer.status;
        }
        else
        {
            LogOutput(TRACE_LEVEL_WARNING, "Input queue empty: Skip V4L2 buffer %d.", v4l2Buf.index);
        }
    }

    // Recycle V4L2 buffer, queue it again
    v4l2Buf.reserved = v4l2Buf.reserved2 = 0U;
    ret = xioctl(pStream->fdV4L2, VIDIOC_QBUF, &v4l2Buf);
    if (ret < 0)
    {
        int xioctlErrno = errno;
        LogOutput(TRACE_LEVEL_FATAL, "Queuing of buffer failed.");
        errno = xioctlErrno;
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
///
int PrepareV4L2Buffer(StreamData* pStream, size_t numBuffers)
{
    struct v4l2_requestbuffers v4l2ReqBuf;
    memset(&v4l2ReqBuf, 0, sizeof v4l2ReqBuf);

    v4l2ReqBuf.count = 3U;
    v4l2ReqBuf.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2ReqBuf.memory = V4L2_MEMORY_MMAP;

    int ret = xioctl(pStream->fdV4L2, VIDIOC_REQBUFS, &v4l2ReqBuf);
    if (ret < 0)
    {
        int xioctlErrno = errno;
        LogOutput(TRACE_LEVEL_FATAL, "Buffer allocation failed.");
        errno = xioctlErrno;

        return ret;
    }

    pStream->nBuffers = numBuffers;
    pStream->v4l2Buffers.resize(v4l2ReqBuf.count);
    LogOutput(TRACE_LEVEL_INFORMATION, "Allocated %zu buffers of type V4L2_MEMORY_MMAP", v4l2ReqBuf.count);

    for (unsigned int i = 0U; i < v4l2ReqBuf.count; ++i)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        ret = xioctl(pStream->fdV4L2, VIDIOC_QBUF, &buf);
        if (ret < 0)
        {
            int xioctlErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Queuing of buffer failed.");
            errno = xioctlErrno;

            return ret;
        }

        // Map V4L2 buffer to user space memory
        pStream->v4l2Buffers[i].length = buf.length;
        pStream->v4l2Buffers[i].start = mmap(NULL, buf.length, PROT_READ, MAP_SHARED, pStream->fdV4L2, buf.m.offset);
        if (MAP_FAILED == pStream->v4l2Buffers[i].start)
        {
            int mmapErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Mapping to user space memory failed.");
            errno = mmapErrno;

            return -1;
        }
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
///
int FreeV4L2Buffer(StreamData* pStream)
{
    int ret = 0;

    for (std::vector<V4L2Buffer>::iterator it = pStream->v4l2Buffers.begin(); it != pStream->v4l2Buffers.end(); ++it)
    {
        // Unmap V4L2 buffer from user space memory
        if (-1 == munmap(it->start, it->length))
        {
            int munmapErrno = errno;
            LogOutput(TRACE_LEVEL_FATAL, "Unmapping from user space memory failed.");
            errno = munmapErrno;
            ret = -1;
        }
    }

    pStream->v4l2Buffers.clear();

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
///
int QueueV4L2Buffer(StreamData* pStream, size_t index)
{
    // Queue already done in prepare
    // on memory mapped mode
    //
    return 0;
}


