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

#include "macros.h"

#include "per_emifa.h"
#include "per_aintc.h"
#include "per_gpio.h"

/*----- Macros -------------------------------------------------------*/

#define HOST_ACK_PIN 98
#define HOST_ACK_GPIO_INT_CHANNEL 9

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
#define HOST_STATUS_BTE (1 << 9)

#define HOST_CONFIG_WNR (1 << 1)   // 0=Host read, 1=Host write
#define HOST_CONFIG_DMA2D (1 << 4) // 0=Linear   , 1=Two-dimensional
#define HOST_CONFIG_FLOW (1 << 12) // 0=Stop mode, 1=Autobuffer mode

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    EMIFA_OFF = 0,
    EMIFA_IDLE,
    EMIFA_ERROR,
    EMIFA_HOST_WRITE_HEADER_CONFIG,
    EMIFA_HOST_WRITE_HEADER_TRANSFER,
    EMIFA_HOST_WRITE_CONFIG,
    EMIFA_HOST_WRITE_TRANSFER,
    EMIFA_HOST_WRITE_DONE,
    EMIFA_HOST_READ_HEADER,
    EMIFA_HOST_READ_CONFIG,
    EMIFA_HOST_READ_TRANSFER,
    EMIFA_HOST_READ_DONE,
} t_emifa_state;

typedef struct {
    uint16_t words_total;
    uint16_t words_remaining;
    uint16_t block_count;
    uint16_t block_length;
    uint32_t current_dsp_address;
    const uint16_t *input_ptr;
    t_emifa_metadata metadata;
} t_host_write_state;

typedef struct {
    uint16_t words_total;
    uint16_t words_remaining;
    uint16_t block_length;
    uint16_t *host_address;
    uint32_t dsp_address;
    t_emifa_metadata metadata;
} t_host_read_state;

/*----- Static function prototypes -----------------------------------*/

// State handlers (per_emifa_task)
static void _handle_off();
static void _handle_idle();
static void _handle_error();
static void _handle_host_write_header_config();
static void _handle_host_write_config();
static void _handle_host_write_done();
static void _handle_host_read_config();
static void _handle_host_read_done();
static void _handle_transfer_states();

// ISR state handling
static inline void _isr_host_write_header_transfer();
static inline void _isr_host_write_transfer();
static inline void _isr_host_read_header();
static inline void _isr_host_read_transfer();
static void _host_ack_isr(void);

// HostDMA control functions
static void _request_hostdp_status_interrupt();
static void _issue_dma_finish_command();
static void _hostdma_config(uint32_t dsp_address, uint16_t word_count, uint16_t extra_flags);
static void _catch_dma_errors();

// Bit helpers
static inline bool _allow_config();
static inline bool _is_dma_ready();
static inline bool _is_dma_complete();
static inline bool _dma_dir_bit();
static inline bool _is_fifo_full();
static inline bool _is_fifo_empty();
static inline bool _handshake_bit();
static inline bool _bus_timeout_enabled();
static inline bool _was_hostdma_initialized();

#ifdef DEBUG
static void _print_hostdma_state();
static void _print_emifa_state();
static void _gen_dummy_data();
#endif

/*----- Static variable definitions ----------------------------------*/

static t_emifa_idle_callback  g_idle_callback;
static t_emifa_tx_callback    g_tx_callback;
static t_emifa_rx_callback    g_rx_callback;
static t_emifa_error_callback g_error_callback;

static volatile t_emifa_state g_state;
static volatile t_host_write_state g_tx_state;
static volatile t_host_read_state g_rx_state;

// State dispatch table (per_emifa_task)
typedef void (*t_emifa_state_handler)(void);
static const t_emifa_state_handler g_state_handlers[] = {
    _handle_off,
    _handle_idle,
    _handle_error,
    _handle_host_write_header_config,
    NULL, // header transfer happens in ISR
    _handle_host_write_config,
    NULL, // data transfer happens in ISR
    _handle_host_write_done,
    NULL, // read header happens in ISR
    _handle_host_read_config,
    NULL, // read data happens in ISR
    _handle_host_read_done,
};

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Sets up the EMIFA engine.
 * 
 * @param   idle_callback   Fires when state machine is idle.
 * @param   tx_callback     Fires once a transfer completes.
 * @param   rx_callback     Fires when we received data from the DSP.
 * @param   error_callback  Fires when a DMA error was encountered and
 *                          the active transaction had been flushed.
 */
