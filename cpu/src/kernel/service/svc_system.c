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

/*
 * @file    svc_system.c
 *
 * @brief   System task.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stddef.h>

// TODO: Device layer blocking delay.
//       integrate timers into system service.
#include "svc_delay.h"

#include "dev_board.h"
#include "dev_flash.h"

#include "ft_error.h"

/*----- Macros and Definitions ---------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } t_system_task_state;

/*----- Static variable definitions ----------------------------------*/

static void (*p_print_callback)(char *text) = NULL;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _system_init(void);
static void _system_run(void);

/*----- Extern function implementations ------------------------------*/

void svc_system_task(void) {

    static t_system_task_state state = STATE_INIT;

    switch (state) {

    // Initialise system task.
    case STATE_INIT:
        if (error_check(_system_init()) == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:
        _system_run();
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
}

void svc_system_print(char *text) {

    if (p_print_callback != NULL) {
        (p_print_callback)(text);
    }
}

void svc_system_register_print_callback(void (*callback)(char *)) {

    if (callback != NULL) {
        p_print_callback = callback;
    }
}

/*----- Static function implementations ------------------------------*/

static t_status _system_init(void) {

    t_status result = TASK_INIT_ERROR;

    // Initialise hardware.
    dev_board_init();

    // TODO: How long is necessary?
    //          Are there signals we can test?
    //
    // Give the hardware time to wake up.
    delay_block_us(2000000);

    // TODO: Only necessary if flash is locked.
    //
    // Workaround flash write protect.
    // flash_unlock();

    result = SUCCESS;

    return result;
}

static void _system_run(void) {

    // TODO: Maybe integrate timers into system task
    //       and kick watchdog in run state.
}

/*----- End of file --------------------------------------------------*/
