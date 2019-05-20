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
 * @file    MyBconAdapterZynqSimpleMutex.h
 *
 * @brief   Simple portable critical section/mutex implementation
 *
 * @author  Björn Rennfanz
 *
 * @date    17.11.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#ifndef MYBCONADAPTERZYNQSIMPLEMUTEX_H_
#define MYBCONADAPTERZYNQSIMPLEMUTEX_H_

#include <pthread.h>

namespace MyBconAdapterZynq
{
    class CSimpleMutex
    {
    public:
        CSimpleMutex()
        {
            // Create a recursive mutex
            ::pthread_mutexattr_t attr;
            ::pthread_mutexattr_init(&attr);
            ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

            ::pthread_mutex_init(&m_mutex, &attr);

            ::pthread_mutexattr_destroy(&attr);
        }

        ~CSimpleMutex()
        {
            ::pthread_mutex_destroy(&m_mutex);
        }

    public:
        void Lock()
        {
            ::pthread_mutex_lock(&m_mutex);
        }

        void Unlock()
        {
            ::pthread_mutex_unlock(&m_mutex);
        }

        pthread_mutex_t* GetMutexPtr()
        {
            return &m_mutex;
        }

    protected:
        pthread_mutex_t m_mutex;
    };

    // Simple auto lock class for CSimpleMutex
    class CScopedSimpleLock
    {
    public:
        CScopedSimpleLock(CSimpleMutex& m)
            :m_mutex(&m)
        {
            m_mutex->Lock();
        }

        ~CScopedSimpleLock()
        {
            Leave();
        }

        void Leave()
        {
            if(m_mutex)
            {
                m_mutex->Unlock();
                m_mutex = 0;
            }
        }

    private:
        // Non copyable
        CScopedSimpleLock(const CScopedSimpleLock&);
        CScopedSimpleLock& operator=(const CScopedSimpleLock&);

    private:
        CSimpleMutex* m_mutex;
    };
};

#endif /* MYBCONADAPTERZYNQSIMPLEMUTEX_H_ */
