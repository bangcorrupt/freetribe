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

#define SPI_DATA_INT IRQ_DMA7
#define SPI_ERROR_INT IRQ_SPI_ERR

/*----- Typedefs -----------------------------------------------------*/

typedef struct {

    uint8_t *tx_buffer;
    uint8_t *rx_buffer;

    uint32_t trx_length;

    void (*trx_callback)();
    void (*error_callback)();

} t_spi;

/*----- Static variable definitions ----------------------------------*/

static t_spi g_spi;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _spi_isr(void) __attribute__((interrupt_handler));

static inline void _spi_interrupt_enable(uint32_t int_flags);
static inline void _spi_interrupt_disable(uint32_t int_flags);

/*----- Extern function implementations ------------------------------*/

/// TODO: Generalise with config struct.

void per_spi_init(void) {

    *pPORTGIO_SET = HWAIT;

    // Don't attempt to drive the clock.
    *pSPI_BAUD = 0;

    // Reset the flags register to defaults.
    *pSPI_FLG = 0xff00;

    // 8 bit, MSB first, non-dma rx mode.
    // Interrupt when SPI_RDBR is full.
    *pSPI_CTL = SZ | EMISO;

    // Clear tx register.
    *pSPI_TDBR = 0;

    // SPI data interrupt IVG11.
    *pSIC_IAR2 |= P21_IVG(11);

    *pEVT11 = &_spi_isr;

    int i;
    // Unmask in the core event processor
    asm volatile("cli %0; bitset(%0, 11); sti %0; csync;" : "+d"(i));
    ssync();

    // Enable SPI.
    *pSPI_CTL |= SPE;
    ssync();

    // Clear the SPI rx register by reading from it.
    int j = *pSPI_RDBR;
    (void)j;

    // Clear the Rx error bit (sticky - W1C).
    *pSPI_STAT |= 0x10;

    *pPORTGIO_CLEAR = HWAIT;
}

void per_spi_trx_int(uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t length) {

    if (tx_buffer != NULL && tx_buffer != NULL && length != 0) {

        g_spi.rx_buffer = rx_buffer;
        g_spi.tx_buffer = tx_buffer;

        g_spi.trx_length = length;

        _spi_interrupt_enable(SPI_DATA_INT);
    }
}

void per_spi_register_callback(t_spi_event event, void (*callback)()) {

    switch (event) {

    case EVT_SPI_TRX_COMPLETE:
        g_spi.trx_callback = callback;
        break;

    case EVT_SPI_ERROR:
        g_spi.error_callback = callback;
        break;

        // case SPI_ERROR:

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

static inline void _spi_interrupt_enable(uint32_t int_flags) {

    *pSIC_IMASK0 |= int_flags;
    ssync();
}

static inline void _spi_interrupt_disable(uint32_t int_flags) {

    *pSIC_IMASK0 &= ~int_flags;
    ssync();
}

__attribute__((interrupt_handler)) static void _spi_isr(void) {

    *pPORTGIO_SET = HWAIT;

    *g_spi.rx_buffer++ = *pSPI_RDBR;

    *pSPI_TDBR = *g_spi.tx_buffer++;

    if (--g_spi.trx_length == 0) {

        _spi_interrupt_disable(SPI_DATA_INT);

        if (g_spi.trx_callback != NULL) {
            g_spi.trx_callback();
        }
    }

    ssync();

    *pPORTGIO_CLEAR = HWAIT;
}

/*----- End of file --------------------------------------------------*/
