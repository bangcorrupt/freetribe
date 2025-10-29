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
 * @file    shutdown.c
 *
 * @brief   Example app showing how to shutdown the system.
 */

/*----- Includes -----------------------------------------------------*/

#include "per_gpio.h"

#include "freetribe.h"

/*----- Macros -------------------------------------------------------*/

// Bank 7 pin 15.
#define GPIO_POWER_BUTTON 128

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {

    t_status status = ERROR;

    // Nothing to do here.

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 *
 * Test if GPIO index 128 (bank 7 pin 15) is low, if so, shutdown.
 */
void app_run(void) {

    if (per_gpio_get_indexed(GPIO_POWER_BUTTON) == 0) {

        ft_shutdown();

        // Should never reach here.
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
