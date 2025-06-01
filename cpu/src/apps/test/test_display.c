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
 * @file    test_display.c
 *
 * @brief   Test display.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

#include "gui_task.h"
#include "test_task.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    STATE_SET_INIT,
    STATE_SET_RUN,
    STATE_CLEAR_INIT,
    STATE_CLEAR_RUN,
    STATE_RESULT,
    STATE_ERROR
} e_test_state;

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_status test_display(void) {

    static e_test_state state = STATE_SET_INIT;

    static bool passed_set;
    static bool passed_clear;

    t_status result = ERROR;

    switch (state) {

    case STATE_SET_INIT:

        ft_fill_frame(0, 0, 127, 63, 0);
        ft_print("Press [Play] if all pixels set.");

        state = STATE_SET_RUN;

    case STATE_SET_RUN:
        if (test_confirmed()) {

            passed_set = true;
            reset_test();

            state = STATE_CLEAR_INIT;
        }
        break;

    case STATE_CLEAR_INIT:

        ft_fill_frame(0, 0, 127, 63, 1);
        ft_print("Press [Play] if all pixels clear.");

        state = STATE_CLEAR_RUN;

    case STATE_CLEAR_RUN:
        if (test_confirmed()) {

            passed_clear = true;
            reset_test();

            state = STATE_RESULT;
        }
        break;

    case STATE_RESULT:

        if (passed_set && passed_clear) {
            result = SUCCESS;
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
