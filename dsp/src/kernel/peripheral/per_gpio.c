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
 * @file    per_gpio.c
 *
 * @brief   Peripheral driver for BF523 GPIO.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>

#include <blackfin.h>
#include <builtins.h>

#include "per_gpio.h"

/*----- Macros and Definitions ---------------------------------------*/

#define PFMUX_SPORT 0x0555
#define PGMUX_SPISSEL 0x0002
#define PGMUX_UART 0x0020
// #define PGMUX_SPORT_SCLK 0x0140
#define PGMUX_HOSTDP 0x2800
#define PHMUX_HOSTDP 0x002a

#define PFFER_SPORT 0xd7ff // TSFS1 and TSCLK1 not enabled.
/* #define PFFER_SPORT 0xffff // TSFS1 and TSCLK1 enabled. */
#define PGFER_SPI 0x001e
#define PGFER_UART 0x0180
#define PGFER_HOSTDP 0xf800
#define PHFER_HOSTDP 0xffff

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void per_gpio_init(void) {

    *pPORTF_MUX = PFMUX_SPORT;
    *pPORTG_MUX = PGMUX_UART | PGMUX_HOSTDP;
    *pPORTH_MUX = PHMUX_HOSTDP;
    ssync();

    // GPIO mode.
    *pPORTF_FER = PFFER_SPORT;
    *pPORTG_FER = PGFER_SPI | PGFER_UART | PGFER_HOSTDP;
    *pPORTH_FER = PHFER_HOSTDP;
    ssync();

    // Set p0, p7.
    *pPORTGIO_SET = HWAIT | UART0_TX;

    // p0, p7, p9, p10 are outputs.
    *pPORTGIO_DIR = HWAIT | UART0_TX | PG9 | PG10;

    *pPORTGIO_SET = HWAIT | UART0_TX;
    ssync();
}

bool per_gpio_get(uint8_t port, uint8_t pin) {

    //
    return 0;
}

void per_gpio_set(uint8_t port, uint8_t pin, bool state) {
    //
}

void per_gpio_toggle(uint8_t port, uint8_t pin) {

    //
}

uint16_t per_gpio_get_port(uint8_t port) {

    switch (port) {

    case PORT_F:
        return *pPORTFIO;

    case PORT_G:
        return *pPORTGIO;

    case PORT_H:
        return *pPORTHIO;

    default:
        return 0;
    }
}

void per_gpio_set_port(uint8_t port, uint16_t value) {

    switch (port) {

    case PORT_F:
        *pPORTFIO = value;
        break;

    case PORT_G:
        *pPORTGIO = value;
        break;

    default:
        break;
    }

    //
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
