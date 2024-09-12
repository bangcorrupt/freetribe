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
 * @file    svc_systick.c
 *
 * @brief   Configuration and handling for systick timer.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "hal_interrupt.h"

#include "per_timer.h"

/*----- Macros and Definitions ---------------------------------------*/

#define SYSTICK_TIMER SOC_TMR_0_REGS
#define SYSTICK_PERIOD 0x5dbf // 1ms

#define SYSTICK_MODE                                                           \
    TMR_CFG_32BIT_UNCH_CLK_BOTH_INT & ~TMR_TGCR_TIM34RS & ~TMR_TGCR_PLUSEN

#define SYSTICK_INT TMR_INT_TMR12_NON_CAPT_MODE

/// TODO: Single header file with interrupt priorities.
//
//  Lowest priority. SPI0 is 8.
#define SYSTICK_INT_CHAN 9 // 8

/*----- Static variable definitions ----------------------------------*/

volatile static uint32_t g_systick;

static void (*p_systick_callback)(uint32_t systick) = NULL;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _systick_isr(void);

/*----- Extern function implementations ------------------------------*/

void systick_init(void) {

    t_timer_config systick_cfg = {.base_addr = SYSTICK_TIMER,
                                  .mode = SYSTICK_MODE,
                                  .period = SYSTICK_PERIOD,
                                  .int_flags = SYSTICK_INT,
                                  .int_chan = SYSTICK_INT_CHAN,
                                  .p_isr = _systick_isr};

    timer_init(systick_cfg);
}

void systick_register_callback(void (*callback)(uint32_t)) {

    if (callback != NULL) {
        p_systick_callback = callback;
    }
}

uint32_t systick_get(void) { return g_systick; }

/*----- Static function implementations ------------------------------*/

/// TODO: ISR should not be at service layer.
///       Integrate timers into system service.

void _systick_isr(void) {

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clear interrupt.
    IntSystemStatusClear(SYS_INT_TINT12_0);
#endif
    TimerIntStatusClear(SYSTICK_TIMER, SYSTICK_INT << 1);

    /// TODO: Handle overflow (7 weeks at 1ms interval).
    g_systick++;

    if (p_systick_callback != NULL) {
        (*p_systick_callback)(g_systick);
    }
}

/*----- End of file --------------------------------------------------*/