void per_emifa_init(t_emifa_idle_callback idle_callback,
                    t_emifa_tx_callback tx_callback,
                    t_emifa_rx_callback rx_callback,
                    t_emifa_error_callback error_callback) {

    EMIFAWaitTimingConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_WAITTIME_CONFIG(
         0, // wset   Write setup time or width in EMA_CLK cycles
         3, // wstb   Write strobe time or width in EMA_CLK cycles
         0, // whld   Write hold time or width in EMA_CLK cycles
         0, // rset   Read setup time or width in EMA_CLK cycles
         3, // rstb   Read strobe time or width in EMA_CLK cycles
         0, // rhld   Read hold time or width in EMA_CLK cycles
         0  // ta     Minimum Turn-Around time
    ));
    EMIFAAsyncDevOpModeSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_ASYNC_INTERFACE_NORMAL_MODE);
    EMIFAAsyncDevDataBusWidthSelect(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_DATA_BUSWITTH_16BIT);
    EMIFAExtendedWaitConfig(SOC_EMIFA_0_REGS, EMIFA_CHIP_SELECT_2, EMIFA_EXTENDED_WAIT_DISABLE);
    
    // Configure EMIFA interrupts
    EMIFAMskedIntDisable(SOC_EMIFA_0_REGS, EMIFA_ASYNC_TIMOUT_INT);
    EMIFAMskedIntDisable(SOC_EMIFA_0_REGS, EMIFA_LINE_TRAP_INT);
    EMIFAMskedIntDisable(SOC_EMIFA_0_REGS, EMIFA_WAIT_RISE_INT);

    // HOST_ACK pin interrupt (HRDY/FRDY)
    per_aintc_register_gpio_interrupt(
        HOST_ACK_GPIO_INT_CHANNEL,
        HOST_ACK_PIN,
        GPIO_INT_TYPE_FALLEDGE,
        _host_ack_isr
    );

    g_idle_callback  = idle_callback;
    g_tx_callback    = tx_callback;
    g_rx_callback    = rx_callback;
    g_error_callback = error_callback;

#ifdef DEBUG
    _gen_dummy_data();
#endif
}

/**
 * @brief   Perform a non-blocking transfer.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   words         Pointer to the buffer of 16-bit words to write
 * @param   word_count    Number of 16-bit words to write. Cannot be uneven!
 * @param   metadata      Gets sent along through the header.
 */
t_emifa_status per_emifa_transfer(uint32_t dsp_address,
                                  const uint16_t *words,
                                  uint16_t word_count,
                                  t_emifa_metadata metadata) {

    if (EMIFA_OFF == g_state) {
        return EMIFA_UNINITIALISED;
    }
    
    if (EMIFA_IDLE != g_state) {
        return EMIFA_BUS_OCCUPIED;
    }
    
    DEBUG_LOG("per_emifa_transfer");
    g_tx_state.words_total         = word_count;
    g_tx_state.words_remaining     = word_count;
    g_tx_state.current_dsp_address = dsp_address;
    g_tx_state.input_ptr           = words;
    g_tx_state.block_count         = (word_count + 15) / 16;
    g_tx_state.metadata            = metadata;
    
    g_state = EMIFA_HOST_WRITE_HEADER_CONFIG;
    _handle_host_write_header_config(); // don't wait for next driver tick
    _catch_dma_errors();

    return EMIFA_SUCCESS;

}

/**
 * @brief   Update tick of the EMIFA engine.
 */
void per_emifa_task() {

    if (g_state < (sizeof(g_state_handlers) / sizeof(g_state_handlers[0]))
     && g_state_handlers[g_state]) {
        g_state_handlers[g_state]();
    }

    _catch_dma_errors();

#ifdef DEBUG
    // _print_emifa_state();
    // _print_hostdma_state();
    // ft_printf("");
#endif
}

