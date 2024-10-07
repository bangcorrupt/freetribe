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
 * @file  startup.c
 *
 * @brief Configures the PLL registers to achieve the required Operating
 *        frequency. Power and sleep controller is activated for UART and
 *        Interuppt controller. Interrupt vector is copied to the shared Ram.
 *        After doing all the above, controller is given to the application.
 */

/*----- Includes -----------------------------------------------------*/

#include "hw_ddr2_mddr.h"
#include "hw_pllc_AM1808.h"
#include "hw_syscfg0_AM1808.h"
#include "hw_syscfg1_AM1808.h"
#include "hw_types.h"
#include "soc_AM1808.h"

#include "hal_cp15.h"
#include "hal_psc.h"
#include "hal_syscfg.h"

#include "per_ddr.h"

#include "startup.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static unsigned int const vecTbl[14] = {0xE59FF018,
                                        0xE59FF018,
                                        0xE59FF018,
                                        0xE59FF018,
                                        0xE59FF014,
                                        0xE24FF008,
                                        0xE59FF010,
                                        0xE59FF010,
                                        (unsigned int)start,
                                        (unsigned int)UndefInstHandler,
                                        (unsigned int)SWIHandler,
                                        (unsigned int)AbortHandler,
                                        (unsigned int)IRQHandler,
                                        (unsigned int)FIQHandler};

static volatile unsigned int page_table[4 * 1024]
    __attribute__((aligned(16 * 1024)));

/*----- Extern variable definitions ----------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

int main(void);

/*----- Static function prototypes -----------------------------------*/

static void _copy_vector_table(void);
static void _psc0_init(void);
static void _psc1_init(void);
static void _psc1_init(void);
static void _pll0_init(unsigned char clk_src, unsigned char pllm,
                       unsigned char prediv, unsigned char postdiv,
                       unsigned char div1, unsigned char div3,
                       unsigned char div7);
static void _pll1_init(unsigned char pllm, unsigned char postdiv,
                       unsigned char div1, unsigned char div2,
                       unsigned char div3);

static void _config_cache_mmu(void);
static void _ddr_memtest(void);
static void _boot_abort(void);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Boot strap function which enables the PLL(s) and PSC(s) for basic
 *          module(s)
 *
 * @param   none
 *
 * @return  None.
 *
 * This function is the first function that needs to be called in a system.
 * This should be set as the entry point in the linker script if loading the
 * elf binary via a debugger, on the target. This function never returns, but
 * gives control to the application entry point
 **/
void start_boot(void) {

    // Enable write-protection for registers of SYSCFG module.
    SysCfgRegistersLock();

    // Disable write-protection for registers of SYSCFG module.
    SysCfgRegistersUnlock();

    /// TODO: Configure Master Priority Control

    _psc0_init();
    _psc1_init();

    // Set the PLL0 to generate 300MHz for ARM.
    _pll0_init(PLL_CLK_SRC, PLL0_MUL, PLL0_PREDIV, PLL0_POSTDIV, PLL0_DIV1,
               PLL0_DIV3, PLL0_DIV7);

    _pll1_init(PLL1_MUL, PLL1_POSTDIV, PLL1_DIV1, PLL1_DIV2, PLL1_DIV3);

    ddr_init();

    _config_cache_mmu();

    // if (ddr_memtest() != 0) {
    //     _boot_abort();
    // }

    // Initialize the vector table with opcodes.
    _copy_vector_table();

    main();

    _boot_abort();
}

void delay(unsigned int count) {

    while (count--)
        ;
}

/*----- Static function implementations ------------------------------*/

/// TODO: Move peripheral power up to driver init function.
/*
 *  Configure PSC0:
 */
static void _psc0_init(void) {

    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC1, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_EMIFA, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_SPI0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_MMCSD0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_AINTC, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_ARM_RAMROM, 0,
                     PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_UART0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_SCR0_SS, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_SCR1_SS, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_SCR2_SS, 0, PSC_MDCTL_NEXT_ENABLE);

    /// TODO: Investigate why this causes boot to fail.
    //
    // Boot fails
    // PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_ARM, 0, PSC_MDCTL_NEXT_ENABLE);
}

/// TODO: Move peripheral power up to driver init function.
/*
 *  Configure PSC1:
 */
static void _psc1_init(void) {

    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_CC1, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_USB0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, 0, PSC_MDCTL_NEXT_ENABLE);

    // PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_DDR2_MDDR, 0,
    //                  PSC_MDCTL_NEXT_ENABLE);

    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_MCASP0, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SPI1, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART1, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART2, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_TC2, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SCRF0_SS, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SCRF1_SS, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SCRF6_SS, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SCRF7_SS, 0, PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SCRF8_SS, 0, PSC_MDCTL_NEXT_ENABLE);

    /// TODO: Investigate why this causes boot to fail.
    //
    // Boot fails
    // PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_SHRAM, 0, PSC_MDCTL_NEXT_ENABLE);
}

