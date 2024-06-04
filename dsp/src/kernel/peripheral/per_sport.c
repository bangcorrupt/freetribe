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

/*
 * @file    per_sport.c
 *
 * @brief   Peripheral driver for for BF523 SPORT.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include <blackfin.h>
#include <builtins.h>

#include "per_gpio.h"
#include "per_sport.h"

#include "knl_profile.h"

/*----- Macros and Definitions ---------------------------------------*/

#define DTYPE_SIGX 0x0004 // SPORTx RCR1 Data Format Sign Extend.

/*----- Static variable definitions ----------------------------------*/

// SPORT0 DMA receive buffer
__attribute__((section(".l1.data.a")))
__attribute__((aligned(32))) static fract32 g_codec_rx_buffer[BUFFER_LENGTH];

// SPORT0 DMA transmit buffer
// __attribute__((section(".l1.data.b")))
__attribute__((section(".l1.data.a")))
__attribute__((aligned(32))) static fract32 g_codec_tx_buffer[BUFFER_LENGTH];

// 2 channels of input from ADC.
// __attribute__((section(".l1.data.a")))
__attribute__((section(".l1.data.b")))
__attribute__((aligned(32))) static t_audio_buffer g_codec_in;

// 2 channels of output to DAC.
__attribute__((section(".l1.data.b")))
__attribute__((aligned(32))) static t_audio_buffer g_codec_out;

volatile static bool g_sport0_tx_complete = false;
volatile static bool g_sport0_rx_complete = false;

static uint32_t g_sport_isr_period[CYCLE_LOG_LENGTH];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _sport0_rx_isr(void) __attribute__((interrupt_handler));
static void _sport0_tx_isr(void) __attribute__((interrupt_handler));

/*----- Extern function implementations ------------------------------*/

/// TODO: DMA ping-pong block buffer.

void sport0_init(void) {

    /// TODO: Do we need secondary enabled?
    ///         If secondary is enabled,
    ///         we must account for the
    ///         interleaved data when
    ///         handling DMA buffers.

    // Configure SPORT0 Rx.
    // Clock Falling Edge, Receive Frame Sync, Data Format Sign Extend.
    *pSPORT0_RCR1 = RCKFE | RFSR | DTYPE_SIGX;
    // Rx Stereo Frame Sync Enable, Rx Word Length 32 bit.
    *pSPORT0_RCR2 = RSFSE | SLEN(0x1f);

    // Configure SPORT0 Tx.
    *pSPORT0_TCR1 = TCKFE | TFSR;
    *pSPORT0_TCR2 = TSFSE | SLEN(0x1f);
    ssync();

    /// TODO: DMA linked descriptor mode.

    // SPORT0 Rx DMA.
    *pDMA3_PERIPHERAL_MAP = PMAP_SPORT0RX;
    *pDMA3_CONFIG = FLOW_AUTO | WDSIZE_32 | WNR | DI_EN;
    // Start address of data buffer.
    *pDMA3_START_ADDR = &g_codec_rx_buffer;
    // DMA inner loop count.
    *pDMA3_X_COUNT = BUFFER_LENGTH;
    // Inner loop address increment
    *pDMA3_X_MODIFY = SAMPLE_SIZE;
    ssync();

    // SPORT0 Tx DMA.
    *pDMA4_PERIPHERAL_MAP = PMAP_SPORT0TX;
    *pDMA4_CONFIG = FLOW_AUTO | WDSIZE_32 | DI_EN;
    // Start address of data buffer
    *pDMA4_START_ADDR = &g_codec_tx_buffer;
    // DMA inner loop count
    *pDMA4_X_COUNT = BUFFER_LENGTH;
    // Inner loop address increment
    *pDMA4_X_MODIFY = SAMPLE_SIZE;
    ssync();

    /// TODO: Review interrupt priorities.

    // SPORT0 Tx DMA4 interrupt IVG9.
    *pSIC_IAR2 |= P17_IVG(9);
    // SPORT0 Rx DMA3 interrupt IVG10.
    *pSIC_IAR2 |= P16_IVG(10);
    ssync();

    // Set SPORT0 Rx interrupt vector.
    *pEVT9 = _sport0_tx_isr;
    *pEVT10 = _sport0_rx_isr;
    ssync();

    // Enable SPORT0 Rx interrupt.
    *pSIC_IMASK0 |= IRQ_DMA3;
    *pSIC_IMASK0 |= IRQ_DMA4;
    ssync();

    int i;
    // unmask in the core event processor
    asm volatile("cli %0; bitset(%0, 9); bitset(%0, 10); sti %0; csync;"
                 : "+d"(i));
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

    // /// TODO: Do we need secondary enabled?
    //
    // // Configure SPORT1 Rx.
    // *pSPORT1_RCR1 = RCKFE | RFSR | DTYPE_SIGX;
    // *pSPORT1_RCR2 = RSFSE | RXSE | SLEN(0x1f);
    //
    // // Configure SPORT1 Tx.
    // *pSPORT1_TCR1 = TCKFE | TFSR;
    // *pSPORT1_TCR2 = TSFSE | TXSE | SLEN(0x1f);
    // ssync();
    //
    // /// TODO: DMA linked descriptor mode.
    //
    // // SPORT1 Rx DMA.
    // *pDMA5_PERIPHERAL_MAP = PMAP_SPORT1RX;
    // // Configure DMA.
    // *pDMA5_CONFIG = FLOW_AUTO | WDSIZE_32 | WNR;
    // // Start address of data buffer.
    // *pDMA5_START_ADDR = &g_cpu_rx_buffer;
    // // DMA inner loop count.
    // *pDMA5_X_COUNT = 2; // 2 samples.
    // // Inner loop address increment.
    // *pDMA5_X_MODIFY = 4; // 32 bit.
    // ssync();
    //
    // // SPORT1 Tx DMA.
    // *pDMA6_PERIPHERAL_MAP = PMAP_SPORT1TX;
    // // Configure DMA.
    // *pDMA6_CONFIG = FLOW_AUTO | WDSIZE_32;
    // // Start address of data buffer
    // *pDMA6_START_ADDR = &g_cpu_tx_buffer;
    // // DMA inner loop count
    // *pDMA6_X_COUNT = 2; // 2 samples.
    // // Inner loop address increment
    // *pDMA6_X_MODIFY = 4; // 32 bit.
    // ssync();
    //
    // // Enable SPORT1 Rx DMA.
    // *pDMA5_CONFIG |= DMAEN;
    // ssync();
    //
    // // Enable SPORT1 Tx DMA.
    // *pDMA6_CONFIG |= DMAEN;
    // ssync();
    //
    // // Enable SPORT1 Rx.
    // *pSPORT1_RCR1 |= RSPEN;
    // ssync();
    //
    // // Enable SPORT1 Tx.
    // *pSPORT1_TCR1 |= TSPEN;
    // ssync();
}