/*----- Poll state handlers ------------------------------------------*/

/**
 * @brief   When HostDMA has not yet initialised itself on the DSP end
 *          we will wait for it do to so.
 */
void _handle_off() {
    if (_was_hostdma_initialized()) {
        g_state = EMIFA_IDLE;
    }
}

/**
 * @brief   In idle state, poll for DSP request to lock the bus (HSHK bit).
 *          If asserted, configure for a host read operation.
 */
void _handle_idle() {

    if (_handshake_bit() && _allow_config()) {

        g_state = EMIFA_HOST_READ_HEADER;
        _hostdma_config(DSP_TO_HOST_HEADER_BASE, DSP_TO_HOST_HEADER_LENGTH, 0x0000);

    } else {
        g_idle_callback(); // serve device layer
    }
}

/**
 * @brief   DMA errors occurred; request HostDP status IRQ which tells HostDMA
 *          driver to reset HostDMA.
 */
void _handle_error() {

    _request_hostdp_status_interrupt(); // I wonder if this request could ever fail
    g_error_callback(_dma_dir_bit() ? g_tx_state.metadata : g_rx_state.metadata);
    g_state = EMIFA_OFF;

}

/* Host read */

/**
 * @brief   A data transfer just happend, so now poll for a new configuration
 *          opportunity.
 */
void _handle_host_read_config() {

    if (_allow_config()) {
        g_state = EMIFA_HOST_READ_TRANSFER;
        g_rx_state.block_length = MIN(g_rx_state.words_remaining, 16);
        _hostdma_config(g_rx_state.dsp_address, g_rx_state.block_length, 0x0000);
    }
    
}

/**
 * @brief   Tell DSP operation has completed and reset driver in idle state.
 */
void _handle_host_read_done() {

    if (_is_dma_complete() && (EMIFA_ERROR != g_state)) {
        g_state = EMIFA_IDLE;
        g_rx_callback(g_rx_state.metadata);
    }

}

/* Host write */

/**
 * @brief   First action to happen when CPU requested a host write operation.
 *          We check to see if we can configure HostDMA.
 */
void _handle_host_write_header_config() {
    if (_allow_config()) {
        g_state = EMIFA_HOST_WRITE_HEADER_TRANSFER;
        _hostdma_config(HOST_TO_DSP_HEADER_BASE, HOST_TO_DSP_HEADER_LENGTH, HOST_CONFIG_WNR);
    }
}

/**
 * @brief   If possible, tell HostDMA that we want to transfer a new block of data.
 */
void _handle_host_write_config() {
    if (_allow_config()) {
        g_state = EMIFA_HOST_WRITE_TRANSFER;
        g_tx_state.block_length = MIN(g_tx_state.words_remaining, 16);
        _hostdma_config(g_tx_state.current_dsp_address, g_tx_state.block_length, HOST_CONFIG_WNR);
    }
}

/**
 * @brief   Reset driver to idle state.
 */
void _handle_host_write_done() {
    
    if (_is_dma_complete() && (EMIFA_ERROR != g_state)) {
        g_state = EMIFA_IDLE;
        g_tx_callback();
    }

}


/*----- ISR state handling -------------------------------------------*/

/* Host write */

