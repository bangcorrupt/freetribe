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
 * @file    per_timer.c
 *
 * @brief   Driver for timer peripheral.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "hal_interrupt.h"
#include "hal_timer.h"

#include "per_timer.h"

/*----- Macros -------------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/// TODO: This only supports Timer12, which is fine for our use case.
//
/// TODO: ISR should be part of this peripheral driver,
///         Register callback from device driver above.
//
void timer_init(t_timer_config config) {

    // Set emulation mode FREE.
    TimerEmulationModeSet(config.base_addr, TMR_EMUMGT_FREE);

    // Set timer mode.
    TimerConfigure(config.base_addr, config.mode);

    // Set period.
    TimerPeriodSet(config.base_addr, TMR_TIMER12, config.period);

    // Enable continuous timer.
    TimerEnable(config.base_addr, TMR_TIMER12, TMR_ENABLE_CONT);

    // Configure interrupt in AINTC.
    if (config.p_isr) {

        // Register interrupt service routine.
        IntRegister(SYS_INT_TINT12_0, config.p_isr);

        // Set interrupt channel
        IntChannelSet(SYS_INT_TINT12_0, config.int_chan);

        // Enable system interrupts for timer.
        IntSystemEnable(SYS_INT_TINT12_0);
    }

    // Enable specified interrupts.
    TimerIntEnable(config.base_addr, config.int_flags);

    // Clear all interrupts.
    TimerIntStatusClear(config.base_addr, TMR_INTSTAT12_TIMER_NON_CAPT |
                                              TMR_INTSTAT12_TIMER_CAPT |
                                              TMR_INTSTAT34_TIMER_NON_CAPT |
                                              TMR_INTSTAT34_TIMER_CAPT);
}

uint32_t timer_count_get(uint32_t base_addr) {

    return TimerCounterGet(base_addr, TMR_TIMER12);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
