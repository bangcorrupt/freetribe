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
 * @file    dev_lcd.h
 *
 * @brief   Public API for LCD device driver.
 */

#ifndef DEV_LCD_H
#define DEV_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void dev_lcd_init(void);

// True sets reset pin low.
void dev_lcd_reset(bool state);

void dev_lcd_set_frame(uint8_t *frame_buffer);
void dev_lcd_set_page(uint8_t page_index, uint8_t *page_buffer);
void dev_lcd_set_contrast(uint8_t contrast);
void dev_lcd_set_backlight(bool red, bool green, bool blue);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
