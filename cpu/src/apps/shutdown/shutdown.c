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

#include "freetribe.h"

/*----- Macros -------------------------------------------------------*/

#define BUTTON_EXIT 0x0d

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static bool g_shutdown = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _button_callback(uint8_t index, bool state);

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

    ft_register_panel_callback(BUTTON_EVENT, _button_callback);

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) {

    if (g_shutdown) {

        ft_shutdown();

        // Should never reach here.
    }
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Callback triggered by panel button events.
 *
 * @param[in]   index   Index of button.
 * @param[in]   state   State of button.
 */
void _button_callback(uint8_t index, bool state) {

    switch (index) {

    case BUTTON_EXIT:
        if (state == 1) {
            g_shutdown = true;
        }
        break;

    default:
        break;
    }
}

/*----- End of file --------------------------------------------------*/
