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
 * @file    libbconctl-led.c
 *
 * @brief   Implementation of libbconctl carrier card LED functions
 *
 * @author  Thomas Köller
 * @author  Björn Rennfanz
 *
 * @date    20.02.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <basler/bconctl.h>

///////////////////////////////////////////////////////////////////////////
//
static int led_user_command(unsigned int led, const char *parm, const char *val)
{
    static const char fmt[] = "/bin/echo '%s' >/run/bconctl/leds/led-user%u/%s";

    const size_t sz = snprintf(NULL, 0U, fmt, val, led, parm);
    char *buf = alloca(sz + 1U);
    sprintf(buf, fmt, val, led, parm);

    return system(buf);
}

///////////////////////////////////////////////////////////////////////////
//
static const bconctl_led_user_t available_bconctl_led_user[] = { bconctl_led_user0, bconctl_led_user1, bconctl_led_user2 };
static int is_led_user_enum(int value)
{
    for (int i = 0; i < (sizeof(available_bconctl_led_user) / sizeof(*available_bconctl_led_user)); i++)
    {
        if ((int)available_bconctl_led_user[i] == value)
        {
            return 1;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_board_led_on(bconctl_led_user_t led)
{
    // Check if inputs are valid
    if (is_led_user_enum((int)led))
    {
        // Enable selected LED
        return led_user_command((int)led, "brightness", "1");
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_board_led_off(bconctl_led_user_t led)
{
    // Check if inputs are valid
    if (is_led_user_enum((int)led))
    {
        // Disable selected LED
        return led_user_command((int)led, "brightness", "0");
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_board_led_status(bconctl_led_user_t led)
{
    // Check if inputs are valid
    if (is_led_user_enum((int)led))
    {
        static const char fmt[] = "/usr/bin/test \"`cat /run/bconctl/leds/led-user%u/brightness`\" != \"1\"";

        const size_t sz = snprintf(NULL, 0U, fmt, (int)led);
        char *buf = alloca(sz + 1U);
        sprintf(buf, fmt, (int)led);

        return system(buf);
    }

    errno = EINVAL;
    return -1;
}

///////////////////////////////////////////////////////////////////////////
//
int bconctl_board_led_trigger_select(bconctl_led_user_t led, const char *ledtrg)
{
    // Check if inputs are valid
    if ((is_led_user_enum((int)led)) && (ledtrg != NULL))
    {
        return led_user_command((int)led, "trigger", ledtrg);
    }

    errno = EINVAL;
    return -1;
}
