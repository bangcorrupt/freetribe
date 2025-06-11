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

                       Copyright bangcorrupt 2023

----------------------------------------------------------------------*/

/**
 * @file    test_button.c
 *
 * @brief   Test buttons.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

#include "gui_task.h"
#include "test_task.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } e_test_state;

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_status test_button(void) {

    static e_test_state state = STATE_INIT;

    t_status result = ERROR;

    switch (state) {

    // Initialise test.
    case STATE_INIT:

        ft_print("Button test:");
        ft_print("Press [Play] to confirm pass.");
        gui_print(8, 32, "Press [Play].");

        state = STATE_RUN;
        break;

    case STATE_RUN:

        if (test_confirmed()) {
            ft_print("Button test passed.");

            result = SUCCESS;
            reset_test();
        }
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }

    return result;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