/*
 * @brief   This function Configures the PLL0 registers.
 *          PLL Register are set to achieve the desired frequencies.
 *
 * @param   clk_src
 * @param   pllm             This value is assigned to the PLLMultipler
 * register.
 * @param   prediv           This value is assigned to the PLLMultipler
 * register.
 * @param   postdiv          This value is assigned to the PLL_Postdiv register.
 * @param   div1             This value is assigned to the PLL_DIV1 register.
 * @param   div3             This value is assigned to the PLL_DIV3 register.
 * @param   div7             This value is assigned to the PLL_DIV7 register.
 *
 * @return  Int              Returns success or failure
 */
static void _pll0_init(unsigned char clk_src, unsigned char pllm,
                       unsigned char prediv, unsigned char postdiv,
                       unsigned char div1, unsigned char div3,
                       unsigned char div7) {

    // Clear lock bit.
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_CFGCHIP0) &=
        ~SYSCFG_CFGCHIP0_PLL_MASTER_LOCK;

    // PLLENSRC must be cleared before PLLEN bit have any effect
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLENSRC;

    // PLLCTL.EXTCLKSRC bit 9 should be left at 0
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_EXTCLKSRC;

    // PLLEN = 0 put pll in bypass mode
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLEN;

    // wait for 4 counts to switch pll to the bypass mode
    delay(4);

    // Select the Clock Mode bit 8 as On Chip Oscillator
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_CLKMODE;
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) |= (clk_src << 8);

    // Clear the PLLRST to reset the PLL
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLRST;

    // Disable PLL out
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) |= PLLC_PLLCTL_PLLDIS;

    // PLL initialization sequece, power up the PLL
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLPWRDN;

    // Enable PLL out.
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLDIS;

    // Wait for 2000 counts
    delay(2000);

    // Program the required multiplier value
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLM) = pllm;

    HWREG(SOC_PLLC_0_REGS + PLLC_PREDIV) = PLLC_PREDIV_PREDEN | prediv;
    HWREG(SOC_PLLC_0_REGS + PLLC_POSTDIV) = PLLC_POSTDIV_POSTDEN | postdiv;

    // Check for the GOSTAT bit in PLLSTAT to clear to 0
    // to indicate that no GO operation is currently in progress
    while (HWREG(SOC_PLLC_0_REGS + PLLC_PLLSTAT) & PLLC_PLLSTAT_GOSTAT)
        ;

    // divider values are assigned
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV1) = PLLC_PLLDIV1_D1EN | div1;

    HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV2) =
        PLLC_PLLDIV2_D2EN | (((div1 + 1) * 2) - 1);

    HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV4) =
        PLLC_PLLDIV4_D4EN | (((div1 + 1) * 4) - 1);

    HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV6) = PLLC_PLLDIV6_D6EN | div1;

    HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV3) = PLLC_PLLDIV3_D3EN | div3;

    HWREG(SOC_PLLC_0_REGS + PLLC_PLLDIV7) = PLLC_PLLDIV7_D7EN | div7;

    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCMD) |= PLLC_PLLCMD_GOSET;

    // Wait for the Gostat bit in PLLSTAT to clear to 0
    // (completion of phase alignment)
    while (HWREG(SOC_PLLC_0_REGS + PLLC_PLLSTAT) & PLLC_PLLSTAT_GOSTAT)
        ;

    // Wait for 200 counts
    delay(200);

    // set the PLLRST bit in PLLCTL to 1,bring the PLL out of reset
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) |= PLLC_PLLCTL_PLLRST;

    // Wait for 0x960 counts
    delay(0x960);

    // removing pll from bypass mode
    HWREG(SOC_PLLC_0_REGS + PLLC_PLLCTL) |= PLLC_PLLCTL_PLLEN;

    // set PLL lock bit
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_CFGCHIP0) |=
        (0x01 << SYSCFG_CFGCHIP0_PLL_MASTER_LOCK_SHIFT) &
        SYSCFG_CFGCHIP0_PLL_MASTER_LOCK;

    /// TODO: Is this the default value?
    ///         Could drive directly from PLL
    ///         to increase speed.
    //
    // Not set in factory firmware.
    // EMIFA driven by PLL0_SYSCLK3
    // HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_CFGCHIP3) &= CLK_PLL0_SYSCLK3;
}

