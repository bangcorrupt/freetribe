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

#include <blackfin.h>
#include <builtins.h>

#include "per_gpio.h"
#include "per_spi.h"

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: Central header for queue sizes.
//
#define SPI_RX_BUF_LEN 0x200
#define SPI_TX_BUF_LEN 0x200

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

// SPI Rx ring buffer.
static rbd_t spi_rx_rbd;
static uint8_t spi_rx_rbmem[SPI_RX_BUF_LEN];

// SPI Tx ring buffer.
static rbd_t spi_tx_rbd;
static uint8_t spi_tx_rbmem[SPI_TX_BUF_LEN];

static bool g_spi_tx_complete = true;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _spi_rx_isr(void) __attribute__((interrupt_handler));

static int _spi_tx_dequeue(uint8_t *spi_byte);
static void _spi_rx_enqueue(uint8_t *spi_byte);

/*----- Extern function implementations ------------------------------*/

void spi_init(void) {

    *pPORTGIO_SET = HWAIT;

    // Tx ring buffer attributes.
    rb_attr_t tx_attr = {sizeof(spi_tx_rbmem[0]), ARRAY_SIZE(spi_tx_rbmem),
                         spi_tx_rbmem};

    // Rx ring buffer attributes.
    rb_attr_t rx_attr = {sizeof(spi_rx_rbmem[0]), ARRAY_SIZE(spi_rx_rbmem),
                         spi_rx_rbmem};

    // Initialise MCU message ring buffers.
    if (ring_buffer_init(&spi_tx_rbd, &tx_attr) ||
        ring_buffer_init(&spi_rx_rbd, &rx_attr) == 0) {
    }

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
    // *pSPI_CTL = CPOL | CPHA | SZ | EMISO;
    // *pSPI_CTL = CPOL | SZ | EMISO;
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

int per_spi_rx_dequeue(uint8_t *spi_byte) {

    return ring_buffer_get(spi_rx_rbd, spi_byte);
}

/// TODO: Check/return status.
void per_spi_tx_enqueue(uint8_t *spi_byte) {

    // Overwrite on overflow?
    ring_buffer_put_force(spi_tx_rbd, spi_byte);
}

/*----- Static function implementations ------------------------------*/

__attribute__((interrupt_handler)) static void _spi_rx_isr(void) {

    *pPORTGIO_SET = HWAIT;

    /// TODO: Queue register contents directly.
    //
    uint8_t rx_byte = 0;
    uint8_t tx_byte = 0;

    rx_byte = *pSPI_RDBR;

    // Always queues received data.
    _spi_rx_enqueue(&rx_byte);

    // Transmits 0 if queue empty.
    _spi_tx_dequeue(&tx_byte);

    *pSPI_TDBR = tx_byte;

    ssync();

    *pPORTGIO_CLEAR = HWAIT;
}

/// TODO: Check/return status.
static int _spi_tx_dequeue(uint8_t *spi_byte) {

    return ring_buffer_get(spi_tx_rbd, spi_byte);
}

/// TODO: Check/return status.
static void _spi_rx_enqueue(uint8_t *spi_byte) {

    // Overwrite on overflow?
    ring_buffer_put_force(spi_rx_rbd, spi_byte);
}

/*----- End of file --------------------------------------------------*/
