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

/// TODO: This should be at device layer.
///         Prefix function names.

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

/// TODO: Init functions should return status code.
//
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

/// TODO: I think this only works if called at least once per second.
///         This should be ok for most reasonable use cases.
///
///         Could maybe increase DELAY_PERIOD to allow
///         more time before overflow if needed.
///
///         Check if timer peripheral has overflow flag.
///
///         May cause issues when debugging,
///         check timer emulation mode.
//
bool delay_us(t_delay_state *state) {

    uint32_t current_count = 0;
    uint32_t delta = 0;

    /// TODO: Test state->expired.

    if (!state->expired) {

        if (state->elapsed_us < state->delay_time) {

            current_count = delay_get_current_count();

            if (current_count >= state->start_time) {
                delta = current_count - state->start_time;

            } else {
                delta = (DELAY_PERIOD - state->start_time) + current_count + 1;
            }

            state->elapsed_cycles += delta;

            /// TODO: Optimise.
            //
            while (state->elapsed_cycles >= CYCLES_PER_US) {
                state->elapsed_us++;
                state->elapsed_cycles -= CYCLES_PER_US;
            }

            state->start_time = current_count;

        } else {
            state->elapsed_us = 0;
            state->elapsed_cycles = 0;
            state->expired = true;
        }
    }

    /// TODO: Do we need 'expired' in the struct as well as return value?
    //
    return state->expired;
}

void delay_start(t_delay_state *state, uint32_t time) {

    state->start_time = delay_get_current_count();
    state->delay_time = time;
    state->elapsed_cycles = 0;
    state->elapsed_us = 0;
    state->expired = false;
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
