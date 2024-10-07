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
 * @file    usr_main.c
 *
 * @brief   Freetribe user task.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ft_error.h"

#include "usr_main.h"

/*----- Macros -------------------------------------------------------*/

#define DEBUG_TICK_DIV 1000
#define USER_TICK_DIV 0

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } t_user_task_state;

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _usr_init(void);
static void _usr_run(void);

/*----- Extern function implementations ------------------------------*/

void usr_main_task(void) {

    static t_user_task_state state = STATE_INIT;

    switch (state) {

    // Initialise user task.
    case STATE_INIT:

        if (error_check(_usr_init()) == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:

        _usr_run();
        // Remain in RUN state.
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        // TODO: Record unhandled state.
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

__attribute__((weak)) t_status app_init(void) {
    //
    return 0;
}

__attribute__((weak)) void app_run(void) {
    //
}

/*----- Static function implementations ------------------------------*/

static t_status _usr_init(void) {

    t_status result = ERROR;

    result = app_init();

    return result;
}

static void _usr_run(void) { app_run(); }

/*----- End of file --------------------------------------------------*/
