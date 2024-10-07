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
 * @file    per_pinmux.c
 *
 * @brief   Peripheral driver for configuring pin multiplexing.
 */

/*----- Includes -----------------------------------------------------*/

#include "hw_syscfg0_AM1808.h"
#include "hw_types.h"

#include "soc_AM1808.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/// TODO: Maybe configure pinmux in each peripheral driver.
//        Could save power for apps that don't need some peripherals.
void per_pinmux_init(void) {

    // TODO: #define masks for pinmux.

    // McASP0 Clock, GPIO0 8-9.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(0)) = 0x88111111;

    // GPIO0 0-7.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) = 0x88888888;

    // McASP0 AXR0, AXR1, GPIO1 10-15.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(2)) = 0x11444448;

    // UART0 RX, TX, SPI0 SIMO, ENA, CLK, GPIO8 1-2, 6.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(3)) = 0x44221411;

    // UART1 RX, TX, SPI0 SCS[0], GPIO1 2-5, 7.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(4)) = 0x22888814;

    //  SPI1 SIMO, SOMI, CLK, SCS[0,1], ENA, EMIFA, GPIO2 8.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) = 0x81111111;

    // GPIO2 0-7.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) = 0x88888888;

    // EMIFA Control, GPIO3 8-9, 12-14.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(7)) = 0x88118881;

    // EMIFA Data 8-15.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(8)) = 0x11111111;

    // EMIFA Data 0-7.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(9)) = 0x11111111;

    // SD, GPIO4 0-1.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(10)) = 0x88222222;

    // EMIFA Address 8-15.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(11)) = 0x11111111;

    // EMFIA Address 0-7.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(12)) = 0x11111111;

    // Reset Out / GPIO6 8-14.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) = 0x88888881;

    /// TODO: Are the RMII pins exposed?
    //
    // RMII, GPIO6 6-7.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(14)) = 0x88888888;

    // RMII.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(15)) = 0x00000088;

    // GPIO6 5, GPIO7 10-15.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(16)) = 0x88888880;

    // GPIO7 8-9.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(17)) = 0x00000088;

    // GPIO8 10-15.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(18)) = 0x88888800;

    // GPIO8 0-4, 8-9, RTCK.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) = 0x18888888;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
