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
 * @file    knl_events.h
 *
 * @brief   Public API for kernel event queue.
 *
 */

#ifndef KNL_EVENTS_H
#define KNL_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "ft_error.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef struct {
    uint8_t id;
    uint16_t len;
    uint8_t *data;

} t_event;

typedef enum {
    KNL_EVENT_UART_DATA_RX,
    KNL_EVENT_UART_DATA_TX,

    KNL_EVENT_MIDI_CC_RX,
    KNL_EVENT_MIDI_CC_TX,

    KNL_EVENT_COUNT,
} e_event_id;

typedef void (*t_listener)(const t_event *);

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void knl_event_task(void);

t_status knl_event_publish(t_event *event);
t_status knl_event_subscribe(e_event_id id, t_listener listener);

void knl_enter_critical(void);
void knl_exit_critical(void);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
