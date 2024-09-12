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
 * @file    startup.h
 *
 * @brief   Header for startup.c.
 *
 */

#ifndef STARTUP_H
#define STARTUP_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

/*----- Macros and Definitions ---------------------------------------*/

/// TODO: Move to sys_pll.c
//
#define CLK_PLL0_SYSCLK3 (0xFFFFFFF8)

// PLL0 register values
#define PLL_CLK_SRC 0
#define PLL0_MUL 0x18
#define PLL0_PREDIV 0
#define PLL0_POSTDIV 1
#define PLL0_DIV1 0
#define PLL0_DIV3 2
#define PLL0_DIV7 5

// Fixing PLL1 register values
#define PLL1_MUL 0x18
#define PLL1_POSTDIV 1
#define PLL1_DIV1 0
#define PLL1_DIV2 1
#define PLL1_DIV3 2

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

// Exception vectors, see exception_handler.s
extern void start(void);
extern void UndefInstHandler(void);
extern void SWIHandler(void);
extern void AbortHandler(void);
extern void IRQHandler(void);
extern void FIQHandler(void);

void delay(unsigned int count);
#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
