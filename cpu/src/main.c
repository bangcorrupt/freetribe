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

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _set_backlight(bool red, bool green, bool blue);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief  Run kernel and app.
 *
 */
int main(void) {

    per_gpio_init();

    // LEDs will not light unless this pin is high.
    per_gpio_set_indexed(GPIO_BOARD_POWER, true); // Set GP7P13

    _set_backlight(BACKLIGHT_GREEN);

    while (true) {

        //
    }

    return 0;
}

/*----- Static function implementations ------------------------------*/

static void _set_backlight(bool red, bool green, bool blue) {

    per_gpio_set_indexed(GPIO_LCD_RED, red);     // Set GP6P3: LCD Red
    per_gpio_set_indexed(GPIO_LCD_GREEN, green); // Set GP6P4: LCD Green
    per_gpio_set_indexed(GPIO_LCD_BLUE, blue);   // Set GP6P5: LCD Blue
}

/*----- End of file --------------------------------------------------*/
