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
 * @file    per_emifa.c.
 *
 * @brief   Configuration and handling of EMIFA peripheral.
 *          EMIFA communicates with the DSP's HostDMA engine and has the
 *          highest bandwidth for transmitting data between CPU and DSP.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "soc_AM1808.h"
#include "csl_interrupt.h"
#include "csl_gpio.h"
#include "csl_emifa.h"
#include "hw_emifa2.h"
#include "hw_types.h"
#include "hw_syscfg0_AM1808.h"
#include "per_emifa.h"
#include "per_aintc.h"
#include "per_gpio.h"
#include "freetribe.h" // ft_printf

/*----- Macros -------------------------------------------------------*/

#ifndef MIN
#define MIN(A,B) ((A) < (B) ? (A) : (B))
#endif

#define EMIFA_ASYNC_CFG2 HWREG(SOC_EMIFA_0_REGS + EMIFA_CE2CFG)

#define HDMA_DATA_PORT ((volatile uint16_t*)(SOC_EMIFA_CS2_ADDR + 0x00))
#define HDMA_CONFIG_PORT ((volatile uint16_t*)(SOC_EMIFA_CS2_ADDR + 0x02))

#define HOST_STATUS_DMA_RDY (1 << 0)
#define HOST_STATUS_FIFOFULL (1 << 1)
#define HOST_STATUS_FIFOEMPTY (1 << 2)
#define HOST_STATUS_DMA_CMPLT (1 << 3)
#define HOST_STATUS_HSHK (1 << 4)
#define HOST_STATUS_HOSTDP_TOUT (1 << 5)
#define HOST_STATUS_HIRQ (1 << 6)
#define HOST_STATUS_ALLOW_CNFG (1 << 7)
#define HOST_STATUS_DMA_DIR (1 << 8)

#define HOST_CONFIG_WNR (1 << 1)   // 0=Host read, 1=Host write
#define HOST_CONFIG_DMA2D (1 << 4) // 0=Linear   , 1=Two-dimensional
#define HOST_CONFIG_FLOW (1 << 12) // 0=Stop mode, 1=Autobuffer mode

/*----- Typedefs -----------------------------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_emifa_status _emifa_config(uint32_t start_address, int word_count, uint16_t extra_flags);
static inline bool _is_dma_ready();
static inline bool _is_fifo_full();
static inline bool _is_fifo_empty();
static inline bool _is_dma_complete();
static inline bool _is_handshake_bit_set();
static inline bool _is_allow_config_bit_set();
static inline bool _was_hostdma_initialized();
static void _request_host_status_interrupt();

/*----- Static variable definitions ----------------------------------*/

static bool g_hostdma_started = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Sets up the EMIFA engine.
 */
void per_emifa_init() {

    // EMA_CLK is 10~ ns per cycle?
    // @TODO: Calculate actual values and maybe adjust
    EMIFAWaitTimingConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_WAITTIME_CONFIG(
        11, // wset   Write setup time or width in EMA_CLK cycles
         6, // wstb   Write strobe time or width in EMA_CLK cycles
         6, // whld   Write hold time or width in EMA_CLK cycles
        11, // rset   Read setup time or width in EMA_CLK cycles
         6, // rstb   Read strobe time or width in EMA_CLK cycles
         6, // rhld   Read hold time or width in EMA_CLK cycles
        11  // ta     Minimum Turn-Around time
    ));
    EMIFAAsyncDevOpModeSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_INTERFACE_NORMAL_MODE);
    EMIFAAsyncDevDataBusWidthSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_DATA_BUSWITTH_16BIT);
    EMIFAExtendedWaitConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_EXTENDED_WAIT_DISABLE);
    
    // Configure EMIFA interrupts
    EMIFAMskedIntDisable(SOC_EMIFA_0_REGS, EMIFA_ASYNC_TIMOUT_INT);
    EMIFAMskedIntDisable(SOC_EMIFA_0_REGS, EMIFA_LINE_TRAP_INT);
    EMIFAMskedIntDisable(SOC_EMIFA_0_REGS, EMIFA_WAIT_RISE_INT);

    // Dummy data for debugging
    uint16_t *ptr = (uint16_t*)0xC0000000;
    for (int i = 0; i < 64; i++) {
        *ptr++ = i;
    }

}

/**
 * @brief   Poll for incoming HostDMA requests. When the bus is occupied
 *          nothing is able to come through. When the HostDMA connection
 *          has not been initialised we will wait for it to do so.
 */
