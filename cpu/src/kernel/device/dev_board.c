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
 * @file    dev_board.c
 *
 * @brief   Initialisation and termination of main board.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "per_aintc.h"
#include "per_ddr.h"
#include "per_gpio.h"
#include "per_pinmux.h"
#include "per_spi.h"
#include "per_uart.h"

#include "dev_board.h"

// TODO: Move up to svc_system.
#include "dev_flash.h"
#include "dev_lcd.h"

// TODO: Move blocking delay to device layer.
#include "svc_delay.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _hardware_init(void);

static void _hardware_terminate(void);

/*----- Extern function implementations ------------------------------*/

/*
 * @brief   Initialise and configure main board.
 *
 * @param   None
 *
 * @return  None
 */
void dev_board_init(void) {

    per_pinmux_init();

    per_aintc_init();

    // TODO: Move blocking delay to device layer.
    delay_init();

    per_gpio_init();

    _hardware_init();

    // TODO: Move up to svc_system.
    dev_lcd_set_backlight(true, true, true);

    // TODO: Move up to svc_system.
    dev_flash_init();
}

/*----- Static function implementations ------------------------------*/

/*
 * @brief   Shut down periperals.
 *
 *          Returns main board to unitialised state,
 *          allowing factory SBL and APP to boot.
 *
 * @param  none
 *
 * @return none
 *
 */
void dev_board_terminate(void) {
    // Allows handing off to factory bootloader.
    // Factory SBL will hang if DDR already initialised.
    ddr_terminate();

    // Flush TRS MIDI before terminating MCU.
    per_uart_terminate(1);
    per_uart_terminate(0);

    _hardware_terminate();
}

/*
 * @brief   Set GPIO to initialise board hardware?
 *
 * @param   none
 *
 * @return  none
 */
static void _hardware_init(void) {

    // TODO: What does each pin represent?
    //       Refactor to use GPIO peripheral driver.

    // MCU out of reset.
    per_gpio_set_indexed(105, 1); // Set GP6P8

    per_gpio_set_indexed(103, 1); // Set GP6P6

    // Only red lights if not set.
    per_gpio_set_indexed(126, 1); // Set GP7P13

    // 60us in factory firmware.
    delay_block_us(60);

    // ADC Reset.
    per_gpio_set_indexed(99, 0); // Clear GP6P2

    // ADC MCLK on ??
    per_gpio_set_indexed(123, 1); // Set GP7P10

    // Wait until B8P15 is high.
    // TODO: Timeout error.
    while (!per_gpio_get_indexed(144))
        ;

    // 10us in factory firmware.
    delay_block_us(10);

    per_gpio_set_indexed(124, 1); // Set GP7P11

    // This is set during boot, then toggles continuously while app running.
    per_gpio_set(6, 11, 1); // Set GP6P11

    // TODO: What are these for?
    // per_gpio_set(6, 9, 1);  // Set GP6P9
    // per_gpio_set(6, 12, 1); // Set GP6P12
}

static void _hardware_terminate(void) {

    per_gpio_set_indexed(105, 0); // Set GP6P8
    per_gpio_set_indexed(103, 0); // Set GP6P6
    per_gpio_set_indexed(126, 0); // Set GP7P13

    // Should be 60 us
    delay_block_us(60);

    per_gpio_set_indexed(99, 0);  // Set GP6P2
    per_gpio_set_indexed(123, 0); // Set GP7P10

    // Should be 10 us
    delay_block_us(10);
}

/*----- End of file --------------------------------------------------*/
