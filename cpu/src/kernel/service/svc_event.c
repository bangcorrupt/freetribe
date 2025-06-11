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
 * @file    svc_event.c
 *
 * @brief   Central event queue.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdlib.h>

#include "ring_buffer.h"

#include "svc_event.h"
#include "svc_system.h"

/*----- Macros -------------------------------------------------------*/

#define SVC_EVENT_QUEUE_LEN 0x200
#define SVC_DATA_QUEUE_LEN 0x1000

#define SVC_MAX_EVENT_LISTENER 255

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } e_event_task_state;

typedef struct {
    // Array of function pointers to event listeners.
    t_listener listener[SVC_MAX_EVENT_LISTENER];

    // Maximum subscribed listener.
    uint8_t top;

} t_listener_row;

t_listener_row g_listener_table[SVC_EVENT_COUNT];

/*----- Static variable definitions ----------------------------------*/

// Event queue.
static rbd_t g_event_rbd;
static uint8_t g_event_rbmem[SVC_EVENT_QUEUE_LEN][sizeof(t_event)];

// Data queue.
static rbd_t g_data_rbd;
static uint8_t g_data_rbmem[SVC_DATA_QUEUE_LEN];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _event_init(void);

static t_status _event_enqueue(t_event *event);
static t_status _event_dequeue(t_event *event);
static void _event_dispatch(t_event *event);

/*----- Extern function implementations ------------------------------*/

void svc_event_task(void) {

    static e_event_task_state state = STATE_INIT;

    t_event event;

    switch (state) {

    // Initialise event task.
    case STATE_INIT:
        if (error_check(_event_init()) == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:
        if (error_check(_event_dequeue(&event)) == SUCCESS) {

            _event_dispatch(&event);
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

t_status svc_event_publish(t_event *event) {

    t_status result = ERROR;

    if (event->id < SVC_EVENT_COUNT) {

        result = _event_enqueue(event);
    }

    return result;
}

t_status svc_event_subscribe(e_event_id id, t_listener listener) {

    t_status result = ERROR;

    t_listener_row *row = &g_listener_table[id];

    t_listener *entry;

    /// TODO: This reuses unsubscribed entries,
    ///         but does not reduce row->top.
    ///         event_dispatch will loop until row->top.
    ///         Probably want a linked list instead.

    // Search for first available entry.
    int i;
    for (i = 0; i <= row->top; i++) {

        if (row->listener[i] == NULL) {

            entry = &row->listener[i];
        }
    }

    // If not found, attempt to extend row.
    if (entry == NULL && ++row->top <= SVC_MAX_EVENT_LISTENER) {

        entry = &row->listener[row->top];
    }

    if (entry != NULL) {

        *entry = listener;

        result = SUCCESS;
    }

    return result;
}

/*----- Static function implementations ------------------------------*/

static t_status _event_init(void) {

    t_status result = ERROR;

    // Event queue ring buffer attributes.
    rb_attr_t tx_attr = {sizeof(g_event_rbmem[0]), ARRAY_SIZE(g_event_rbmem),
                         g_event_rbmem};

    // Data queue ring buffer attributes.
    rb_attr_t rx_attr = {sizeof(g_data_rbmem[0]), ARRAY_SIZE(g_data_rbmem),
                         g_data_rbmem};

    // Initialise event queue ring buffers.
    if (ring_buffer_init(&g_event_rbd, &tx_attr) ||
        ring_buffer_init(&g_data_rbd, &rx_attr) == 0) {

        result = SUCCESS;
    }

    return result;
}

static void _event_dispatch(t_event *event) {

    t_listener_row *row = &g_listener_table[event->id];

    int i;
    for (i = 0; i <= row->top; i++) {

        if (row->listener[i] != NULL) {

            row->listener[i](event);
        }
    }

    if (event->data != NULL) {

        free(event->data);
    }
}

t_status _event_enqueue(t_event *event) {

    t_status result = ERROR;

    svc_system_enter_critical();

    result = ring_buffer_put(g_event_rbd, event);

    if (result == SUCCESS) {

        if (event->len > 0 && event->data != NULL) {

            int i;
            for (i = 0; i < event->len; i++) {

                result = ring_buffer_put(g_data_rbd, &event->data[i]);
            }
        }
    }

    svc_system_exit_critical();

    return result;
}

static t_status _event_dequeue(t_event *event) {

    t_status result = ERROR;

    result = ring_buffer_get(g_event_rbd, event);

    if (result == SUCCESS) {

        if (event->len > 0) {

            event->data = malloc(event->len);

            if (event->data != NULL) {

                int i;
                for (i = 0; i < event->len; i++) {

                    result = ring_buffer_get(g_data_rbd, &event->data[i]);
                }
            }
        }
    }

    return result;
}

/*----- End of file --------------------------------------------------*/