inline t_audio_buffer *sport0_get_rx_buffer(void) { return &g_codec_in; }

inline t_audio_buffer *sport0_get_tx_buffer(void) { return &g_codec_out; }

inline bool sport0_frame_received(void) {

    return g_sport0_rx_complete && g_sport0_tx_complete;
}

inline void sport0_frame_processed(void) {

    g_sport0_tx_complete = false;
    g_sport0_rx_complete = false;
}

/*----- Static function implementations ------------------------------*/

__attribute__((interrupt_handler)) static void _sport0_rx_isr(void) {

    // *pPORTGIO_SET = HWAIT;
    // Clear interrupt status.
    *pDMA3_IRQ_STATUS = DMA_DONE;
    ssync();

    /// TODO: DMA ping-pong block buffer.
    //
    int j;
    // for (j = 0; j < BUFFER_LENGTH; j++) {
    for (j = 0; j < BLOCK_SIZE; j++) {

        // Get input from codec.
        g_codec_in[0][j] = g_codec_rx_buffer[j * 2];
        g_codec_in[1][j] = g_codec_rx_buffer[j * 2 + 1];
    }

    g_sport0_rx_complete = true;
    // *pPORTGIO_CLEAR = HWAIT;
}

__attribute__((interrupt_handler)) static void _sport0_tx_isr(void) {
    // *pPORTGIO_SET = HWAIT;

    // Clear interrupt status.
    *pDMA4_IRQ_STATUS = DMA_DONE;
    ssync();

    /// TODO: DMA ping-pong block buffer.
    //
    int k;
    // for (k = 0; k < BUFFER_LENGTH; k++) {
    for (k = 0; k < BLOCK_SIZE; k++) {

        g_codec_tx_buffer[k * 2] = g_codec_out[0][k];
        g_codec_tx_buffer[k * 2 + 1] = g_codec_out[1][k];
    }

    g_sport0_tx_complete = true;
    // *pPORTGIO_CLEAR = HWAIT;
}

/*----- End of file --------------------------------------------------*/
