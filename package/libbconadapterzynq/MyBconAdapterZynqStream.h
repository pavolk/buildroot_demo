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
 * @file    MyBconAdapterZynqStream.h
 *
 * @brief   Header of the BCON adapter stream interface
 *
 * @author  Björn Rennfanz
 *
 * @date    28.11.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#ifndef MYBCONADAPTERZYNQSTREAM_H_
#define MYBCONADAPTERZYNQSTREAM_H_

#include <bconadapter/BconAdapterDefines.h>
#include <bconadapter/BconAdapterStream.h>

#include "MyBconAdapterZynqSimpleMutex.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <list>
#include <vector>
#include <string>

#include <asm/types.h>

#define TrailerExtraLines 4U
#define TrailerVersion    0x00010000U
#define TrailerMagic      0x52544758U

//////////////////////////////////////////////////////////////////////////////
//
struct ImageTrailer
{
    // 1. extra image line
    uint32_t magic;
    uint32_t version;
    // 2. extra image line
    BCONSTATUS status;
    uint32_t pixelFormat;
    // 3. extra image line
    uint64_t blockId;
    // 4. extra image line
    uint64_t timeStamp;
};

//////////////////////////////////////////////////////////////////////////////
//
struct BufferContextData
{
    BufferContextData()
    {
        Clear();
    }

    void Clear()
    {
        status = BCON_E_BUFFER_CANCELLED;
        pBuffer = NULL;
        bufferSize = 0;
        userContext = NULL;
        queued = false;
    }

    void OnBeforeQueue()
    {
        status = BCON_E_BUFFER_CANCELLED;
        queued = true;
    }

    void OnDeQueue()
    {
        status = BCON_E_BUFFER_CANCELLED;
        queued = false;
    }

    BCONSTATUS status;
    void* pBuffer;
    size_t bufferSize;
    const void* userContext;
    bool queued;
    struct ImageTrailer trailer;
};


struct V4L2Buffer
{
    void*  start;
    size_t length;
};

//////////////////////////////////////////////////////////////////////////////
//
struct StreamData
{
    StreamData(BconAdapterStreamHandle myHandleIn = NULL)
        : width(0)
        , height(0)
        , pixelFormatPfncValue(0)
        , pixelsPerClockCycle(0)
        , payloadSize(0)
        , nBuffers(0U)
        , pumpThread(0)
        , pumpThreadRun(false)
        , streamBufferReadyCallback(NULL)
        , streamBufferReadyCallbackContext(NULL)
        , myHandle(myHandleIn)
        , v4l2StreamingEnabled(false)
        , fdV4L2(-1)
        , fdVsrc(-1)
    {
        // Initialize pthread wait condition
        pthread_cond_init(&this->v4l2StreamingStartedCond, NULL);
    }

    ~StreamData()
    {
        // Destroy pthread wait condition
        pthread_cond_destroy(&this->v4l2StreamingStartedCond);
    }

    std::vector<BufferContextData> bufferContexts;
    std::list<BufferContextData*> inputQueue;
    std::list<BufferContextData*> outputQueue;

    std::string deviceID;
    uint32_t width;
    uint32_t height;
    uint32_t pixelFormatPfncValue;
    uint32_t pixelsPerClockCycle;
    size_t payloadSize;/**
     * \file
     * \brief Header of BCON adapter stream for V4L2 on Basler BCON DevKit
     */

    size_t nBuffers;

    MyBconAdapterZynq::CSimpleMutex lock;
    pthread_t pumpThread;
    volatile bool pumpThreadRun;

    BconAdapterStreamBufferReadyCallback streamBufferReadyCallback;
    void* streamBufferReadyCallbackContext;
    BconAdapterStreamHandle myHandle;

    pthread_cond_t v4l2StreamingStartedCond;
    bool v4l2StreamingEnabled;
    int fdV4L2;
    int fdVsrc;
    std::vector<V4L2Buffer> v4l2Buffers;     // used for MMAP io method
};


//////////////////////////////////////////////////////////////////////////////
//
int xioctl(int fd, unsigned long int request, void *arg);


#endif /* MYBCONADAPTERZYNQSTREAM_H_ */
