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
 * @file    init.c
 *
 * @brief   Initialisation for BF523.
 *
 */

/// TODO: initcode in LDR

/*----- Includes -----------------------------------------------------*/

#include <blackfin.h>
#include <bootrom.h>
#include <builtins.h>

#include "init.h"

/*----- Macros and Definitions ---------------------------------------*/

#define RDIV 0x7d6

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _dma_error_isr(void) __attribute__((interrupt_handler));

/*----- Extern function implementations ------------------------------*/

/// TODO: Can incorrect clock configuration damage processor?
//
// Initialize clocks
void pll_init(void) {

    ADI_SYSCTRL_VALUES sysctl;
    sysctl.uwVrCtl = 0x30b0;
    sysctl.uwPllCtl = 0x2a00;

    // Defaults.
    /* write.uwPllDiv = 0x04; */
    /* write.uwPllLockCnt = 0x200; */

    bfrom_SysControl(SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_VRCTL, &sysctl,
                     NULL);
    ssync();
}

void ebiu_init(void) {

    /// TODO: Do we need this?
    //
    // Straight from the self-test example project
    // Initalize EBIU control registers to enable all banks
    /* *pEBIU_AMBCTL1 = 0xFFFFFF02; */
    /* ssync(); */

    // TODO: Do we need to configure asynchronous memory if it is not used?
    *pEBIU_AMGCTL = CDPRIO | AMBEN_ALL;
    ssync();

    // TODO: Do we need this?
    // Check if already enabled
    /* if (SDRS != ((*pEBIU_SDSTAT) & SDRS)) { */
    /*     return; */
    /* } */

    // SDRAM Refresh Rate Control Register
    *pEBIU_SDRRC = RDIV;

    // SDRAM Memory Bank Control Register
    *pEBIU_SDBCTL = EBCAW_9 | EBSZ_32 | EBE;

    // SDRAM Memory Global Control Register
    *pEBIU_SDGCTL = PSS | TWR_2 | TRCD_2 | TRCD_1 | TRP_2 | TRP_1 | TRAS_4 |
                    TRAS_3 | CL_3 | SCTLE;
    ssync();
}

/// TODO: Move to DMA peripheral driver.
void dma_init(void) {

    // DMA error interrupt IVG7.
    *pSIC_IAR0 |= P1_IVG(7);
    ssync();

    // DMA error interrupt vector.
    *pEVT7 = _dma_error_isr;
    ssync();

    // Enable DMA error interrupt.
    *pSIC_IMASK0 |= IRQ_DMA_ERR0;
    ssync();

    int i;
    // Unmask in the core event processor.
    asm volatile("cli %0; bitset(%0, 7); sti %0; csync;" : "+d"(i));
    ssync();
}

__attribute__((interrupt_handler)) static void _dma_error_isr(void) {

    /// TODO: Check actual source of error and handle.
    ///          Register handler in peripheral initialisation function.
    //
    // Clear interrupt status.
    *pDMA3_IRQ_STATUS = DMA_ERR;
    *pDMA4_IRQ_STATUS = DMA_ERR;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
