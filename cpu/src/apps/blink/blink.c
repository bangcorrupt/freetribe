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

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

// 0.5 seconds in microseconds.
#define DELAY_TIME 500000

/*----- Static variable definitions ----------------------------------*/

static t_delay g_blink_delay;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
 * Initialise a delay.
 * Status is assumed successful.
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {

    // Initialise Freetribe API.
    ft_init();

    // Initialise delay.
    g_blink_delay.delay_time = DELAY_TIME;

    ft_start_delay(&g_blink_delay);

    return SUCCESS;
}

/**
 * @brief   Run application.
 *
 * Test if 0.5 seconds have passed  since we started the delay.
 * If so, toggle an LED and restart delay.
 */
void app_run(void) {

    // Wait for delay.
    if (ft_delay(&g_blink_delay)) {

        // Toggle LED.
        ft_toggle_led(LED_TAP);

        // Reset start time.
        ft_start_delay(&g_blink_delay);
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
