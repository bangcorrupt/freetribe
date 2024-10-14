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
 * @file    per_spi.c
 *
 * @brief   Peripheral driver for BF523 SPI.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <blackfin.h>
#include <builtins.h>

#include "per_gpio.h"
#include "per_spi.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef struct {

    uint8_t *tx_byte;
    uint8_t *rx_byte;

    void (*tx_callback)(uint8_t *tx_byte);
    void (*rx_callback)(uint8_t *rx_byte);

} t_spi;

/*----- Static variable definitions ----------------------------------*/

static t_spi g_spi;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _spi_rx_isr(void) __attribute__((interrupt_handler));

/*----- Extern function implementations ------------------------------*/

void per_spi_init(void) {

    *pPORTGIO_SET = HWAIT;

    // SPI Rx interrupt IVG11.
    *pSIC_IAR2 |= P21_IVG(11);
    ssync();

    *pEVT11 = &_spi_rx_isr;

    // Enable SPI Rx interrupt.
    *pSIC_IMASK0 |= IRQ_DMA7; // Not actually using DMA.
    ssync();

    int i;
    // unmask in the core event processor
    asm volatile("cli %0; bitset(%0, 11); sti %0; csync;" : "+d"(i));
    ssync();

    // don't attempt to drive the clock
    *pSPI_BAUD = 0;

    // reset the flags register? to defaults?
    *pSPI_FLG = 0xff00;

    // 8 bit, MSB first, non-dma rx mode.
    // Interrupt when SPI_RDBR is full.
    *pSPI_CTL = SZ | EMISO;
    ssync();

    // enable spi.
    *pSPI_CTL |= SPE;
    ssync();

    // clear the spi rx register by reading from it
    int j = *pSPI_RDBR;
    (void)j;

    // clear the rx error bit (sticky - W1C)
    *pSPI_STAT |= 0x10;

    *pPORTGIO_CLEAR = HWAIT;
}

void per_spi_register_callback(t_spi_event event, void (*callback)()) {

    switch (event) {

    case SPI_TX_COMPLETE:
        g_spi.tx_callback = callback;
        break;

    case SPI_RX_COMPLETE:
        g_spi.rx_callback = callback;
        break;

        // case SPI_ERROR:

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

__attribute__((interrupt_handler)) static void _spi_rx_isr(void) {

    *pPORTGIO_SET = HWAIT;

    *g_spi.rx_byte = *pSPI_RDBR;

    if (g_spi.rx_callback != NULL) {
        g_spi.rx_callback(g_spi.rx_byte);
    }

    // Transmit 0 if no data set by callback.
    *g_spi.tx_byte = 0;

    if (g_spi.tx_callback != NULL) {
        g_spi.tx_callback(g_spi.tx_byte);
    }

    *pSPI_TDBR = *g_spi.tx_byte;

    ssync();

    *pPORTGIO_CLEAR = HWAIT;
}

/*----- End of file --------------------------------------------------*/
