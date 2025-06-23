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

#include "knl_profile.h"

#include "per_sport.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: Add this to defBF52x_base.h and rebuild toolchain.
//
#define DTYPE_SIGX 0x0004 /* SPORTx RCR1 Data Format Sign Extend */

#define SPORT_DMA_X_MOD (DSP_BLOCK_SIZE * DSP_WORD_SIZE)
#define SPORT_DMA_X_COUNT (DSP_CHANNEL_COUNT)
#define SPORT_DMA_Y_MOD (DSP_BLOCK_SIZE)
#define SPORT_DMA_Y_COUNT (-(DSP_BLOCK_SIZE - 1) * DSP_WORD_SIZE)

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
static fract32 g_codec_tx_buffer_a[DSP_CHANNEL_COUNT][DSP_BLOCK_SIZE];
static fract32 g_codec_tx_buffer_b[DSP_CHANNEL_COUNT][DSP_BLOCK_SIZE];
static fract32 g_codec_tx_buffer_c[DSP_CHANNEL_COUNT][DSP_BLOCK_SIZE];

// SPORT0 DMA receive buffer
static fract32 g_codec_rx_buffer_a[DSP_CHANNEL_COUNT][DSP_BLOCK_SIZE];
static fract32 g_codec_rx_buffer_b[DSP_CHANNEL_COUNT][DSP_BLOCK_SIZE];
static fract32 g_codec_rx_buffer_c[DSP_CHANNEL_COUNT][DSP_BLOCK_SIZE];

static bool g_sport0_block_received = false;
static bool g_sport0_overrun = false;

static t_dma_desc g_sport0_rx_dma_a;
static t_dma_desc g_sport0_rx_dma_b;
static t_dma_desc g_sport0_rx_dma_c;

static t_dma_desc g_sport0_tx_dma_a;
static t_dma_desc g_sport0_tx_dma_b;
static t_dma_desc g_sport0_tx_dma_c;

static uint64_t g_start;
static uint64_t g_stop;
static uint64_t g_elapsed;

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
    *pDMA3_CONFIG = FLOW_LARGE | WDSIZE_32 | NDSIZE_9 | WNR | DMA2D;
    ssync();

    // SPORT0 Tx DMA.
    *pDMA4_PERIPHERAL_MAP = PMAP_SPORT0TX;

    // Linked descriptor mode.
    *pDMA4_NEXT_DESC_PTR = &g_sport0_tx_dma_a;
    *pDMA4_CONFIG = FLOW_LARGE | WDSIZE_32 | NDSIZE_9 | DMA2D;
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

fract32 *sport0_get_rx_buffer(void) {

    return (fract32 *)&(((t_dma_desc *)(*pDMA3_NEXT_DESC_PTR))->start_addr)[0];
}

fract32 *sport0_get_tx_buffer(void) {

    return (fract32 *)&(((t_dma_desc *)(*pDMA4_NEXT_DESC_PTR))->start_addr)[0];
}

bool sport0_block_received(void) { return g_sport0_block_received; }

bool sport0_overrun(void) { return g_sport0_overrun; }

void sport0_block_processed(void) { g_sport0_block_received = false; }

uint64_t sport0_period(void) { return g_elapsed; }

/*----- Static function implementations ------------------------------*/

__attribute__((interrupt_handler)) static void _sport0_isr(void) {

    g_stop = cycles();

    g_elapsed = g_stop - g_start;

    if (g_sport0_block_received) {

        g_sport0_overrun = true;
    }

    // Clear interrupt status.
    *pDMA3_IRQ_STATUS = DMA_DONE;
    ssync();

    g_sport0_block_received = true;

    g_start = cycles();
}

