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
 * @file    per_gpio.h
 *
 * @brief   Public API for GPIO peripheral driver.
 */

#ifndef PER_GPIO_H
#define PER_GPIO_H

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

void per_gpio_init(void);
bool per_gpio_get(uint8_t bank, uint8_t pin);
void per_gpio_set(uint8_t bank, uint8_t pin, bool state);
void per_gpio_toggle(uint8_t bank, uint8_t pin);

bool per_gpio_get_indexed(uint8_t pin_index);
void per_gpio_set_indexed(uint8_t pin_index, bool state);
void per_gpio_toggle_indexed(uint8_t pin_index);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
