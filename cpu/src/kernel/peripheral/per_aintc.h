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
 * @file    per_aintc.h
 *
 * @brief   Public API for ARM Interrupt Controller peripheral driver.
 */

#ifndef PER_AINTC_H
#define PER_AINTC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void per_aintc_init(void);

/**
 * @brief   Register interrupt triggered by GPIO pin.
 *
 * @param   channel    Channel number for a system interrupt. 0-1 are mapped to FIQ
 *                     and 2-31 are mapped to IRQ of ARM. One or more system interrupts
 *                     can be mapped to one channel. However, one system interrupt
 *                     can not be mapped to multiple channels.
 * 
 * @param   pin        Indexed GPIO pin number, starting from 1 through 144.
 * 
 * @param   int_type   How interrupt generation triggers based on electrical signal:
 *                     - GPIO_INT_TYPE_NOEDGE
 *                     - GPIO_INT_TYPE_FALLEDGE
 *                     - GPIO_INT_TYPE_RISEDGE
 *                     - GPIO_INT_TYPE_BOTHEDGE
 * 
 * @param   isr        Callback function.
 */
void per_aintc_register_gpio_interrupt(uint8_t channel, uint8_t pin, uint8_t int_type, void (*isr)(void));

/**
 * @brief   Clear interrupt status for GPIO interrupts.
 * 
 * @param   pin    Indexed GPIO pin number, starting from 1 through 144.
 */
void per_aintc_clear_status_gpio(uint8_t pin);

/**
 * @brief   Change trigger type of GPIO pin interrupt.
 * 
 * @param   pin        Indexed GPIO pin number, starting from 1 through 144.
 * 
 * @param   int_type   How interrupt generation triggers based on electrical signal:
 *                     - GPIO_INT_TYPE_NOEDGE
 *                     - GPIO_INT_TYPE_FALLEDGE
 *                     - GPIO_INT_TYPE_RISEDGE
 *                     - GPIO_INT_TYPE_BOTHEDGE
 * 
 */
void per_aintc_change_gpio_int_type(uint8_t pin, uint8_t int_type);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
