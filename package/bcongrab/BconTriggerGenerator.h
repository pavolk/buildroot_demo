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
 * @file    BconTriggerGenerator.h
 *
 * @brief   Helper class for trigger generator in libbconctl
 *
 * @author  Rüdiger Köpke
 *
 * @date    16.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */


#pragma once

// Include header to use the BCON control library.
#include <basler/bconctl.h>

#include <stdio.h>      // for fprintf()

/******************************************************************************/
// Helper class for trigger generator in libbconctl.
class BconTriggerGenerator
{
public:
    BconTriggerGenerator() : m_ctxt(NULL) {};
    ~BconTriggerGenerator() { Close(); };

// set non-copyable
private:
    BconTriggerGenerator(const BconTriggerGenerator&);
    BconTriggerGenerator& operator=(const BconTriggerGenerator&);

public:
    bool Start(int period_ms, int duration_ms) {
        if (!Open()) {
            return false;
        }
        if (bconctl_trggen_set_pulse(m_ctxt, period_ms, duration_ms) < 0) {
            fprintf(stderr, "Failed to set pulse period/duration: %s\n", strerror(EINVAL));
            return false;
        }
        if (bconctl_trggen_start(m_ctxt) < 0) {
            fprintf(stderr, "Failed to start trigger generator: %s\n", strerror(errno));
            return false;
        }
        return true;
    }

    bool Stop() {
        if (!Open()) {
            return false;
        }
        if (bconctl_trggen_stop(m_ctxt) < 0) {
            fprintf(stderr, "Failed to stop trigger generator: %s\n", strerror(errno));
            return false;
        }
        return true;
    }

private:
    bool Open() {
        if (!m_ctxt) {
            m_ctxt = bconctl_trggen_open();
            if (!m_ctxt) {
                fprintf(stderr, "Failed to open trigger generator: %s\n", strerror(errno));
                return false;
            }
        }
        return true;
    }

    void Close() {
        if (m_ctxt) {
            bconctl_trggen_close(m_ctxt);
            m_ctxt = NULL;
        }
    }

private:
    struct bconctl_trggen_context* m_ctxt;
};
