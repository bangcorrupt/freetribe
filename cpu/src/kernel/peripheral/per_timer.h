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
 * @file    per_timer.h
 *
 * @brief   Header file for per_tiomer.c.
 *
 */

#ifndef PER_TIMER_H
#define PER_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "hal_timer.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef struct {
    uint32_t base_addr;
    uint32_t mode;
    uint32_t period;
    uint32_t int_flags; // See hal_timer.h.
    uint32_t int_chan;
    void (*p_isr)(void);
} t_timer_config;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void timer_init(t_timer_config config);

uint32_t timer_count_get(uint32_t base_addr);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
