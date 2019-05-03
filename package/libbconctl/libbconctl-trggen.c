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
 * @file    libbconctl-trggen.c
 *
 * @brief   Implementation of libbconctl trigger generator functions
 *
 * @author  Thomas Köller
 * @author  Björn Rennfanz
 *
 * @date    08.12.2016
 *
 * @copyright (c) 2016-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/ioctl.h>

#include <uapi/misc/basler/trggen.h>
#include <basler/bconctl.h>

/*
 * Private definition of trggen context data.
 */
struct bconctl_trggen_context {
    int fd;
    struct trggen_staticdata sdata;
};

///////////////////////////////////////////////////////////////////////////
//
bconctl_trggen_ctx_t * bconctl_trggen_open(void)
{
    bconctl_trggen_ctx_t ctx, *result = NULL;

    ctx.fd = open("/dev/trggen0", O_RDWR);
    if (ctx.fd < 0)
    {
        return NULL;
    }

    if (ioctl(ctx.fd, TRGGEN_GET_SDATA, &ctx.sdata) < 0)
    {
        close(ctx.fd);
        return NULL;
    }

    result = malloc(sizeof ctx);
    if (result)
    {
        *result = ctx;
    }
    else
    {
        close(ctx.fd);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_close(bconctl_trggen_ctx_t *ctx)
{
    if (ctx != NULL)
    {
        // Close file and free
        close(ctx->fd);
        free(ctx);

        return 0;
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
static int get_period_ms(unsigned int clk_hz, unsigned int scale, unsigned int val) __attribute__((const));
static inline int get_period_ms(unsigned int clk_hz, unsigned int scale, unsigned int val)
{
    // Check for devision by zero
    if (clk_hz == 0)
    {
        errno = EINVAL;
        return -1;
    }

    return val * (1000ULL << scale) / clk_hz;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_get_minimum_pulse_period_ms(const bconctl_trggen_ctx_t *ctx)
{
    if (ctx != NULL)
    {
        return get_period_ms(ctx->sdata.clk_hz, ctx->sdata.scale_min, 2U);
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_get_maximum_pulse_period_ms(const bconctl_trggen_ctx_t *ctx)
{
    if (ctx != NULL)
    {
        return get_period_ms(ctx->sdata.clk_hz, ctx->sdata.scale_max, USHRT_MAX);
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_set_pulse(const bconctl_trggen_ctx_t *ctx, unsigned int period_ms, unsigned int duration_ms)
{
    if (ctx != NULL)
    {
        const struct trggen_staticdata *s = &ctx->sdata;
        unsigned int scale, period_val, duration_val;

        if (duration_ms >= period_ms)
        {
            errno = EINVAL;
            return -1;
        }

        /* Compute the smallest prescaler value possible for the requested period_ms. */
        for (scale = s->scale_min; scale <= s->scale_max; ++scale)
        {
            const int x = get_period_ms(s->clk_hz, scale, USHRT_MAX);
            if (x >= period_ms)
            {
                goto scale_found;
            }
        }

        /* Cannot set requested period_ms */
        errno = EINVAL;
        return -1;

    scale_found:
        // Check for possible overflow on shift operation
        if (scale >= 32)
        {
            errno = EINVAL;
            return -1;
        }

        period_val = period_ms * (s->clk_hz / 1000U) / (1U << scale);
        duration_val = duration_ms * (s->clk_hz / 1000U) / (1U << scale);

        if (0 > ioctl(ctx->fd, TRGGEN_SET_SCALE, &scale) ||
            0 > ioctl(ctx->fd, TRGGEN_SET_PERIOD, &period_val) ||
            0 > ioctl(ctx->fd, TRGGEN_SET_DURATION, &duration_val))
        {
            return -1;
        }

        return 0;
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_get_pulse(const bconctl_trggen_ctx_t *ctx, unsigned int *period_ms, unsigned int *duration_ms)
{
    if ((ctx != NULL) && (period_ms != NULL) && (duration_ms != NULL))
    {
        const struct trggen_staticdata *s = &ctx->sdata;
        unsigned int scale, period_val, duration_val;

        if (0 > ioctl(ctx->fd, TRGGEN_GET_SCALE, &scale) ||
            0 > ioctl(ctx->fd, TRGGEN_GET_PERIOD, &period_val) ||
            0 > ioctl(ctx->fd, TRGGEN_GET_DURATION, &duration_val))
        {
            return -1;
        }

        // Check for possible devision by zero
        if (s->clk_hz == 0)
        {
            errno = EINVAL;
            return -1;
        }

        *period_ms = period_val * (1U << scale) / (s->clk_hz / 1000U);
        *duration_ms = duration_val * (1U << scale) / (s->clk_hz / 1000U);

        return 0;
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_stop(const bconctl_trggen_ctx_t *ctx)
{
    if (ctx != NULL)
    {
        return ioctl(ctx->fd, TRGGEN_STOP);
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_start(const bconctl_trggen_ctx_t *ctx)
{
    if (ctx != NULL)
    {
        return ioctl(ctx->fd, TRGGEN_START);
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_trggen_status(const bconctl_trggen_ctx_t *ctx)
{
    if (ctx != NULL)
    {
        unsigned int trggen_is_running;
        if (ioctl(ctx->fd, TRGGEN_IS_RUNNING, &trggen_is_running) < 0)
        {
            return -1;
        }

        return trggen_is_running;
    }

    errno = EINVAL;
    return -1;
}
