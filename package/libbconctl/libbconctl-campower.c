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
 * @file    libbconctl-campower.c
 *
 * @brief   Implementation of libbconctl camera power functions
 *
 * @author  Thomas Köller
 * @author  Björn Rennfanz
 *
 * @date    24.02.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#include <stdlib.h>
#include <basler/bconctl.h>

#define CMD(arg) "echo " #arg " >/run/bconctl/gpio/gpio_camera_power/value"

///////////////////////////////////////////////////////////////////////////
// Enable the camera power pin by setting gpio
int bconctl_camera_power_on(void)
{
    static const char cmd[] = CMD(1);
    return system(cmd);
}

///////////////////////////////////////////////////////////////////////////
// Disable the camera power pin by setting gpio
int bconctl_camera_power_off(void)
{
    static const char cmd[] = CMD(0);
    return system(cmd);
}

///////////////////////////////////////////////////////////////////////////
// Get the status of the camera power enable gpio pin
int bconctl_camera_power_status()
{
    static const char cmd[] = "/usr/bin/test \"`cat /run/bconctl/gpio/gpio_camera_power/value`\" != \"1\"";
    return system(cmd);
}
