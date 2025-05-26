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

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef int (*t_syscall)(void *);

typedef t_syscall *(*t_get_syscalls)(void);

typedef struct {
    uint16_t x;
    uint16_t y;
    bool state;
} t_pixel;

typedef struct {
    t_led_index index;
    uint8_t value;
} t_led;

typedef struct {
    t_delay_state state;
    uint32_t time;
} t_delay;

typedef enum {
    SYSCALL_PUT_PIXEL,
    SYSCALL_PRINT,
    SYSCALL_SET_LED,
    SYSCALL_INIT_DELAY,
    SYSCALL_TEST_DELAY,
    SYSCALL_SHUTDOWN,

    SYSCALL_COUNT,
} e_syscall;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

t_syscall *knl_get_syscalls(void);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
