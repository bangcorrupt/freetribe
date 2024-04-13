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
 * @brief   Public API for SPI peripheral driver.
 *
 */

#ifndef PER_SPI_H
#define PER_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "soc_AM1808.h"

/*----- Macros and Definitions ---------------------------------------*/

#define SPI0_BASE SOC_SPI_0_REGS
#define SPI1_BASE SOC_SPI_1_REGS

#define SPI_0 0
#define SPI_1 1

typedef enum { SPI_TX_COMPLETE, SPI_RX_COMPLETE } t_spi_event;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void per_spi0_init(void);
void per_spi1_init(void);
bool per_spi_initialised(uint8_t instance);
void per_spi_chip_format(uint8_t instance, uint8_t data_format,
                         uint8_t chip_select, bool cshold);

void per_spi1_register_callback(t_spi_event event, void (*callback)());

void per_spi1_tx_int(uint8_t *buffer, uint32_t length);

void per_spi1_transceive_int(uint8_t *tx_buffer, uint8_t *rx_buffer,
                             uint32_t length);

void per_spi1_tx(uint8_t *buffer, uint32_t length);

void per_spi_tx(uint32_t, uint8_t *, uint32_t);
void per_spi_rx(uint32_t, uint8_t *, uint32_t);
void spi_tx_e2(uint32_t, uint8_t *, uint32_t);
void spi_rx_e2(uint32_t, uint8_t *, uint32_t);

void per_spi_chip_select(uint32_t spi_base, uint8_t cs, bool state);
void spi0_set_chip_select(uint8_t cs);
void per_spi0_tx(uint8_t *buffer, uint32_t length);
void per_spi0_tx_int(uint8_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif
#endif
/*----- End of file --------------------------------------------------*/
