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

/* Original work by Texas Instruments, modified by bangcorrupt 2023. */

/*
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file per_ddr.c
 *
 * @brief  Initialise and terminate DDR.
 */

/*----- Includes -----------------------------------------------------*/

#include "csl_psc.h"
#include "hw_ddr2_mddr.h"
#include "hw_syscfg1_AM1808.h"
#include "hw_types.h"
#include "soc_AM1808.h"

#include "startup.h"

#include "per_ddr.h"

/*----- Macros -------------------------------------------------------*/

// DDR timing values/refresh rates
#define DDR2_SDTIMR1 0x20923249
#define DDR2_SDTIMR2 0x3c11c722
#define DDR2_SDRCR 0xc0000492
#define DDR2_SDRCR_CLEAR 0xc0000000
#define DDR2_SDCR 0x0813ca22
#define VTPIO_CTL_HIGH 0x00080000
#define DDR2_MDDR_DRPYC1R_READ_LAT 0x6

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/*
 * @brief Initialise and configure DDR.
 *
 * @param None.
 *
 * @return None.
 *
 */
void per_ddr_init(void) {

    // Power up DDR controller.
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_DDR2_MDDR, 0,
                     PSC_MDCTL_NEXT_ENABLE);

    if (HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &
        SYSCFG1_VTPIO_CTL_POWERDN) {

        // Set IOPWRDN bit, powerdown enable mode
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) |=
            SYSCFG1_VTPIO_CTL_IOPWRDN;

        // Clear POWERDN bit (enable VTP)
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &=
            ~(SYSCFG1_VTPIO_CTL_POWERDN);

        // Set CLKRZ bit
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) |= SYSCFG1_VTPIO_CTL_CLKRZ;

        // Clear CLRKZ bit
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &=
            ~SYSCFG1_VTPIO_CTL_CLKRZ;

        // CLKRZ bit should be low at least for 2ns
        delay(4);

        // Set CLKRZ bit
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) |= SYSCFG1_VTPIO_CTL_CLKRZ;
        // Poll ready bit in VTPIO_CTL Untill it is high

        while (
            !((HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) & VTPIO_CTL_HIGH) >>
              15))
            ;
        // Set Lock bit for static mode
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) |= SYSCFG1_VTPIO_CTL_LOCK;

        // set PWRSAVE bit to save Power
        HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) |=
            SYSCFG1_VTPIO_CTL_PWRSAVE;
        // VTP Calibration ends
    }

    // Set BOOTUNLOCK
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDCR) |= DDR2_MDDR_SDCR_BOOTUNLOCK;

    // Set peripheral bus burst priority
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_PBBPR) = 0x20;

    // Set EXT_STRBEN and PWRDNEN bit of DDR PHY control register,
    // assign desired value to the RL bit
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_DRPYC1R) =
        DDR2_MDDR_DRPYC1R_EXT_STRBEN | DDR2_MDDR_DRPYC1R_PWRDNEN |
        DDR2_MDDR_DRPYC1R_READ_LAT;

    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDCR) = DDR2_SDCR;
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDTIMR1) = DDR2_SDTIMR1;
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDTIMR2) = DDR2_SDTIMR2;

    // CLEAR TIMINGUNLOCK
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDCR) &= ~DDR2_MDDR_SDCR_BOOTUNLOCK;

    //  IBANK_POS set to 0 so this register does not apply
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDCR2) = DDR2_MDDR_SDCR_IBANK_ONE;

    // SET the refreshing rate
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDRCR) = DDR2_SDRCR;

    // SyncReset the Clock to SDRAM
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_DDR2_MDDR, 0,
                     PSC_MDSTAT_STATE_SYNCRST);

    // Enable clock to SDRAM
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_DDR2_MDDR, 0,
                     PSC_MDCTL_NEXT_ENABLE);

    // Disable Self refresh rate
    HWREG(SOC_DDR2_0_CTRL_REGS + DDR2_MDDR_SDRCR) &= ~DDR2_SDRCR_CLEAR;
}

/*
 * @brief Return DDR to uninitialised state.
 *        Prevents factory bootloader hanging.
 *
 *  @param None.
 *
 *  @return None.
 */
void per_ddr_terminate(void) {

    // Set POWERDN bit (disable VTP).
    HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) |= SYSCFG1_VTPIO_CTL_POWERDN;

    // Clear Lock bit
    HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &= ~(SYSCFG1_VTPIO_CTL_LOCK);

    // Clear PWRSAVE bit.
    HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &=
        ~(SYSCFG1_VTPIO_CTL_PWRSAVE);

    // Clear IOPWRDN bit, powerdown disable mode.
    HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &=
        ~(SYSCFG1_VTPIO_CTL_IOPWRDN);

    // Clear CLRZ bit
    HWREG(SOC_SYSCFG_1_REGS + SYSCFG1_VTPIO_CTL) &= ~SYSCFG1_VTPIO_CTL_CLKRZ;

    // CLRZ bit should be low at least for 2ns.
    delay(4);
}

uint8_t per_ddr_memtest(void) {

    int i;
    unsigned int *ddr = (unsigned int *)0xc0000000;

    uint8_t result = 0;

    for (i = 0; i < 0x4000000; i += 4) {

        *ddr++ = i;
    }

    for (i = 0; i < 0x4000000; i += 4) {

        if (*ddr++ != i) {
            result = 1;
        }
    }

    return result;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
