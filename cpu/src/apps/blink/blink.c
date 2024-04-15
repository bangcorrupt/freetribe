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
 * @file    blink.c
 *
 * @brief   Minimal example application for Freetribe.
 *
 * This example uses a non-blocking delay
 * to toggle an LED at regular intervals.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

// 1 second in microseconds.
#define DELAY_TIME 1000000

/*----- Static variable definitions ----------------------------------*/

static uint32_t g_start_time;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
 * Store the current value of the delay timer in a
 * static global variable. Status is assumed successful.
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {

    // Set start time.
    g_start_time = ft_get_delay_current();

    return SUCCESS;
}

/**
 * @brief   Run application.
 *
 * Test if 1 second has passed  since we last set `start_time`.
 * If so, toggle an LED and reset `start_time`.
 */
void app_run(void) {

    /// TODO:  Update to use new delay_state.
    // Wait for delay.
    if (ft_delay(g_start_time, DELAY_TIME)) {

        // Toggle LED.
        ft_toggle_led(LED_TAP);

        // Reset start time.
        g_start_time = ft_get_delay_current();
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
