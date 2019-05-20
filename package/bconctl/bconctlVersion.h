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
 * @file    bconctlVersion.h
 *
 * @brief   Header for bconctl version information
 *
 * @author  Björn Rennfanz
 *
 * @date    15.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#pragma once

#define BCONCTL_VERSION_MAJOR           1
#define BCONCTL_VERSION_MINOR           0
#define BCONCTL_VERSION_SUBMINOR        0

#define BCONCTL_VERSIONSTRING_MAJOR     "1"
#define BCONCTL_VERSIONSTRING_MINOR     "0"
#define BCONCTL_VERSIONSTRING_SUBMINOR  "0"

#if (defined(_DEBUG) || defined(DEBUG))
#   define BCONCTL_VERSIONSTRING_DEBUGEXTENSION "debug"
#else
#   define BCONCTL_VERSIONSTRING_DEBUGEXTENSION "release"
#endif

#define BCONCTL_VERSION_STRING "version " BCONCTL_VERSIONSTRING_MAJOR "." BCONCTL_VERSIONSTRING_MINOR "." BCONCTL_VERSIONSTRING_SUBMINOR "-" BCONCTL_VERSIONSTRING_DEBUGEXTENSION


