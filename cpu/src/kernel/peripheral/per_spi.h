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

#include "hw_spi.h"
#include "soc_AM1808.h"

#include "hal_spi.h"

/*----- Macros and Definitions ---------------------------------------*/

#define SPI_INSTANCES SOC_SPI_PER_CNT

#define SPI_0 SOC_SPI_0
#define SPI_1 SOC_SPI_1

#define SPI0_BASE SOC_SPI_0_REGS
#define SPI1_BASE SOC_SPI_1_REGS

#define SPI_PIN_SOMI SPI_SPIPC0_SOMIFUN
#define SPI_PIN_SIMO SPI_SPIPC0_SIMOFUN
#define SPI_PIN_ENA SPI_SPIPC0_ENAFUN
#define SPI_PIN_CLK SPI_SPIPC0_CLKFUN

#define SPI_PIN_CS0 SPI_SPIPC0_SCS0FUN0
#define SPI_PIN_CS1 SPI_SPIPC0_SCS0FUN1
#define SPI_PIN_CS2 SPI_SPIPC0_SCS0FUN2

#define SPI_INT_LEVEL_TX SPI_TRANSMIT_INTLVL
#define SPI_INT_LEVEL_TX_RX SPI_TRANSMIT_INTLVL | SPI_RECV_INTLVL

#define SPI_INT_LEVEL_TX_RX_TIMEOUT_DESYNC                                     \
    SPI_TRANSMIT_INTLVL | SPI_RECV_INTLVL | SPI_TIMEOUT_INTLVL |               \
        SPI_DESYNC_SLAVE_INTLVL

#define SPI_MODULE_FREQ SOC_SPI_0_MODULE_FREQ

#define SPI_FREQ_50_MHZ 50000000
#define SPI_FREQ_37_5_MHZ 37500000
#define SPI_FREQ_30_MHZ 30000000

#define SPI_PIN_CTL_FUNC 0

#define SPI_DEFAULT_DATA_FORMAT SPI_DATA_FORMAT0
#define SPI_DEFAULT_FREQ SPI_FREQ_30_MHZ
#define SPI_DEFAULT_PHASE SPI_CLK_OUTOFPHASE
#define SPI_DEFAULT_CHAR_LENGTH 8

typedef enum { SPI_TX_COMPLETE, SPI_RX_COMPLETE } t_spi_event;

typedef struct {
    uint8_t instance;
    uint8_t int_channel;
    uint32_t int_level;
    bool int_enable;
    uint32_t pin_func;
} t_spi_config;

typedef struct {
    uint8_t instance;
    uint8_t index;
    uint32_t freq;
    uint8_t char_length;
    uint8_t ena_timeout;
} t_spi_format;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void per_spi_init(t_spi_config *config);
void per_spi_set_data_format(t_spi_format *format);

void per_spi_tx_int(uint8_t instance, uint8_t *buffer, uint32_t length);
void per_spi_trx_int(uint8_t instance, uint8_t *tx_buffer, uint8_t *rx_buffer,
                     uint32_t length);

bool per_spi_initialised(uint8_t instance);
void per_spi_chip_format(uint8_t instance, uint8_t data_format,
                         uint8_t chip_select, bool cshold);

void per_spi_register_callback(uint8_t instance, t_spi_event event,
                               void (*callback)());

void per_spi_tx(uint32_t, uint8_t *, uint32_t);
void per_spi_rx(uint32_t, uint8_t *, uint32_t);

void per_spi1_tx_wait(uint8_t *buffer, uint32_t length);
#ifdef __cplusplus
}
#endif
#endif
/*----- End of file --------------------------------------------------*/
