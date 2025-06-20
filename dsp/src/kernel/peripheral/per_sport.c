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
 * @file    per_sport.c
 *
 * @brief   Peripheral driver for for BF523 SPORT.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include <blackfin.h>
#include <builtins.h>

#include "defBF52x_base.h"
#include "types.h"

#include "per_sport.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: Add this to defBF52x_base.h and rebuild toolchain.
//
#define DTYPE_SIGX 0x0004 /* SPORTx RCR1 Data Format Sign Extend */

#define DSP_BLOCK_SIZE 16

/*----- Typedefs -----------------------------------------------------*/

typedef struct {

} t_sport;

// Large descriptor list mode.
typedef struct {
    void *next_desc;
    void *start_addr;
    uint16_t config;
    uint16_t x_count;
    uint16_t x_mod;
    uint16_t y_count;
    uint16_t y_mod;

} t_dma_desc;

/*----- Static variable definitions ----------------------------------*/

// SPORT0 DMA transmit buffer
static fract32 g_codec_tx_buffer_a[DSP_BLOCK_SIZE];
static fract32 g_codec_tx_buffer_b[DSP_BLOCK_SIZE];

// SPORT0 DMA receive buffer
static fract32 g_codec_rx_buffer_a[DSP_BLOCK_SIZE];
static fract32 g_codec_rx_buffer_b[DSP_BLOCK_SIZE];

// 2 channels of input from ADC.
static fract32 *g_codec_in;

// 2 channels of output to DAC.
static fract32 *g_codec_out;

static bool g_sport0_frame_received = false;

static t_dma_desc g_sport0_rx_dma_a;
static t_dma_desc g_sport0_rx_dma_b;

static t_dma_desc g_sport0_tx_dma_a;
static t_dma_desc g_sport0_tx_dma_b;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _dma_desc_init(void);
static void _sport0_isr(void) __attribute__((interrupt_handler));

/*----- Extern function implementations ------------------------------*/

void sport0_init(void) {

    _dma_desc_init();

    // Configure SPORT0 Rx.
    // Clock Falling Edge, Receive Frame Sync, Data Format Sign Extend.
    *pSPORT0_RCR1 = RCKFE | RFSR | DTYPE_SIGX;
    // Rx Stereo Frame Sync Enable, Rx Word Length 32 bit.
    *pSPORT0_RCR2 = RSFSE | SLEN(0x1f); // RXSE;

    // Configure SPORT0 Tx.
    *pSPORT0_TCR1 = TCKFE | TFSR;
    *pSPORT0_TCR2 = TSFSE | SLEN(0x1f); // TXSE ;
    ssync();

    // SPORT0 Rx DMA.
    *pDMA3_PERIPHERAL_MAP = PMAP_SPORT0RX;

    // Linked descriptor mode.
    *pDMA3_NEXT_DESC_PTR = &g_sport0_rx_dma_a;
    *pDMA3_CONFIG = FLOW_LARGE | NDSIZE_7;
    ssync();

    // SPORT0 Tx DMA.
    *pDMA4_PERIPHERAL_MAP = PMAP_SPORT0TX;

    // Linked descriptor mode.
    *pDMA4_NEXT_DESC_PTR = &g_sport0_tx_dma_a;
    *pDMA4_CONFIG = FLOW_LARGE | NDSIZE_7;
    ssync();

    // SPORT0 Rx DMA3 interrupt IVG9.
    *pSIC_IAR2 |= P16_IVG(9);
    ssync();

    // Set SPORT0 Rx interrupt vector.
    *pEVT9 = _sport0_isr;
    ssync();

    // Enable SPORT0 Rx interrupt.
    *pSIC_IMASK0 |= IRQ_DMA3;
    ssync();

    int i;
    // unmask in the core event processor
    asm volatile("cli %0; bitset(%0, 9); sti %0; csync;" : "+d"(i));
    ssync();

    // Enable SPORT0 Rx DMA.
    *pDMA3_CONFIG |= DMAEN;
    ssync();

    // Enable SPORT0 Tx DMA.
    *pDMA4_CONFIG |= DMAEN;
    ssync();

    // Enable SPORT0 Rx.
    *pSPORT0_RCR1 |= RSPEN;
    ssync();

    // Enable SPORT0 Tx.
    *pSPORT0_TCR1 |= TSPEN;
    ssync();
}

