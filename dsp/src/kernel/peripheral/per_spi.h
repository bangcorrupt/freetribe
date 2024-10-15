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
 * @file    per_spi.h
 *
 * @brief   Public API for BF523 SPI peripheral driver.
 */

#ifndef PER_SPI_H
#define PER_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    SPI_TX_COMPLETE,
    SPI_RX_COMPLETE,
} t_spi_event;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void per_spi_init(void);
void per_spi_register_callback(t_spi_event event, void (*callback)());
void per_spi_trx_int(uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t length);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
