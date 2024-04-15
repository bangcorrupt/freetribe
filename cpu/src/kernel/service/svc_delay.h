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
 * @file    svc_delay.h
 *
 * @brief   Public interface for delay.c.
 *
 */

#ifndef SVC_DELAY_H
#define SVC_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros and Definitions ---------------------------------------*/

typedef struct {
    uint32_t start_time;
    uint32_t delay_time;
    uint32_t elapsed_cycles;
    uint32_t elapsed_us;
    bool expired;
} t_delay_state;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void delay_init(void);
bool delay_us(t_delay_state *state);
/* bool delay_us(uint32_t *start_count, uint32_t delay_time); */
void delay_block_us(uint32_t time);
void delay_block_ms(uint32_t time);
uint32_t delay_get_current_count(void);
uint32_t delay_get_elapsed_cycles(uint32_t start_count);
void delay_cycles(uint32_t count);
bool delay_ready(void);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
