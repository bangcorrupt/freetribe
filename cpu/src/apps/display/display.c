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
 * @file    display.c
 *
 * @brief   Example application for Freetribe display.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

#define TRIGGER_MODE_CONTINUOUS 1

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_pad_pressure;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _trigger_callback(uint8_t pad, uint8_t vel, bool state);

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

    ft_register_panel_callback(TRIGGER_EVENT, _trigger_callback);
    ft_set_trigger_mode(TRIGGER_MODE_CONTINUOUS);

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) {

    bool pixel_state = 0;
    uint8_t y_coord = 32;

    uint8_t i;

    for (i = 0; i < 128; i++) {

        if (i <= g_pad_pressure) {
            pixel_state = 1;
        } else {
            pixel_state = 0;
        }

        ft_put_pixel(i, y_coord, pixel_state);
    }
}

/*----- Static function implementations ------------------------------*/

static void _trigger_callback(uint8_t pad, uint8_t vel, bool state) {

    switch (pad) {

    case 0:
        g_pad_pressure = vel;
        break;

    default:
        break;
    }
}

/*----- End of file --------------------------------------------------*/
