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
 * @file    per_gpio.c
 *
 * @brief   Configuration and handling for GPIO peripheral.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "hw_gpio.h"
#include "hw_types.h"

#include "soc_AM1808.h"

#include "csl_gpio.h"

#include "per_gpio.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/*
 * @brief   Initialise  and configure GPIO.
 *
 * @param   None.
 *
 * @return  None.
 *
 */
void per_gpio_init(void) {

    /// TODO: Use bitmask macros instead of magic numbers.

    // Set pin direction and state.

    // SPI1 CS0 CS1 outputs, set.
    //      Is this necessary if using pins in SPI mode?
    // HWREG(SOC_GPIO_0_REGS + GPIO_OUT_DATA(1)) = 0xc000;
    GPIOBankOutputDataSet(SOC_GPIO_0_REGS, 1, 0xc000);

    // HWREG(SOC_GPIO_0_REGS + GPIO_DIR(1)) = 0xffff3fff;
    GPIOBankDirModeSet(SOC_GPIO_0_REGS, 1, 0xffff3fff);

    // B5 P14 output, clear.
    // HWREG(SOC_GPIO_0_REGS + GPIO_OUT_DATA(2)) = 0x0;
    GPIOBankOutputDataSet(SOC_GPIO_0_REGS, 2, 0x0);

    // HWREG(SOC_GPIO_0_REGS + GPIO_DIR(2)) = 0xbfffffff;
    GPIOBankDirModeSet(SOC_GPIO_0_REGS, 2, 0xbfffffff);

    // B6 0, 1, 7, 13, 14, 15 out.
    // B6 2, 11 set
    // B7 14, 13, 12, 11, 10 out
    // B7 14 Set
    // HWREG(SOC_GPIO_0_REGS + GPIO_OUT_DATA(3)) = 0x40000804;
    GPIOBankOutputDataSet(SOC_GPIO_0_REGS, 3, 0x40000804);

    // HWREG(SOC_GPIO_0_REGS + GPIO_DIR(3)) = 0x83ffe083;
    GPIOBankDirModeSet(SOC_GPIO_0_REGS, 3, 0x83ffe083);

    // B8 P2 LCD Cmd/Data.
    // HWREG(SOC_GPIO_0_REGS + GPIO_OUT_DATA(4)) = 0x0;
    GPIOBankOutputDataSet(SOC_GPIO_0_REGS, 4, 0x0);

    // HWREG(SOC_GPIO_0_REGS + GPIO_DIR(4)) = 0x0000fffd;
    GPIOBankDirModeSet(SOC_GPIO_0_REGS, 4, 0x0000fffd);
}

/// TODO: Refactor to always use indexed access.
//          Allows #define PIN_NAME and avoids multiple conversions.

bool per_gpio_get(uint8_t bank, uint8_t pin) {

    // Pins indexed from 1 to 0x90.
    uint8_t pin_index = (bank << 4) + pin + 1;

    return per_gpio_get_indexed(pin_index);
}

void per_gpio_set(uint8_t bank, uint8_t pin, bool state) {

    // Pins indexed from 1 to 0x90.
    uint8_t pin_index = (bank << 4) + pin + 1;

    per_gpio_set_indexed(pin_index, state);
}

void per_gpio_toggle(uint8_t bank, uint8_t pin) {

    // Pins indexed from 1 to 0x90.
    uint8_t pin_index = (bank << 4) + pin + 1;

    per_gpio_toggle_indexed(pin_index);
}

bool per_gpio_get_indexed(uint8_t pin_index) {

    return GPIOPinRead(SOC_GPIO_0_REGS, pin_index);
}

void per_gpio_set_indexed(uint8_t pin_index, bool state) {

    GPIOPinWrite(SOC_GPIO_0_REGS, pin_index, state);
}

void per_gpio_toggle_indexed(uint8_t pin_index) {

    bool state;

    state = GPIOPinRead(SOC_GPIO_0_REGS, pin_index);
    GPIOPinWrite(SOC_GPIO_0_REGS, pin_index, !state);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
