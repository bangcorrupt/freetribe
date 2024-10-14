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
 * @file    svc_clock.c
 *
 * @brief   Configuration and handling for clock timer.
 */

/// TODO: This should be a device driver.

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "csl_interrupt.h"

#include "per_timer.h"

/*----- Macros -------------------------------------------------------*/

#define CLOCK_TIMER SOC_TMR_2_REGS
#define CLOCK_PERIOD 0x124F7 // 500us

#define CLOCK_MODE                                                             \
    TMR_CFG_32BIT_UNCH_CLK_BOTH_INT & ~TMR_TGCR_TIM34RS & ~TMR_TGCR_PLUSEN

#define CLOCK_INT TMR_INT_TMR12_NON_CAPT_MODE
#define CLOCK_INT_CHAN 5

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

volatile static uint32_t g_clock;

static void (*p_clock_callback)(void) = NULL;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _clock_isr(void);

/*----- Extern function implementations ------------------------------*/

void clock_init(void) {

    t_timer_config clock_cfg = {.base_addr = CLOCK_TIMER,
                                .mode = CLOCK_MODE,
                                .period = CLOCK_PERIOD,
                                .int_flags = CLOCK_INT,
                                .int_chan = CLOCK_INT_CHAN,
                                .p_isr = _clock_isr};

    timer_init(clock_cfg);
}

void clock_register_callback(void (*callback)(void)) {

    if (callback != NULL) {
        p_clock_callback = callback;
    }
}

uint32_t clock_get(void) { return g_clock; }

/*----- Static function implementations ------------------------------*/

// TODO: ISR should not be at service layer.

void _clock_isr(void) {

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clear interrupt.
    IntSystemStatusClear(SYS_INT_TIMR2_ALL);
#endif
    TimerIntStatusClear(CLOCK_TIMER, CLOCK_INT << 1);

    g_clock++;

    if (p_clock_callback != NULL) {
        (*p_clock_callback)();
    }
}

/*----- End of file --------------------------------------------------*/