/**
 * @brief This function Configures the PLL1 registers.
 * PLL Register are set to achieve the desired frequencies.
 *
 * @param   clk_src
 *          pllm             This value is assigned to the PLL1Multipler
 register.
 *          postdiv          This value is assigned to the PLL1_Postdiv
 register.
 *          div1             This value is assigned to the PLL1_Div1 register.
 *          div2             This value is assigned to the PLL1_Div2 register.
 *          div3             This value is assigned to the PLL1_Div3 register.
 *
 * @return  Int          Returns Success or Failure,depending on the execution
**/
static void _pll1_init(unsigned char pllm, unsigned char postdiv,
                       unsigned char div1, unsigned char div2,
                       unsigned char div3) {
    // Clear PLL lock bit
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_CFGCHIP3) &=
        ~SYSCFG_CFGCHIP3_PLL1_MASTER_LOCK;

    // PLLENSRC must be cleared before PLLEN has any effect
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLENSRC;

    // PLLCTL.EXTCLKSRC bit 9 should be left at 0
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_EXTCLKSRC;

    // Set PLLEN=0 to put in bypass mode
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLEN;

    // wait for 4 cycles to allow PLLEN mux
    // switches properly to bypass clock
    delay(4);

    // Clear PLLRST bit to reset the PLL
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLRST;

    // Disable the PLL output
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= PLLC_PLLCTL_PLLDIS;

    // PLL initialization sequence
    // Power up the PLL by setting PWRDN bit set to 0
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLPWRDN;

    // Enable the PLL output
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) &= ~PLLC_PLLCTL_PLLDIS;

    delay(2000);

    // Multiplier value is set
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLM) = pllm;

    HWREG(SOC_PLLC_1_REGS + PLLC_POSTDIV) = PLLC_POSTDIV_POSTDEN | postdiv;

    while (HWREG(SOC_PLLC_1_REGS + PLLC_PLLSTAT) & PLLC_PLLCMD_GOSET)
        ;
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLDIV1) = PLLC_PLLDIV1_D1EN | div1;
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLDIV2) = PLLC_PLLDIV2_D2EN | div2;
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLDIV3) = PLLC_PLLDIV3_D3EN | div3;

    // Set the GOSET bit in PLLCMD to 1 to initiate a new divider transition
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCMD) |= PLLC_PLLCMD_GOSET;

    while (HWREG(SOC_PLLC_1_REGS + PLLC_PLLSTAT) & PLLC_PLLSTAT_GOSTAT)
        ;

    // Wait for the Gostat bit in PLLSTAT to clear to 0
    // (completion of phase alignment).
    delay(200);

    // set the PLLRST bit in PLLCTL to 1,bring the PLL out of reset
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) |= PLLC_PLLCTL_PLLRST;
    delay(0x960);

    // Removing PLL from bypass mode
    HWREG(SOC_PLLC_1_REGS + PLLC_PLLCTL) |= PLLC_PLLCTL_PLLEN;

    // set PLL lock bit
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_CFGCHIP3) |=
        (0x1 << SYSCFG_CFGCHIP3_PLL1_MASTER_LOCK_SHIFT) &
        SYSCFG_CFGCHIP3_PLL1_MASTER_LOCK;
}

static void _copy_vector_table(void) {
    unsigned int *dest = (unsigned int *)0xFFFF0000;
    unsigned int *src = (unsigned int *)vecTbl;
    unsigned int count;

    for (count = 0; count < sizeof(vecTbl) / sizeof(vecTbl[0]); count++) {
        dest[count] = src[count];
    }
}

static void _config_cache_mmu() {
    int i;

    for (i = 0; i < (4 * 1024); i++) {

        if ((i >= 0xc00 && i < 0xc3f) || (i == 0x800)) {

            // 64 MB DDR and OC RAM cacheable and bufferable.
            page_table[i] = (i << 0x14) | 0x00000c1e;

            /// TODO: Investigate why.
            ///         Is there a performance improvement?
            //
            // Map second 64MB of DDR address space over actual DDR.
            // } else if (i >= 0xc40 && i < 0xc7f) {
            //     page_table[i] = (i - 0x40) * 0x100000 | 0xc12;

        } else {
            page_table[i] = (i << 0x14) | 0x00000c12;
        }

        // Map start of address space to RAM region.
        //  Allows setting interrupt vector to 0x0 then selecting OC RAM or DDR.
        //      Probably don't need this if we have vector table in ARM RAM.
        //      Will need this before booting into factory app.
        // page_table[0] = 0xc1e | 0xc0000000;
        // page_table[0] = 0xc1e | 0x80000000;
    }

    /// TODO: Move Domain Access setting to separate function.
    //
    CP15TtbSet((unsigned int)page_table);

    CP15ICacheFlush();

    CP15DCacheFlush();

    CP15MMUEnable();

    CP15ICacheEnable();

    CP15DCacheEnable();
}

static void _boot_abort(void) {
    while (1)
        ;
}

// void privileged_mode(void) { asm("    SWI   458752"); }
//
// void system_mode(void) {
//     asm("    mrs     r0, CPSR\n\t"
//         "    bic     r0, #0x0F\n\t"
//         "    orr     r0, #0x10\n\t "
//         "    msr     CPSR, r0");
// }
//

/// TODO: Read and write CPSR.
//
// unsigned int read_cpsr(void) {
//     asm volatile("eor     r3, %1, %1, ror #16\n\t"
//                  "bic     r3, r3, #0x00FF0000\n\t"
//                  "mov     %0, %1, ror #8\n\t"
//                  "eor     %0, %0, r3, lsr #8"
//                  : "=r"(val)
//                  : "0"(val)
//                  : "r3");
//     return val;
// }

/*----- End of file --------------------------------------------------*/
