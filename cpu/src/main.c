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
 * @file    main.c
 *
 * @brief   Main function for Freetribe CPU firmware.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>

#include "per_gpio.h"
#include "per_uart.h"

/*----- Macros -------------------------------------------------------*/

#define GPIO_LCD_RED 100
#define GPIO_LCD_GREEN 101
#define GPIO_LCD_BLUE 102

#define GPIO_BOARD_POWER 126

#define BACKLIGHT_RED true, false, false
#define BACKLIGHT_GREEN false, true, false
#define BACKLIGHT_BLUE false, false, true

#define BACKLIGHT_YELLOW true, true, false
#define BACKLIGHT_CYAN false, true, true
#define BACKLIGHT_MAGENTA true, false, true

#define TRS_UART UART_1

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _board_init(void);

static void _set_backlight(bool red, bool green, bool blue);

static void _serial_init(void);
static void _serial_echo(void);
static void _serial_parse(uint8_t *byte);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief  Echo serial bytes.
 *
 */
int main(void) {

    _board_init();

    _serial_init();

    _set_backlight(BACKLIGHT_GREEN);

    while (true) {

        _serial_echo();
    }

    return 0;
}

/*----- Static function implementations ------------------------------*/

static void _board_init(void) {

    per_gpio_init();

    // LEDs will not light unless this pin is high.
    per_gpio_set_indexed(GPIO_BOARD_POWER, true); // Set GP7P13
}

static void _set_backlight(bool red, bool green, bool blue) {

    per_gpio_set_indexed(GPIO_LCD_RED, red);     // Set GP6P3: LCD Red
    per_gpio_set_indexed(GPIO_LCD_GREEN, green); // Set GP6P4: LCD Green
    per_gpio_set_indexed(GPIO_LCD_BLUE, blue);   // Set GP6P5: LCD Blue
}

static void _serial_init(void) {

    t_uart_config uart_cfg = {.instance = TRS_UART,
                              .baud = 31250,
                              .word_length = 8,
                              .oversample = OVERSAMPLE_16};

    // Initialise UART for MIDI, polling mode.
    per_uart_init(&uart_cfg);
}

static void _serial_echo(void) {

    uint8_t rx_byte;

    // Block until byte received.
    per_uart_receive(TRS_UART, &rx_byte, 1);

    _serial_parse(&rx_byte);

    // Echo received byte.
    per_uart_transmit(TRS_UART, &rx_byte, 1);
}

static void _serial_parse(uint8_t *byte) {
    //
    switch (*byte) {

    case 0xf8:
        _set_backlight(BACKLIGHT_BLUE);
        break;

    case 0xf9:
        // Ignore.
        break;

    case 0xfa:
        _set_backlight(BACKLIGHT_CYAN);
        break;

    case 0xfb:
        _set_backlight(BACKLIGHT_YELLOW);
        break;

    case 0xfc:
        _set_backlight(BACKLIGHT_RED);
        break;

    case 0xfd:
        // Ignore.
        break;

    case 0xfe:
        _set_backlight(BACKLIGHT_MAGENTA);
        break;

    case 0xff:
        _set_backlight(BACKLIGHT_GREEN);
        break;

    default:
        // Ignore.
        break;
    }
}

/*----- End of file --------------------------------------------------*/