void per_emifa_poll() {

    if (!g_hostdma_started) {
        if (!_was_hostdma_initialized()) {
            return;
        }
        ft_print("HostDMA started!");
        g_hostdma_started = true;
    }
    

    // Poll for incoming transmissions; The DSP requests a host read by
    // setting the handshake bit.
    if (_is_handshake_bit_set()) {

        // Read header:
        // 1. Send the 7 configuration words
        // 2. Wait for config descriptor to initialize DMA engine
        // 3. Send host status IRQ to DSP to clear handshake bit (acknowledges request)
        // 4. Wait for DMA transaction to complete.
        _emifa_config(0x00000000, 6, 0x0000);
        uint16_t word_count      = *HDMA_DATA_PORT;
        uint16_t block_count     = *HDMA_DATA_PORT;
        uint16_t host_address_lo = *HDMA_DATA_PORT;
        uint16_t host_address_hi = *HDMA_DATA_PORT;
        uint16_t dsp_address_lo  = *HDMA_DATA_PORT;
        uint16_t dsp_address_hi  = *HDMA_DATA_PORT;

        // ... The handshake bit has been automatically cleared now that we
        //     read the last header word ...

        while (!_is_dma_complete()) {
            // @TODO: Raise error if handshake bit is set again by ISR (indicating error)!

            // < IF TIMING IS WRONG EVENTUALLY IT WILL COMPLETELY HANG HERE >
            ft_print("per_emifa_poll() waiting for completion of DMA transfer");
        }

        uint16_t *host_address = (uint16_t*)(host_address_lo | (host_address_hi << 16));
        uint32_t dsp_address = (uint32_t)(dsp_address_lo | (dsp_address_hi << 16));
        
        uint16_t words_remaining = word_count;

        while (words_remaining != 0) {

            uint16_t num_block_words = (words_remaining < 16) ? words_remaining : 16;
            // ft_printf("per_emifa_poll()   reading %i words of remaining %i", (int)num_block_words, (int)words_remaining);
            _emifa_config(dsp_address, num_block_words, 0x0000);

            for (int i = 0; i < num_block_words; i++) {
                // ft_printf("per_emifa_poll() reading word:%i num_block_words:%i remaining:%i", i, num_block_words, words_remaining);
                *host_address++ = *HDMA_DATA_PORT;
            }

            words_remaining -= num_block_words;
            dsp_address += 16 * sizeof(uint16_t); // increment up by an entire FIFO, actually
                                                  // should be num_block_words, but once we
                                                  // reach this after last block is received
                                                  // we exit the loop anyway and dsp_address
                                                  // variable won't be needed anymore.

            while (!_is_dma_complete()) {
                // @TODO: Raise error if handshake bit is set again by ISR (indicating error)!
                ft_print("per_emifa_poll() waiting for block transaction to complete...");
            }
            
        }

        // ft_print("per_emifa_poll() COMPLETE:");
        // host_address = (uint16_t*)(host_address_lo | (host_address_hi << 16));
        // for (int i = 0; i < word_count; i++) {
        //     ft_printf("%04X", *host_address++);
        // }
        // ft_print("per_emifa_poll() END HOST READ");
        host_address = (uint16_t*)(host_address_lo | (host_address_hi << 16));
        if (*host_address != 0x0000) {
            ft_print("HOSTDMA ROUNDTRIP FAILED");
        }

    }

}

/**
 * @brief   Perform a blocking transfer.
 *          @TODO: MUST BE 32-BIT INTS!
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   words         Pointer to the buffer of 16-bit words to write
 * @param   word_count    Number of 16-bit words to write. Cannot be uneven amount!
 */
t_emifa_status per_emifa_transfer(uint32_t dsp_address, uint16_t *words, uint16_t word_count) {

    if (!g_hostdma_started) {
        // ft_print("per_emifa_transfer() HostDMA uninitialized: cancelling transfer.");
        return EMIFA_UNINITIALISED;
    }

    // Check if host bit is set indicating it is waiting for us to acknowledge
    // a transfer. Since this is a blocking operation on the DSP side, we want
    // acknowledge it as fast as possible, this means the DSP's request needs
    // to be prioritised over the host's.
    if (_is_handshake_bit_set()) {
        // ft_print("per_emifa_transfer() DSP wants request: cancelling transfer!");
        return EMIFA_BUS_OCCUPIED; // prioritises the DSP over host
    }

    // ft_print("per_emifa_transfer() start ...");

    int32_t words_remaining = word_count;
    while (words_remaining > 0) {

        // ft_printf("per_emifa_transfer() words remaining: %i", (int)words_remaining);
        int32_t num_words = MIN(words_remaining, 16);

        _emifa_config(dsp_address, num_words, HOST_CONFIG_WNR);
        if (_is_handshake_bit_set()) {
            _request_host_status_interrupt();
        }
        
        for (int i = 0; i < num_words; i++) {
            *HDMA_DATA_PORT = *words++;
        }
        
        words_remaining -= num_words;
        dsp_address += num_words * sizeof(uint16_t);
    }

    // ft_print("per_emifa_transfer() end ...");
    return EMIFA_SUCCESS;

}

