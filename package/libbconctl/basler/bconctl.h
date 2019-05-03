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
 * @file    bconctl.h
 *
 * @brief   Header for public libbconctl interface
 *
 * @author  Björn Rennfanz
 *
 * @date    06.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */

#ifndef _BASLER_BCONCTL_H
#define _BASLER_BCONCTL_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Enumeration for Carrier Card LEDs
 *
 * These LEDs can be found on Basler's dart BCON for LVDS Development Kit carrier card.
 */
typedef enum {
    bconctl_led_user0 = 0, /**< Enum for user LED zero. */
    bconctl_led_user1 = 1, /**< Enum for user LED one. */
    bconctl_led_user2 = 2  /**< Enum for user LED two. */
} bconctl_led_user_t;

/* Forward declaration of trggen module context */
typedef struct bconctl_trggen_context bconctl_trggen_ctx_t;


/**
 * @brief Opens the trigger generator.
 * @return Returns a pointer to bconctl_trggen_context struct or NULL
 *         if an error occurred (in which case, errno is set appropriately).
 */
bconctl_trggen_ctx_t * bconctl_trggen_open(void);

/**
 * @brief Closes the trigger generator.
 * @param[in] ctx Pointer to a trigger generator context.
 * @return Returns 0 for success or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_close(bconctl_trggen_ctx_t *ctx);

/**
 * @brief Gets the minimum period for trigger generation in milliseconds.
 * @param[in] ctx Pointer to a trigger generator context.
 * @return Returns the minimum period for trigger generation in milliseconds
 *         or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_get_minimum_pulse_period_ms(const bconctl_trggen_ctx_t *ctx);

/**
 * @brief Gets the maximum period for trigger generation in milliseconds.
 * @param[in] ctx Pointer to a trigger generator context.
 * @return Returns the maximum period for trigger generation in milliseconds
 *         or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_get_maximum_pulse_period_ms(const bconctl_trggen_ctx_t *ctx);

/**
 * @brief Sets pulse-forming parameters for trigger generator.
 * @param[in] ctx Pointer to a trigger generator context.
 * @param[in] period_ms Period of the currently generated pulses in milliseconds.
 * @param[in] duration_ms Duration (signal high) of the currently generated pulses in milliseconds.
 * @return Returns 0 for success or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_set_pulse(const bconctl_trggen_ctx_t *ctx, unsigned int period_ms, unsigned int duration_ms);

/**
 * @brief Gets the current pulse forming parameters of trigger generator.
 * @param[in] ctx Pointer to a trigger generator context.
 * @param[out] period_ms Period of the currently generated pulses in milliseconds.
 * @param[out] duration_ms Duration (signal high) of the currently generated pulses in milliseconds.
 * @return Returns 0 for success or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_get_pulse(const bconctl_trggen_ctx_t *ctx, unsigned int *period_ms, unsigned int *duration_ms);

/**
 * @brief Starts pulse generation of trigger generator.
 * @param[in] ctx Pointer to a trigger generator context.
 * @return Returns 0 for success or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_start(const bconctl_trggen_ctx_t *ctx);

/**
 * @brief Stops pulse generation of trigger generator.
 * @param[in] ctx Pointer to a trigger generator context.
 * @return Returns 0 for success or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_stop(const bconctl_trggen_ctx_t *ctx);

/**
 * @brief Gets status of trigger generator.
 * @param[in] ctx Pointer to a trigger generator context.
 * @return Returns 0 for stopped state, 1 for started state or -1 if an
 *         error occurred (in which case, errno is set appropriately).
 */
int bconctl_trggen_status(const bconctl_trggen_ctx_t *ctx);


/**
 * @brief Enables power supply for BCON camera.
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_camera_power_on(void);

/**
 * @brief Disables power supply for BCON camera.
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_camera_power_off(void);

/**
 * @brief Gets status of power supply for BCON camera.
 * @return Returns 0 for power supply disabled, 1 for power supply enabled
 *         or -1 if an error occurred (in which case, errno is set appropriately).
 */
int bconctl_camera_power_status(void);


/**
 * @brief Sets the address select pin of BCON camera.
 * @param [in] i2c_id The value (0 or 1) of the address select pin for BCON camera.
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_camera_address_select(int i2c_id);

/**
 * @brief Gets the value of address select pin for BCON camera.
 * @return Returns the value of the address select pin (0 or 1) or -1 if
 *         an error occurred (in which case, errno is set appropriately).
 */
int bconctl_camera_address_status(void);


/**
 * @brief Resets all BCON cameras on I2C bus by toggling the address select pin.
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_camera_reset(void);


/**
 * @brief Enables user LED on carrier card.
 * @param [in] led ID of selected user LED on carrier card.
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_board_led_on(bconctl_led_user_t led);

/**
 * @brief Disables user LED on carrier card.
 * @param [in] led ID of selected user LED on carrier card.
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_board_led_off(bconctl_led_user_t led);

/**
 * @brief Gets status of user LED on carrier card.
 * @param [in] led ID of selected user LED on carrier card.
 * @return Returns 0 for disabled LED, 1 for enabled LED or -1 if
 *         an error occurred (in which case, errno is set appropriately).
 */
int bconctl_board_led_status(bconctl_led_user_t led);

/**
 * @brief Sets kernel trigger for user LEDs on carrier card.
 * @param [in] led ID of selected user LED on carrier card.
 * @param [in] ledtrg Null-terminated string with kernel trigger name. Examples:
 * - "heartbeat" - Selected LED blinks according to the CPU activity.
 * - "mmc0" - Selected LED blinks every time mmc0 access occurs (try with sync command to see it blinking).
 * - "none" - Disables the kernel LED trigger mode.
 *
 * For more information about LED trigger modes, see: @link https://www.kernel.org/doc/Documentation/leds @endlink
 * @return Returns 0 for success or -1 if an error occurred
 *         (in which case, errno is set appropriately).
 */
int bconctl_board_led_trigger_select(bconctl_led_user_t led, const char *ledtrg);

#ifdef __cplusplus
}
#endif

#endif /* _BASLER_BCONCTL_H */
