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
 * @file    knl_syscalls.h
 *
 * @brief   Public API for Freetribe kernel system calls.
 *
 */

#ifndef KNL_SYSCALLS_H
#define KNL_SYSCALLS_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "svc_delay.h"
#include "svc_panel.h"

#include "svc_event.h"

/*----- Macros -------------------------------------------------------*/

#define PTR_GET_SYSCALLS 0x8001fffc

/*----- Typedefs -----------------------------------------------------*/

typedef int (*t_syscall)(void *);

typedef t_syscall *(*t_get_syscalls)(void);

typedef struct {
    uint16_t x;
    uint16_t y;
    bool state;
} t_pixel;

typedef struct {
    uint16_t x_start;
    uint16_t y_start;
    uint16_t x_end;
    uint16_t y_end;
    bool state;
} t_frame;

typedef struct {
    t_led_index index;
    uint8_t value;
} t_led;

typedef struct {
    uint16_t module_id;
    uint16_t index;
    uint32_t value;
} t_param;

typedef struct {
    uint8_t status;
    uint8_t byte_1;
    uint8_t byte_2;
} t_midi_msg;

typedef enum {
    MIDI_STATUS_NOTE_OFF = 0x80,
    MIDI_STATUS_NOTE_ON = 0x90,
    MIDI_STATUS_CC = 0xb0,
} e_midi_status;

typedef enum {
    CALLBACK_TICK,
    CALLBACK_PANEL,
    CALLBACK_MIDI,

    NUM_CALLBACKS,
} e_callback_id;

typedef struct {
    e_callback_id id;
    uint32_t arg;
    void *handler;

} t_callback;

typedef enum {
    SYSCALL_PRINT,
    SYSCALL_PUT_PIXEL,
    SYSCALL_FILL_FRAME,
    SYSCALL_SET_LED,
    SYSCALL_INIT_DELAY,
    SYSCALL_TEST_DELAY,
    SYSCALL_REGISTER_CALLBACK,
    SYSCALL_EVENT_SUBSCRIBE,
    SYSCALL_SEND_MIDI_MSG,
    SYSCALL_SET_MODULE_PARAM,
    SYSCALL_GET_MODULE_PARAM,
    SYSCALL_SET_TRIGGER_MODE,
    SYSCALL_SHUTDOWN,

    NUM_SYSCALLS,
} e_syscall;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

t_syscall *knl_get_syscalls(void);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
