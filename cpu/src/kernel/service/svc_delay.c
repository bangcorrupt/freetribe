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
 * @file    svc_delay.c
 *
 * @brief   Configuration and handling for delay timer.
 *
 */

/*----- Includes -----------------------------------------------------*/

// TODO: Integrate into svc_system.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "per_timer.h"

#include "svc_delay.h"

/*----- Macros and Definitions ---------------------------------------*/

#define DELAY_TIMER SOC_TMR_3_REGS
#define DELAY_PERIOD 0x8F0D17F // 1s

#define DELAY_MODE                                                             \
    TMR_CFG_32BIT_UNCH_CLK_BOTH_INT & ~TMR_TGCR_TIM34RS & ~TMR_TGCR_PLUSEN

#define CYCLES_PER_US 150 // 150MHz

/*----- Static variable definitions ----------------------------------*/

static bool g_delay_ready = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void delay_init(void) {

    t_timer_config delay_cfg = {.base_addr = DELAY_TIMER,
                                .mode = DELAY_MODE,
                                .period = DELAY_PERIOD,
                                .int_flags = 0,
                                .int_chan = 0,
                                .p_isr = NULL};

    // TODO: Init functions should return True on success.
    timer_init(delay_cfg);

    g_delay_ready = true;
}

bool delay_ready(void) { return g_delay_ready; }

void delay_block_us(uint32_t time) {

    uint32_t start_count = 0;
    uint32_t current_count = 0;
    uint32_t delta = 0;
    uint32_t elapsed_cycles = 0;
    uint32_t elapsed_us = 0;

    start_count = delay_get_current_count();

    while (elapsed_us < time) {

        current_count = delay_get_current_count();

        if (current_count >= start_count) {
            delta = current_count - start_count;

        } else {
            delta = (DELAY_PERIOD - start_count) + current_count + 1;
        }

        elapsed_cycles += delta;

        while (elapsed_cycles >= CYCLES_PER_US) {

            elapsed_us++;
            elapsed_cycles -= CYCLES_PER_US;
        }

        start_count = current_count;
    }
}

bool delay_us(uint32_t start_count, uint32_t delay_time) {

    static uint32_t elapsed_cycles = 0;
    static uint32_t elapsed_us = 0;

    uint32_t current_count = 0;
    uint32_t delta = 0;

    bool expired = false;

    if (elapsed_us < delay_time) {

        current_count = delay_get_current_count();

        if (current_count >= start_count) {
            delta = current_count - start_count;

        } else {
            delta = (DELAY_PERIOD - start_count) + current_count + 1;
        }

        elapsed_cycles += delta;

        // TODO: Optimise.
        while (elapsed_cycles >= CYCLES_PER_US) {
            elapsed_us++;
            elapsed_cycles -= CYCLES_PER_US;
        }

        start_count = current_count;

    } else {
        elapsed_us = 0;
        elapsed_cycles = 0;
        expired = true;
    }

    return expired;
}

void delay_block_ms(uint32_t time) { delay_block_us(time * 1000); }

uint32_t delay_get_current_count(void) { return timer_count_get(DELAY_TIMER); }

// TODO: Get elapsed time.

void delay_cycles(uint32_t count) {
    while (count--)
        ;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