static inline void _isr_host_write_header_transfer() {

    *HDMA_DATA_PORT = g_tx_state.block_count;
    *HDMA_DATA_PORT = 0x0000;
    *HDMA_DATA_PORT = 0x0000;
    *HDMA_DATA_PORT = 0x0000;
    *HDMA_DATA_PORT = 0x0000;
    *HDMA_DATA_PORT = 0x0000;
    *HDMA_DATA_PORT = LO16(g_tx_state.metadata.meta0);
    *HDMA_DATA_PORT = HI16(g_tx_state.metadata.meta0);

    *HDMA_DATA_PORT = LO16(g_tx_state.metadata.meta1);
    *HDMA_DATA_PORT = HI16(g_tx_state.metadata.meta1);
    *HDMA_DATA_PORT = LO16(g_tx_state.metadata.meta2);
    *HDMA_DATA_PORT = HI16(g_tx_state.metadata.meta2);
    *HDMA_DATA_PORT = LO16(g_tx_state.metadata.meta3);
    *HDMA_DATA_PORT = HI16(g_tx_state.metadata.meta3);
    *HDMA_DATA_PORT = LO16(g_tx_state.metadata.meta4);
    *HDMA_DATA_PORT = HI16(g_tx_state.metadata.meta4);

    // Empty transfers are possible, which only sends some metadata
    bool no_payload = (0 == g_tx_state.words_total);
    g_state = no_payload ? EMIFA_HOST_WRITE_DONE
                         : EMIFA_HOST_WRITE_CONFIG;

}

static inline void _isr_host_write_transfer() {

    for (int i = 0; i < g_tx_state.block_length; i++) {
        *HDMA_DATA_PORT = *g_tx_state.input_ptr++;
    }

    g_tx_state.words_remaining -= g_tx_state.block_length;
    g_tx_state.current_dsp_address += g_tx_state.block_length * sizeof(uint16_t);

    bool last_block = (0 == g_tx_state.words_remaining);
    g_state = last_block ? EMIFA_HOST_WRITE_DONE
                         : EMIFA_HOST_WRITE_CONFIG;
}

/* Host read */

static inline void _isr_host_read_header() {

    uint16_t _discard;

    uint16_t word_count      = *HDMA_DATA_PORT;
    _discard                 = *HDMA_DATA_PORT;
    uint16_t host_address_lo = *HDMA_DATA_PORT;
    uint16_t host_address_hi = *HDMA_DATA_PORT;
    uint16_t dsp_address_lo  = *HDMA_DATA_PORT;
    uint16_t dsp_address_hi  = *HDMA_DATA_PORT;
    uint16_t meta0_lo        = *HDMA_DATA_PORT;
    uint16_t meta0_hi        = *HDMA_DATA_PORT;

    uint16_t meta1_lo        = *HDMA_DATA_PORT;
    uint16_t meta1_hi        = *HDMA_DATA_PORT;
    uint16_t meta2_lo        = *HDMA_DATA_PORT;
    uint16_t meta2_hi        = *HDMA_DATA_PORT;
    uint16_t meta3_lo        = *HDMA_DATA_PORT;
    uint16_t meta3_hi        = *HDMA_DATA_PORT;
    uint16_t meta4_lo        = *HDMA_DATA_PORT;
    uint16_t meta4_hi        = *HDMA_DATA_PORT;

    g_rx_state.host_address    = (uint16_t*)COMBINE16(host_address_hi, host_address_lo);
    g_rx_state.dsp_address     = (uint32_t) COMBINE16(dsp_address_hi , dsp_address_lo );
    g_rx_state.words_total     = word_count;
    g_rx_state.words_remaining = word_count;

    g_rx_state.metadata.meta0 = COMBINE16(meta0_hi, meta0_lo);
    g_rx_state.metadata.meta1 = COMBINE16(meta1_hi, meta1_lo);
    g_rx_state.metadata.meta2 = COMBINE16(meta2_hi, meta2_lo);
    g_rx_state.metadata.meta3 = COMBINE16(meta3_hi, meta3_lo);
    g_rx_state.metadata.meta4 = COMBINE16(meta4_hi, meta4_lo);

    g_state = EMIFA_HOST_READ_CONFIG;

}

static inline void _isr_host_read_transfer() {

    for (int i = 0; i < g_rx_state.block_length; i++) {
        *g_rx_state.host_address++ = *HDMA_DATA_PORT;
    }

    g_rx_state.words_remaining -= g_rx_state.block_length;
    g_rx_state.dsp_address += g_rx_state.block_length * sizeof(uint16_t);

    if (0 == g_rx_state.words_remaining) {
        g_state = EMIFA_HOST_READ_DONE;
    } else {
        g_state = EMIFA_HOST_READ_CONFIG;
    }
}


