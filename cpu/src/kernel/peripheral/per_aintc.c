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
 * @file    per_aint.c
 *
 * @brief   Peripheral driver for ARM Interrupt Controller.
 */

/*----- Includes -----------------------------------------------------*/

#include "hal_interrupt.h"

#include "per_aintc.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void per_aintc_init(void) {

    // Enable interrupt controller.
    IntAINTCInit();

#if NESTED_INTERRUPTS
    // Enable interrupt nesting.
    IntNestModeSet(AINTC_CR_NESTMODE_INDIVIDUAL);
#endif

    // Enable interrupts in CPSR.
    IntMasterIRQEnable();

    // Enable global interrupts in AINTC.
    IntGlobalEnable();

    // Enable host interrupts in AINTC.
    IntIRQEnable();
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
