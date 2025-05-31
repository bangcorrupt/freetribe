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
 * @file    svc_event.h
 *
 * @brief   Public API for kernel event queue.
 *
 */

#ifndef SVC_EVENT_H
#define SVC_EVENT_H

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
    SVC_EVENT_TRS_DATA_RX,
    SVC_EVENT_TRS_DATA_TX,

    SVC_EVENT_MCU_DATA_RX,
    SVC_EVENT_MCU_DATA_TX,

    SVC_EVENT_PANEL_ACK,
    SVC_EVENT_HELD_BUTTONS,
    SVC_EVENT_PANEL_BUTTON,
    SVC_EVENT_PANEL_ENCODER,
    SVC_EVENT_PANEL_TRIGGER,
    SVC_EVENT_PANEL_KNOB,
    SVC_EVENT_TOUCHPAD,

    SVC_EVENT_MIDI_CC_RX,
    SVC_EVENT_MIDI_CC_TX,

    SVC_EVENT_PUT_PIXEL,
    SVC_EVENT_FILL_FRAME,

    SVC_EVENT_COUNT,
} e_event_id;

typedef void (*t_listener)(const t_event *);

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void svc_event_task(void);

t_status svc_event_publish(t_event *event);
t_status svc_event_subscribe(e_event_id id, t_listener listener);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
