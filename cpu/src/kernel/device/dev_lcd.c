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
 * @file    dev_lcd.c
 *
 * @brief   LCD device driver.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "per_gpio.h"
#include "per_spi.h"

#include "dev_lcd.h"

#include "svc_delay.h"

/*----- Macros and Definitions ---------------------------------------*/

#define LCD_SPI 0

#define LCD_SPI_INT_CHANNEL 8
#define LCD_SPI_INT_LEVEL SPI_INT_LEVEL_TX
#define LCD_SPI_PIN_FUNC SPI_PIN_SIMO | SPI_PIN_CLK | SPI_PIN_CS0

#define LCD_SPI_DATA_FORMAT SPI_DATA_FORMAT0
#define LCD_SPI_FREQ SPI_FREQ_30_MHZ
#define LCD_SPI_CHAR_LENGTH 8

#define LCD_SPI_CHIP_SELECT 1
#define LCD_SPI_CSHOLD true

#define LCD_COLUMNS 0x80
#define LCD_PAGES 0x8
#define LCD_PAGE_LINES 0x8

typedef enum { COMMAND, DATA } t_lcd_mode;

// LCD_PIN_RESET, LCD_PIN_A0

/// TODO: HAL for LCD module?

/// TODO: typedef enum {} t_lcd_command;

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _lcd_command(uint8_t command);
static void _lcd_mode(t_lcd_mode mode);
static void _lcd_tx(uint8_t *p_tx, uint32_t len);
static void _lcd_tx(uint8_t *buffer, uint32_t length);

/*----- Extern function implementations ------------------------------*/

void dev_lcd_init(void) {

    t_spi_config config = {
        .instance = LCD_SPI,
        .int_channel = LCD_SPI_INT_CHANNEL,
        .int_level = LCD_SPI_INT_LEVEL,
        .pin_func = LCD_SPI_PIN_FUNC,
        .int_enable = true,
    };

    per_spi_init(&config);

    t_spi_format format = {
        .instance = LCD_SPI,
        .index = LCD_SPI_DATA_FORMAT,
        .freq = LCD_SPI_FREQ,
        .char_length = LCD_SPI_CHAR_LENGTH,
    };

    per_spi_set_data_format(&format);

    _lcd_mode(COMMAND);

    // _lcd_command(0xa2); // 0xa2: LCD bias set
    // _lcd_command(0xa0); // 0xa0: ADC select normal
    // _lcd_command(0xc8); // 0xc8: Common output mode reverse
    // _lcd_command(0x40); // 0x40: Line address 0
    // _lcd_command(0x2f); // 0x2f: Power control: Boost=On, Reg=On, Follow=On
    // _lcd_command(0xf8); // 0xf8: Booster ratio select mode set
    // _lcd_command(0x00); // 0x00: Booster ratio register = 0
    // _lcd_command(0x25); // 0x25: Voltage Regulator Resistor Ratio Set
    //
    // // Contrast setting.
    // _lcd_command(0x81); // 0x81: Electronic volume mode set
    // _lcd_command(0x22); // 0x22: Electronic volume register = 0x22
    //
    // _lcd_command(0xa4); // 0xa4: Display all points = Off
    // _lcd_command(0xa6); // 0xa6: Display Normal/Reverse = Normal
    // _lcd_command(0xaf); // 0xaf: Display on

    uint8_t lcd_init_cmd[] = {0xa2, 0xa0, 0xc8, 0x40, 0x2f, 0xf8, 0x00,
                              0x25, 0x81, 0x22, 0xa4, 0xa6, 0xaf};

    _lcd_tx(lcd_init_cmd, sizeof((lcd_init_cmd)));
}

// True sets reset pin low.
void dev_lcd_reset(bool state) { per_gpio_set(6, 9, !state); }

void dev_lcd_set_frame(uint8_t *frame_buffer) {

    uint8_t page_index;
    uint8_t *page_buffer;

    for (page_index = 0; page_index < LCD_PAGES; page_index++) {

        page_buffer = frame_buffer + page_index * LCD_COLUMNS;

        /// TODO: Implement ping-pong frame buffer.
        ///       strcmp page buffer and only update changes.

        dev_lcd_set_page(page_index, page_buffer);
    }
}

void dev_lcd_set_page(uint8_t page_index, uint8_t *page_buffer) {

    _lcd_mode(COMMAND);

    // _lcd_command(0xb0 | page_index); // Set display RAM page address.
    // _lcd_command(0x40);              // Start line.
    // _lcd_command(0x10);              // Column address upper.
    // _lcd_command(0x00);              // Column address lower.

    uint8_t lcd_page_cmd[] = {0xb0 | page_index, 0x40, 0x10, 0x00};

    _lcd_tx(lcd_page_cmd, sizeof(lcd_page_cmd));

    _lcd_mode(DATA);

    _lcd_tx(page_buffer, LCD_COLUMNS);
}

void dev_lcd_set_contrast(uint8_t contrast) {

    contrast = contrast > 0x3f ? 0x3f : contrast;
    contrast = contrast < 0x01 ? 0x01 : contrast;

    _lcd_mode(COMMAND);
    _lcd_command(0x81);
    _lcd_command(contrast);
}

/*
 * @brief   Set LCD backlight.
 *
 * @param   none
 *
 * @return  none
 */
void dev_lcd_set_backlight(bool red, bool green, bool blue) {

    /// TODO: Define pin index.
    //
    // LCD Backlight on
    per_gpio_set_indexed(100, red);   // Set GP6P3: LCD Red
    per_gpio_set_indexed(101, green); // Set GP6P4: LCD Green
    per_gpio_set_indexed(102, blue);  // Set GP6P5: LCD Blue

    // Configure LCD
}

/*----- Static function implementations ------------------------------*/

static void _lcd_tx(uint8_t *buffer, uint32_t length) {

    /// TODO: DMA frame buffer.

    per_spi_chip_format(LCD_SPI, LCD_SPI_DATA_FORMAT, LCD_SPI_CHIP_SELECT,
                        LCD_SPI_CSHOLD);

    per_spi_tx_int(LCD_SPI, buffer, length);
}

static void _lcd_mode(t_lcd_mode mode) { per_gpio_set(8, 1, mode); }

static void _lcd_command(uint8_t command) { _lcd_tx(&command, 1); }

/*----- End of file --------------------------------------------------*/