/**
 * @brief   ISR for HRDY signal; HOST_ACK pin was driven to LOW.
 * 
 * @details About the HOST_ACK pin states:
 *            In host write mode:
 *            - LOW  means FIFO empty (host should write data) "HRDY"
 *            - HIGH means FIFO full (host filled it up)
 *            In host read mode:
 *            - LOW  means FIFO full (host should read data) "HRDY"
 *            - HIGH means FIFO empty (host read all data)
 */
static void _host_ack_isr(void) {

    per_aintc_clear_status_gpio(HOST_ACK_PIN);

    switch (g_state) {
        case EMIFA_HOST_WRITE_HEADER_TRANSFER: _isr_host_write_header_transfer(); break;
        case EMIFA_HOST_WRITE_TRANSFER: _isr_host_write_transfer(); break;
        case EMIFA_HOST_READ_HEADER: _isr_host_read_header(); break;
        case EMIFA_HOST_READ_TRANSFER: _isr_host_read_transfer(); break;
        default: break;
    }

}

/*--------------------------------------------------------------------*/




/**
 * @brief   Send configuration descriptor to HostDMA.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to read/write to.
 * @param   word_count    Max 16, since the FIFO size is 16 16-bit words.
 *                        With burst mode, value must be power of 2 and above 2.
 * @param   extra_flags   HOST_CONFIG flags. use HOST_CONFIG_WNR for host write mode.
 */
static void _hostdma_config(uint32_t dsp_address, uint16_t word_count, uint16_t extra_flags) {

    *HDMA_CONFIG_PORT = 0x00A9 | extra_flags;
    *HDMA_CONFIG_PORT = LO16(dsp_address);
    *HDMA_CONFIG_PORT = HI16(dsp_address);
    *HDMA_CONFIG_PORT = word_count; // XCOUNT
    *HDMA_CONFIG_PORT = 2;          // XMODIFY
    *HDMA_CONFIG_PORT = 1;          // YCOUNT
    *HDMA_CONFIG_PORT = 1;          // YMODIFY

}

/**
 * @brief   Triggers hostdp status IRQ on the DSP. Cannot be used while HostDMA
 *          port is waiting for configuration.
 */
static void _request_hostdp_status_interrupt() {
    *(volatile uint16_t*)HDMA_CONFIG_PORT = 0x1C; // HOST IRQ
}

/**
 * @brief   DMA FINISH command performs the same functions as the HOSTDP reset
 *          (HOSTDP_RST) bit in the HOST_CONTROL register, except that it modifies
 *          the HOST_STATUS register contents and stops any DMA activity. The DMA
 *          FINISH command may not complete immediately but completes only after
 *          the DAB state machine has moved to a particular idle state.
 */
static void _issue_dma_finish_command() {
    *(volatile uint16_t*)HDMA_CONFIG_PORT = 0x2C; // DMA FINISH
}

/**
 * @brief   Bus timeout enabled bit in HOST_STATUS is abused to serve as an
 *          error flag.
 */
static void _catch_dma_errors() {

    // Bus timeout enabled is a flag I abuse to signal an error
    if (_bus_timeout_enabled() && EMIFA_OFF != g_state) {
        g_state = EMIFA_ERROR;
    }

}



/*----- Bit helpers --------------------------------------------------*/