static void _dma_desc_init(void) {

    // Rx Ping.
    g_sport0_rx_dma_a.next_desc = &g_sport0_rx_dma_b;
    g_sport0_rx_dma_a.start_addr = g_codec_rx_buffer_a;
    g_sport0_rx_dma_a.config =
        FLOW_LARGE | DI_EN | WDSIZE_32 | NDSIZE_5 | WNR | DMA2D | DMAEN;

    g_sport0_rx_dma_a.x_count = SPORT_DMA_X_COUNT;
    g_sport0_rx_dma_a.x_mod = SPORT_DMA_X_MOD;
    g_sport0_rx_dma_a.y_count = SPORT_DMA_Y_MOD;
    g_sport0_rx_dma_a.y_mod = SPORT_DMA_Y_COUNT;

    // Rx Pong.
    g_sport0_rx_dma_b.next_desc = &g_sport0_rx_dma_c;
    g_sport0_rx_dma_b.start_addr = g_codec_rx_buffer_b;
    g_sport0_rx_dma_b.config =
        FLOW_LARGE | DI_EN | WDSIZE_32 | NDSIZE_5 | WNR | DMA2D | DMAEN;

    g_sport0_rx_dma_b.x_count = SPORT_DMA_X_COUNT;
    g_sport0_rx_dma_b.x_mod = SPORT_DMA_X_MOD;
    g_sport0_rx_dma_b.y_count = SPORT_DMA_Y_MOD;
    g_sport0_rx_dma_b.y_mod = SPORT_DMA_Y_COUNT;

    // Rx Peng.
    g_sport0_rx_dma_c.next_desc = &g_sport0_rx_dma_a;
    g_sport0_rx_dma_c.start_addr = g_codec_rx_buffer_c;
    g_sport0_rx_dma_c.config =
        FLOW_LARGE | DI_EN | WDSIZE_32 | NDSIZE_5 | WNR | DMA2D | DMAEN;

    g_sport0_rx_dma_c.x_count = SPORT_DMA_X_COUNT;
    g_sport0_rx_dma_c.x_mod = SPORT_DMA_X_MOD;
    g_sport0_rx_dma_c.y_count = SPORT_DMA_Y_MOD;
    g_sport0_rx_dma_c.y_mod = SPORT_DMA_Y_COUNT;

    // Tx Ping.
    g_sport0_tx_dma_a.next_desc = &g_sport0_tx_dma_b;
    g_sport0_tx_dma_a.start_addr = g_codec_tx_buffer_a;
    g_sport0_tx_dma_a.config =
        FLOW_LARGE | WDSIZE_32 | NDSIZE_5 | DMA2D | DMAEN;

    g_sport0_tx_dma_a.x_count = SPORT_DMA_X_COUNT;
    g_sport0_tx_dma_a.x_mod = SPORT_DMA_X_MOD;
    g_sport0_tx_dma_a.y_count = SPORT_DMA_Y_MOD;
    g_sport0_tx_dma_a.y_mod = SPORT_DMA_Y_COUNT;

    // Tx Pong.
    g_sport0_tx_dma_b.next_desc = &g_sport0_tx_dma_c;
    g_sport0_tx_dma_b.start_addr = g_codec_tx_buffer_b;
    g_sport0_tx_dma_b.config =
        FLOW_LARGE | WDSIZE_32 | NDSIZE_5 | DMA2D | DMAEN;

    g_sport0_tx_dma_b.x_count = SPORT_DMA_X_COUNT;
    g_sport0_tx_dma_b.x_mod = SPORT_DMA_X_MOD;
    g_sport0_tx_dma_b.y_count = SPORT_DMA_Y_MOD;
    g_sport0_tx_dma_b.y_mod = SPORT_DMA_Y_COUNT;

    // Tx Peng.
    g_sport0_tx_dma_c.next_desc = &g_sport0_tx_dma_a;
    g_sport0_tx_dma_c.start_addr = g_codec_tx_buffer_c;
    g_sport0_tx_dma_c.config =
        FLOW_LARGE | WDSIZE_32 | NDSIZE_5 | DMA2D | DMAEN;

    g_sport0_tx_dma_c.x_count = SPORT_DMA_X_COUNT;
    g_sport0_tx_dma_c.x_mod = SPORT_DMA_X_MOD;
    g_sport0_tx_dma_c.y_count = SPORT_DMA_Y_MOD;
    g_sport0_tx_dma_c.y_mod = SPORT_DMA_Y_COUNT;
}

/*----- End of file --------------------------------------------------*/