/**
 * @brief   Whenever the EMIFA bus is occupied it's impossible for any new
 *          transmissions to happen.
 * 
 * @return  True if EMIFA bus currently occupied transferring or receiving.
 */
bool per_emifa_is_bus_available() {
    uint16_t status = *HDMA_CONFIG_PORT;
    return !(status & HOST_STATUS_HSHK) && !(status & HOST_STATUS_DMA_RDY) && (status & HOST_STATUS_ALLOW_CNFG);
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Configure HostDMA for reading/writing.
 * 
 * @param   start_address Address in the DSP's virtual memory map to read/write to.
 * @param   word_count    Max 16, since the FIFO size is 16 16-bit words.
 *                        MUST be atleast 2, otherwise burst mode breaks by causing
 *                        infinite wait for DMA_RDY.
 * @param   extra_flags   HOST_CONFIG flags. use HOST_CONFIG_WNR for host write mode.
 */
static t_emifa_status _emifa_config(uint32_t start_address, int word_count, uint16_t extra_flags) {

    while (!_is_allow_config_bit_set()) {
        ft_print("_emifa_config _is_allow_config_bit_set() FALSE!!!");
        // EMIFA timing probably too fast, not sure !
        // return EMIFA_BUS_OCCUPIED; // this should never happen
    }
    
    *HDMA_CONFIG_PORT = 0x00A9 | extra_flags;
    *HDMA_CONFIG_PORT =  start_address        & 0xFFFF;
    *HDMA_CONFIG_PORT = (start_address >> 16) & 0xFFFF;
    *HDMA_CONFIG_PORT = word_count; // XCOUNT
    *HDMA_CONFIG_PORT = 2;          // XMODIFY
    *HDMA_CONFIG_PORT = 1;          // YCOUNT
    *HDMA_CONFIG_PORT = 1;          // YMODIFY

    while (!_is_dma_ready()) {}

    return EMIFA_SUCCESS;
    
}

static inline bool _is_dma_ready() {
    return (*HDMA_CONFIG_PORT & HOST_STATUS_DMA_RDY);
}

static inline bool _is_fifo_full() {
    return (*HDMA_CONFIG_PORT & HOST_STATUS_FIFOFULL);
}

static inline bool _is_fifo_empty() {
    return (*HDMA_CONFIG_PORT & HOST_STATUS_FIFOEMPTY);
}

static inline bool _is_dma_complete() {
    return (*HDMA_CONFIG_PORT & HOST_STATUS_DMA_CMPLT);
}

static inline bool _is_handshake_bit_set() {
    return (*HDMA_CONFIG_PORT & HOST_STATUS_HSHK);
}

static inline bool _is_allow_config_bit_set() {
    return (*HDMA_CONFIG_PORT & HOST_STATUS_ALLOW_CNFG);
}

/**
 * @brief   This is a trick; Before the HostDMA peripheral is initialized on the
 *          DSP side, the FIFOFULL and FIFOEMPTY in HOST_STATUS are both set.
 *          Ofcourse this is an invalid state and from this we can deduce that
 *          the HostDMA peripheral has yet to be initialized by the DSP.
 * 
 * @returns True if HostDMA peripheral has been initialized by the DSP.
 */
static inline bool _was_hostdma_initialized() {
    uint16_t status = *HDMA_CONFIG_PORT;
    return !((status & HOST_STATUS_FIFOFULL) && (status & HOST_STATUS_FIFOEMPTY));
}

/**
 * @brief   Signals IRQ to DSP to clear handshake bit. This is to acknowledge
 *          the DSP's request for a host read operation.
 *          Note that this can only be called after DMA_RDY is set.
 */
static void _request_host_status_interrupt() {
    // *(volatile uint8_t*)HDMA_CONFIG_PORT = 0x1C; // HOST IRQ
    // *(volatile uint8_t*)HDMA_CONFIG_PORT = 0x1C; // HOST IRQ
    *(volatile uint16_t*)HDMA_CONFIG_PORT = 0x1C; // HOST IRQ
}