void sport1_init(void) {

    /// /// TODO: Do we need secondary enabled?
    ///
    /// // Configure SPORT1 Rx.
    /// *pSPORT1_RCR1 = RCKFE | RFSR | DTYPE_SIGX;
    /// *pSPORT1_RCR2 = RSFSE | SLEN(0x1f); // RXSE ;
    ///
    /// // Configure SPORT1 Tx.
    /// *pSPORT1_TCR1 = TCKFE | TFSR;
    /// *pSPORT1_TCR2 = TSFSE | SLEN(0x1f); // TXSE  ;
    /// ssync();
    ///
    ///
    /// // SPORT1 Rx DMA.
    /// *pDMA5_PERIPHERAL_MAP = PMAP_SPORT1RX;
    /// // Configure DMA.
    /// *pDMA5_CONFIG = FLOW_AUTO | WDSIZE_32 | WNR;
    /// // Start address of data buffer.
    /// *pDMA5_START_ADDR = &g_cpu_rx_buffer;
    /// // DMA inner loop count.
    /// *pDMA5_X_COUNT = 2; // 2 samples.
    /// // Inner loop address increment.
    /// *pDMA5_X_MODIFY = 4; // 32 bit.
    /// ssync();
    ///
    /// // SPORT1 Tx DMA.
    /// *pDMA6_PERIPHERAL_MAP = PMAP_SPORT1TX;
    /// // Configure DMA.
    /// *pDMA6_CONFIG = FLOW_AUTO | WDSIZE_32;
    /// // Start address of data buffer
    /// *pDMA6_START_ADDR = &g_cpu_tx_buffer;
    /// // DMA inner loop count
    /// *pDMA6_X_COUNT = 2; // 2 samples.
    /// // Inner loop address increment
    /// *pDMA6_X_MODIFY = 4; // 32 bit.
    /// ssync();
    ///
    /// // Enable SPORT1 Rx DMA.
    /// *pDMA5_CONFIG |= DMAEN;
    /// ssync();
    ///
    /// // Enable SPORT1 Tx DMA.
    /// *pDMA6_CONFIG |= DMAEN;
    /// ssync();
    ///
    /// // Enable SPORT1 Rx.
    /// *pSPORT1_RCR1 |= RSPEN;
    /// ssync();
    ///
    /// // Enable SPORT1 Tx.
    /// *pSPORT1_TCR1 |= TSPEN;
    /// ssync();
}

fract32 *sport0_get_rx_buffer(void) { return g_codec_in; }

fract32 *sport0_get_tx_buffer(void) { return g_codec_out; }

bool sport0_frame_received(void) { return g_sport0_frame_received; }

void sport0_frame_processed(void) { g_sport0_frame_received = false; }

/*----- Static function implementations ------------------------------*/

/// TODO: Block processing.  For now we process each frame as it arrives.
__attribute__((interrupt_handler)) static void _sport0_isr(void) {

    // Clear interrupt status.
    *pDMA3_IRQ_STATUS = DMA_DONE;
    ssync();

    // Get input from codec.
    g_codec_in =
        ((fract32 *)&(((t_dma_desc *)(*pDMA3_NEXT_DESC_PTR))->start_addr)[0]);

    // Send output to codec.
    g_codec_out =
        ((fract32 *)&(((t_dma_desc *)(*pDMA4_NEXT_DESC_PTR))->start_addr)[0]);

    g_sport0_frame_received = true;
}

static void _dma_desc_init(void) {

    g_sport0_rx_dma_a.next_desc = &g_sport0_rx_dma_b;
    g_sport0_rx_dma_a.start_addr = g_codec_rx_buffer_a;
    g_sport0_rx_dma_a.config =
        FLOW_LARGE | DI_EN | WDSIZE_32 | WNR | NDSIZE_7 | DMAEN;
    g_sport0_rx_dma_a.x_count = DSP_BLOCK_SIZE;
    g_sport0_rx_dma_a.x_mod = 4;

    g_sport0_rx_dma_b.next_desc = &g_sport0_rx_dma_a;
    g_sport0_rx_dma_b.start_addr = g_codec_rx_buffer_b;
    g_sport0_rx_dma_b.config =
        FLOW_LARGE | DI_EN | WDSIZE_32 | WNR | NDSIZE_7 | DMAEN;
    g_sport0_rx_dma_b.x_count = DSP_BLOCK_SIZE;
    g_sport0_rx_dma_b.x_mod = 4;

    g_sport0_tx_dma_a.next_desc = &g_sport0_tx_dma_b;
    g_sport0_tx_dma_a.start_addr = g_codec_tx_buffer_a;
    g_sport0_tx_dma_a.config = FLOW_LARGE | WDSIZE_32 | NDSIZE_7 | DMAEN;
    g_sport0_tx_dma_a.x_count = DSP_BLOCK_SIZE;
    g_sport0_tx_dma_a.x_mod = 4;

    g_sport0_tx_dma_b.next_desc = &g_sport0_tx_dma_a;
    g_sport0_tx_dma_b.start_addr = g_codec_tx_buffer_b;
    g_sport0_tx_dma_b.config = FLOW_LARGE | WDSIZE_32 | NDSIZE_7 | DMAEN;
    g_sport0_tx_dma_b.x_count = DSP_BLOCK_SIZE;
    g_sport0_tx_dma_b.x_mod = 4;
}

/*----- End of file --------------------------------------------------*/
