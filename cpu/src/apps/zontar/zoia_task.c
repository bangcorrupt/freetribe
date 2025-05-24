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
 * @file    zoia_task.c
 *
 * @brief   Task to control ZOIA via MIDI.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"
#include "zoia_queue.h"

/*----- Macros -------------------------------------------------------*/

// 1 ms in microseconds.
#define DELAY_TIME 1000

#define ZOIA_BUTTON_BACK 0x2a

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    STATE_INIT,
    STATE_RUN,
    STATE_ERROR,
    STATE_DELAY,
} e_zoia_task_state;

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _zoia_init(void);
static void _event_parse(t_zoia_event *event);

/*----- Extern function implementations ------------------------------*/

void zoia_task(void) {

    static e_zoia_task_state state = STATE_INIT;

    static t_zoia_event event;

    static t_delay_state delay;

    switch (state) {

    // Initialise zoia task.
    case STATE_INIT:

        if (_zoia_init() == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:

        if (zoia_dequeue(&event) == SUCCESS) {

            if (event.value == ZOIA_BUTTON_BACK && event.type == ZOIA_RELEASE) {
                state = STATE_DELAY;

            } else {
                _event_parse(&event);
            }

        } else {
            state = STATE_ERROR;
        }

        break;

    case STATE_DELAY:

        // Initialise delay.
        ft_start_delay(&delay, DELAY_TIME);

        // Wait for delay.
        if (ft_delay(&delay)) {

            // Parse delayed event.
            _event_parse(&event);

            // Reset start time.
            ft_start_delay(&delay, DELAY_TIME);

            // Move back to RUN state.
            state = STATE_RUN;
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
}

/*----- Static function implementations ------------------------------*/

static t_status _zoia_init(void) {

    t_status result = TASK_INIT_ERROR;

    zoia_queue_init();

    result = SUCCESS;

    return result;
}

static void _event_parse(t_zoia_event *p_event) {

    switch (p_event->type) {

    case ZOIA_BYPASS:
        break;

    case ZOIA_PRESS:
        break;

    case ZOIA_RELEASE:
        break;

    case ZOIA_TURN:
        break;

    default:
        break;
    }
}

/*----- End of file --------------------------------------------------*/
