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
 * @file    zoia_queue.h
 *
 * @brief   Public API for ZOIA controller event queue..
 *
 */

#ifndef ZOIA_QUEUE_H
#define ZOIA_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    ZOIA_EVENT_BYPASS,
    ZOIA_EVENT_PRESS,
    ZOIA_EVENT_RELEASE,
    ZOIA_EVENT_TURN,

    ZOIA_EVENT_COUNT
} e_zoia_event;

typedef struct {
    e_zoia_event type;
    uint8_t value;
} t_zoia_event;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

t_status zoia_queue_init(void);
t_status zoia_enqueue(t_zoia_event *event);
t_status zoia_dequeue(t_zoia_event *event);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
