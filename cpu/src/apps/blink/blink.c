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
#include "svc_delay.h"

/*----- Macros and Definitions ---------------------------------------*/

<<<<<<< HEAD
// 1 second in microseconds.
=======
// 0.5 seconds in microseconds.
>>>>>>> dev
#define DELAY_TIME 500000

/*----- Static variable definitions ----------------------------------*/

<<<<<<< HEAD
// static uint32_t g_start_time;
=======
>>>>>>> dev
static t_delay_state g_blink_delay;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
<<<<<<< HEAD
 * Store the current value of the delay timer in a
 * static global variable. Status is assumed successful.
=======
 * Initialise a delay.
 * Status is assumed successful.
>>>>>>> dev
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {

<<<<<<< HEAD
    // Set start time.
    // g_start_time = ft_get_delay_current();
=======
    // Initialise delay.
>>>>>>> dev
    delay_start(&g_blink_delay, DELAY_TIME);

    return SUCCESS;
}

/**
 * @brief   Run application.
 *
<<<<<<< HEAD
 * Test if 1 second has passed  since we last set `start_time`.
 * If so, toggle an LED and reset `start_time`.
=======
 * Test if 0.5 seconds have passed  since we started the delay.
 * If so, toggle an LED and restart delay.
>>>>>>> dev
 */
void app_run(void) {

    /// TODO:  Update to use new delay_state.
    // Wait for delay.
<<<<<<< HEAD
    // if (ft_delay(g_start_time, DELAY_TIME)) {
=======
>>>>>>> dev
    if (ft_delay(&g_blink_delay)) {

        // Toggle LED.
        ft_toggle_led(LED_TAP);

        // Reset start time.
<<<<<<< HEAD
        // g_start_time = ft_get_delay_current();
=======
>>>>>>> dev
        delay_start(&g_blink_delay, DELAY_TIME);
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
