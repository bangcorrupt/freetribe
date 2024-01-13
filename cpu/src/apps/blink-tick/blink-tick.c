/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
                  (at your option) any later version.

     Freetribe is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty
        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
          See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.

                       Copyright bangcorrupt 2024

----------------------------------------------------------------------*/

/**
 * @file    blink-tick.c
 *
 * @brief   Example application for Freetribe tick callback.
 *
 * This example uses the tick callback
 * to toggle an LED at regular intervals.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

#define USER_TICK_DIV 0   // User tick for every systick (~1ms).
#define LED_TICK_DIV 1000 // Debug tick per 1000 user ticks (~1s).

/*----- Static variable definitions ----------------------------------*/

static bool g_toggle_led = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _tick_callback(void);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
 * Register a handler for the tick event callback.
 * Status is assumed successful.
 *
 * @return status   Status code indicating success:
 *                  - #SUCCESS
 *                  - #WARNING
 *                  - #ERROR
 */
t_status app_init(void) {

    t_status status = ERROR;

    ft_register_tick_callback(USER_TICK_DIV, _tick_callback);

    return SUCCESS;
}

/**
 * @brief   Run the application.
 *
 * Test a flag set in the tick callback
 * and conditionally toggle an LED.
 */
void app_run(void) {

    if (g_toggle_led) {
        ft_toggle_led(LED_TAP);

        g_toggle_led = false;
    }
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Callback triggered by user tick events.
 *
 * The Freetribe kernel systick ISR sets a flag to
 * trigger the kernel tick callback in the main loop.
 * The kernel tick callback triggers the user tick
 * callback at specified subdivisions.
 * In this example, we count the number of ticks
 * and when it reaches LED_TICK_DIV we set a
 * flag to be tested in the `app_run()` function.
 */
static void _tick_callback(void) {

    static uint16_t led_count;

    if (led_count >= LED_TICK_DIV) {

        g_toggle_led = true;
        led_count = 0;

    } else {
        led_count++;
    }
}

/*----- End of file --------------------------------------------------*/