static inline bool _allow_config()        { return (*HDMA_CONFIG_PORT & HOST_STATUS_ALLOW_CNFG) != 0x0000; }
static inline bool _is_fifo_full()        { return (*HDMA_CONFIG_PORT & HOST_STATUS_FIFOFULL  ) != 0x0000; }
static inline bool _is_fifo_empty()       { return (*HDMA_CONFIG_PORT & HOST_STATUS_FIFOEMPTY ) != 0x0000; }
static inline bool _is_dma_ready()        { return (*HDMA_CONFIG_PORT & HOST_STATUS_DMA_RDY   ) != 0x0000; }
static inline bool _is_dma_complete()     { return (*HDMA_CONFIG_PORT & HOST_STATUS_DMA_CMPLT ) != 0x0000; }
static inline bool _dma_dir_bit()         { return (*HDMA_CONFIG_PORT & HOST_STATUS_DMA_DIR   ) != 0x0000; }
static inline bool _handshake_bit()       { return (*HDMA_CONFIG_PORT & HOST_STATUS_HSHK      ) != 0x0000; }
static inline bool _bus_timeout_enabled() { return (*HDMA_CONFIG_PORT & HOST_STATUS_BTE       ) != 0x0000; }

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
    return !((status & HOST_STATUS_FIFOFULL) && (status & HOST_STATUS_FIFOEMPTY))
         &&  (status & HOST_STATUS_ALLOW_CNFG) // a previous error was handled
         && !(status & HOST_STATUS_BTE);       // ... ditto ...
}




/*----- Debug --------------------------------------------------------*/

#ifdef DEBUG

/**
 * @brief   Fills DDR2 ram buffer with dummy data.
 */
static void _gen_dummy_data() {
    uint16_t *ptr = (uint16_t*)0xC0000000;
    for (int i = 0; i < 64; i++) {
        *ptr++ = i;
    }
}

/**
 * @brief   Useful for print debugging.
 */
static void _print_hostdma_state() {
    DEBUG_LOG("[HostDMA state] allow_cnfg:%i dma_rdy:%i   dma_cmplt:%i hshk:%i",
        (int)_allow_config(),
        (int)_is_dma_ready(),
        (int)_is_dma_complete(),
        (int)_handshake_bit()
    );
    DEBUG_LOG("                fifofull:%i   fifoempty:%i dma_dir:%i   bte:%i",
        (int)_is_fifo_full(),
        (int)_is_fifo_empty(),
        (int)_dma_dir_bit(),
        (int)_bus_timeout_enabled()
    );
}

/**
 * @brief   Useful for print debugging.
 */
static void _print_emifa_state() {
    static const char* STATE_STRS[] = {
        "EMIFA_OFF",
        "EMIFA_IDLE",
        "EMIFA_ERROR",
        "EMIFA_HOST_WRITE_HEADER_CONFIG",
        "EMIFA_HOST_WRITE_HEADER_TRANSFER",
        "EMIFA_HOST_WRITE_CONFIG",
        "EMIFA_HOST_WRITE_TRANSFER",
        "EMIFA_HOST_WRITE_DONE",
        "EMIFA_HOST_READ_HEADER",
        "EMIFA_HOST_READ_CONFIG",
        "EMIFA_HOST_READ_TRANSFER",
        "EMIFA_HOST_READ_DONE",
    };
    switch (g_state) {
        case EMIFA_HOST_WRITE_HEADER_CONFIG:
        case EMIFA_HOST_WRITE_HEADER_TRANSFER:
        case EMIFA_HOST_WRITE_CONFIG:
        case EMIFA_HOST_WRITE_TRANSFER:
        case EMIFA_HOST_WRITE_DONE:
            DEBUG_LOG("[EMIFA state] %s (tx) total: %i left: %i block: %i",
                STATE_STRS[g_state],
                (int)g_tx_state.words_total,
                (int)g_tx_state.words_remaining,
                (int)g_tx_state.block_length
            );
            break;

        case EMIFA_HOST_READ_HEADER:
        case EMIFA_HOST_READ_CONFIG:
        case EMIFA_HOST_READ_TRANSFER:
        case EMIFA_HOST_READ_DONE:
            DEBUG_LOG("[EMIFA state] %s (rx) total: %i left: %i block: %i dsp: %p host: %p",
                STATE_STRS[g_state],
                (int)g_rx_state.words_total,
                (int)g_rx_state.words_remaining,
                (int)g_rx_state.block_length,
                (int*)g_rx_state.dsp_address,
                (int*)g_rx_state.host_address
            );
            break;

        default:
            DEBUG_LOG("[EMIFA state] %s ", STATE_STRS[g_state]);
            break;
    }

}

#endif

/*----- End of file --------------------------------------------------*/
