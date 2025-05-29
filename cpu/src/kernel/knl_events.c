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
 * @file    knl_events.c
 *
 * @brief   Central event queue.
 */

/*----- Includes -----------------------------------------------------*/

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

#define KNL_EVENT_QUEUE_LEN 0x200
#define KNL_DATA_QUEUE_LEN 0x200
#define KNL_EVENT_LEN sizeof(t_event)

/*----- Typedefs -----------------------------------------------------*/

typedef struct {
    uint16_t id;
    uint16_t len;
    uint8_t *data;

} t_event;

/*----- Static variable definitions ----------------------------------*/

// Event queue.
static rbd_t g_event_rbd;
static uint8_t g_event_rbmem[KNL_EVENT_QUEUE_LEN][KNL_EVENT_LEN];

// Data queue.
static rbd_t g_data_rbd;
static uint8_t g_data_rbmem[KNL_DATA_QUEUE_LEN];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void knl_event_enqueue(t_event *event) {

    /// TODO: Test for queue put failure.
    /// TODO: Test for data pointer errors.

    ring_buffer_put(g_event_rbd, event);

    if (event->len > 0 && event->data != NULL) {

        uint8_t *data = event->data;

        int i;
        for (i = 0; i < event->len; i++) {

            ring_buffer_put(g_data_rbd, data++);
        }
    }
}

t_status knl_event_dequeue(t_event *event) {

    t_status result = ERROR;

    if (ring_buffer_get(g_data_rbd, event) == SUCCESS) {

        if (event->len > 0) {

            uint8_t *data = malloc(event->len);
        }
    }

    return result;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
