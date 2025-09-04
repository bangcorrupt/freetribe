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
 * @file    per_hostdma.c
 * @brief   Configuration and handling of HostDMA peripheral.
 * 
 * @details HostDMA communicates with the CPU's EMIFA peripheral, which
 *          offers maximum bandwidth in CPU-DSP data transmission.
 */

/**
 * @brief   HOST_STATUS bits explained in relation to HostDMA state propagation.
 * @details 
 * 
 * Note that there may be a TOCTTOU hazard between steps 3 and 4.
 *
 * \code
 * #   Event                     Bit Changes      Absolute Bit States
 * 
 * 1   Host started writing      ALLOW_CNFG 0     ALLOW_CNFG 0
 *     first config word                          DMA_CMPLT  1
 *                                                DMA_RDY    0
 * 
 * 2   7 word config descriptor                   ALLOW_CNFG 0
 *     is loading                DMA_CMPLT  0     DMA_CMPLT  0
 *                                                DMA_RDY    0
 * 
 * 3   After last config word    ALLOW_CNFG 1     ALLOW_CNFG 1
 *     (YMODIFY) written by                       DMA_CMPLT  1
 *     the host                                   DMA_RDY    0
 * 
 * 4   +1 hardware cycle                          ALLOW_CNFG 1
 *                                                DMA_CMPLT  0
 *                               DMA_RDY    1     DMA_RDY    1
 * 
 * 5   DMA transaction complete  ALLOW_CNFG 1     ALLOW_CNFG 1
 *     (Corresponding ISR fired  DMA_CMPLT  1     DMA_CMPLT  1
 *     and returned gracefully)  DMA_RDY    0     DMA_RDY    0
 * \endcode
 */

/**
 * @brief   Host-write operation explained from peripheral hardware perspective.
 * @details
 * 
 * -- Expected transfer sequence per block --
 * 1. EMIFA driver configures HostDMA using descriptor.
 * 2. HostDP asserts HRDY (HOST_ACK pin low) to tell host FIFO is empty.
 * 3. EMIFA writes data words into HostDP data port.
 * 4. HostDP drives HOST_ACK pin back up again once FIFO is full.
 * 5. HostDMA moves data words from FIFO into destination memory.
 * 6. HostDMA fires DMA1 IRQ with raised DMA_DONE flag.
 * 7. DMA1 ISR sees DMA_DONE; modifies driver state, raises DMA_CMPLT flag.
 * 
 * -- Error scenario #1: DMA error occurs --
 * In this case, DMA1 ISR fires without DMA_DONE raised.
 * For this I already implemented working error recovery.
 * 
 * -- Error scenario #2: FIFO underrun --
 * The host wouldn't have written enough data words.
 * We could implement two solutions:
 * - If FIFOFULL is always set when XCOUNT is reached (even if it's below 16),
 *   our error check becomes (!FIFOFULL && !FIFOEMPTY).
 * - Otherwise, we can still detect the error using a host-side timeout.
 * Currently, an infinite loop happens on host, since I didn't implement a fix.
 * 
 * -- Error scenario #3: FIFO overflow --
 * The host would have written too many data words. This is an edge case that
 * would cause permanent misalignment between EMIFA and FIFO addressing.
 * Unfortunately this wouldn't be detectable with DMA_ERR or !FIFOEMPTY check.
 * But fortunately this can only happen due to bad EMIFA driver design.
 * 
**/

/**
 * @brief   Host-read operation explained from peripheral hardware perspective.
 * @details
 * 
 * The DSP requests a host read transfer by raising the HSHK bit.
 * 
 * -- Expected transfer sequence per block --
 * 1. EMIFA driver configures HostDMA using descriptor.
 * 2. HostDMA moves data words from DSP memory into FIFO.
 * 3. HostDP asserts HRDY (HOST_ACK pin low) to tell host FIFO is full.
 * 4. EMIFA reads data words from HostDP data port.
 * 5. HostDP drives HOST_ACK pin back up again once FIFO is empty.
 * 6. HostDMA fires HOSTRD_DONE IRQ with raised HOSTRD_DONE flag.
 * 7. HOSTRD_DONE ISR sees HOSTRD_DONE; modifies driver state, raises DMA_CMPLT.
 * 
 * -- Error scenario #1: DMA error occurs --
 * I'm not sure if DMA1 IRQ fires with DMA_ERR flag on host read as well,
 * but either way the error is caught in DMA1 ISR or HOSTRD DONE ISR.
 * Current policy is to signal error to host by abusing BTE host status bit.
 * Host will respond with HOSTDP STATUS IRQ, then our driver resets HostDMA.
 * 
 * -- Error scenario #2: Host didn't read all words --
 * Leftover words remain in the FIFO, catchable by making sure FIFOEMPTY is set.
 * 
 * -- Error scenario #3: Host reads too many words from data port --
 * FIFO handles this "gracefully" in hardware by looping back around.
 * 
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <blackfin.h>
#include <builtins.h>

#include "macros.h"

#include "per_hostdma.h"

/*----- Macros -------------------------------------------------------*/

// #define SIMULATE_HOST_READ_ERROR
#ifdef SIMULATE_HOST_READ_ERROR
#   define HOST_READ_ERROR()   (true)
#else
#   define HOST_READ_ERROR()   (!(*pHOST_STATUS & HOSTRD_DONE))
#endif

// #define SIMULATE_HOST_WRITE_ERROR
#ifdef SIMULATE_HOST_WRITE_ERROR
#   define HOST_WRITE_ERROR()  (true)
#else
#   define HOST_WRITE_ERROR()  (!(*pDMA1_IRQ_STATUS & DMA_DONE))
#endif

#define READ_HEADER_16(offset)       (*(volatile uint16_t*)(HOST_TO_DSP_HEADER_BASE+(offset)))
#define READ_HEADER_32(offset)       (*(volatile uint32_t*)(HOST_TO_DSP_HEADER_BASE+(offset)))
#define WRITE_HEADER_16(offset, val)  *(volatile uint16_t*)(DSP_TO_HOST_HEADER_BASE+(offset)) = (val)
#define WRITE_HEADER_32(offset, val)  *(volatile uint32_t*)(DSP_TO_HOST_HEADER_BASE+(offset)) = (val)

#define BURST_MODE_ON_BY_DEFAULT true

#define MAX_EVENTS 16

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    HOSTDMA_OFF = 0,
    HOSTDMA_IDLE,
    HOSTDMA_HOST_WRITE,
    HOSTDMA_HOST_READ_APPROVED,
} t_hostdma_state;

typedef struct {
    uint16_t blocks_remaining;
    t_hostdma_metadata metadata;
} t_host_write_state;

typedef struct {
    uint16_t blocks_remaining;
    t_hostdma_metadata metadata;
} t_host_read_state;

typedef enum {
    EVENT_HOST_READ_COMPLETE,
    EVENT_HOST_WRITE_COMPLETE,
    EVENT_ERROR,
} t_hostdma_event_type;

typedef struct {
    t_hostdma_event_type type;
    t_hostdma_metadata metadata;
} t_hostdma_event;

/*----- Static function prototypes -----------------------------------*/

static inline bool _check_bus_availability();
static inline void _host_control(bool enable_burst_mode, uint16_t extra_flags);
static inline void _handle_error(t_hostdma_metadata metadata);
static inline void _enqueue_event(t_hostdma_event_type type, t_hostdma_metadata metadata);

__attribute__((interrupt_handler)) static void _hostdp_status_isr(void);
__attribute__((interrupt_handler)) static void _hostdp_dma1_isr(void);
__attribute__((interrupt_handler)) static void _host_read_done_isr(void);

#ifdef DEBUG
static void _test_buffer();
#endif

/*----- Static variable definitions ----------------------------------*/

static t_hostdma_cb g_tx_callback;
static t_hostdma_cb g_rx_callback;
static t_hostdma_cb g_error_callback;

static volatile t_hostdma_state g_state;
static volatile t_host_write_state g_rx_state;
static volatile t_host_read_state g_tx_state;

volatile t_hostdma_event g_event_queue[MAX_EVENTS];
volatile int g_event_head = 0;
volatile int g_event_tail = 0;

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initializes HostDMA peripheral.
 * 
 * @note    Registers to IVG 10, 12, and 13
 */
void per_hostdma_init(t_hostdma_cb rx_callback,
                      t_hostdma_cb tx_callback,
                      t_hostdma_cb error_callback) {
    
    // Register HOSTDP status interrupt
    *pSIC_IAR6 |= P49_IVG(13);
    *pEVT13 = &_hostdp_status_isr;
    *pSIC_IMASK1 |= IRQ_HOSTDP_STATUS;
    ssync();
    int i;
    asm volatile("cli %0; bitset(%0, 13); sti %0; csync;" : "=d"(i)); // Unmask in the core event processor
    ssync();

    // Register HOST READ DONE interrupt
    *pSIC_IAR6 |= P50_IVG(12);
    *pEVT12 = &_host_read_done_isr;
    *pSIC_IMASK1 |= IRQ_HOSTRD_DONE;
    ssync();
    asm volatile("cli %0; bitset(%0, 12); sti %0; csync;" : "=d"(i)); // Unmask in the core event processor
    ssync();

    // Register DMA1 HOSTDP interrupt
    *pSIC_IAR3 |= P28_IVG(10);
    *pEVT10 = &_hostdp_dma1_isr;
    *pSIC_IMASK0 |= IRQ_DMA1;
    ssync();
    asm volatile("cli %0; bitset(%0, 10); sti %0; csync;" : "=d"(i)); // Unmask in the core event processor
    ssync();

#ifdef DEBUG
    _test_buffer();
#endif

    g_rx_callback      = rx_callback;
    g_tx_callback      = tx_callback;
    g_error_callback   = error_callback;

    // Configure HostDMA
    _host_control(BURST_MODE_ON_BY_DEFAULT, 0x0000);

    g_state = HOSTDMA_IDLE;
}

/**
 * @brief   Polls event queue to execute callbacks deferred from ISRs.
 */
void per_hostdma_process_events() {

    while (g_event_tail != g_event_head) {
        t_hostdma_event evt = g_event_queue[g_event_tail];
        g_event_tail = (g_event_tail + 1) % MAX_EVENTS;

        switch (evt.type) {
            case EVENT_HOST_READ_COMPLETE:
                g_tx_callback(evt.metadata);
                break;
            case EVENT_HOST_WRITE_COMPLETE:
                g_rx_callback(evt.metadata);
                break;
            case EVENT_ERROR:
                g_error_callback(evt.metadata);
                break;
        }
    }

}


/**
 * @brief   Requests to transfer a block of data to the host.
 * 
 * @param   host_address  Address in the host's virtual memory map to write to.
 * @param   words         Buffer of data.
 * @param   word_count    Number of words, which must be even.
 * @param   metadata      Gets sent along with the transfer inside the header.
 * 
 * @return  HOSTDMA_SUCCESS on success, otherwise an error code.
 *          HOSTDMA_BUS_OCCUPIED
 */
t_hostdma_status per_hostdma_transfer(uint32_t host_address,
                                      const uint16_t *words,
                                      uint16_t word_count,
                                      t_hostdma_metadata metadata) {
    
    if (HOSTDMA_OFF == g_state)
        return HOSTDMA_UNINITIALISED;
    
    if (!_check_bus_availability())
        return HOSTDMA_BUS_OCCUPIED;

    // Prepare a header for transfer first
    WRITE_HEADER_16(0x00, word_count     );
    WRITE_HEADER_32(0x04, host_address   );
    WRITE_HEADER_32(0x08, (uint32_t)words);
    WRITE_HEADER_32(0x0C, metadata.meta0 );
    WRITE_HEADER_32(0x10, metadata.meta1 );
    WRITE_HEADER_32(0x14, metadata.meta2 );
    WRITE_HEADER_32(0x18, metadata.meta3 );
    WRITE_HEADER_32(0x1C, metadata.meta4 );

    g_tx_state.metadata = metadata;
    g_tx_state.blocks_remaining = ((uint32_t)word_count + 15) / 16;
    
    // Request to claim bus
    *pHOST_STATUS |= HSHK;
    ssync();

    return HOSTDMA_SUCCESS;

}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Check whether it is appropriate to request a host read operation.
 * @details
 *          - Driver state machine must be in idle state
 *          - HSHK bit clear, otherwise a previous request would be pending
 *          Normally, we would check hostdma status to see whether the bus is
 */
static inline bool _check_bus_availability() {
    uint16_t status = *pHOST_STATUS;
    return  (HOSTDMA_IDLE == g_state)
        // &&  (status & ALLOW_CNFG)
        // && !(status & DMA_RDY)
        // &&  (status & DMA_CMPLT)
        && !(status & HSHK);
}

static inline void _host_control(bool enable_burst_mode, uint16_t extra_flags) {
    if (enable_burst_mode)
        *pHOST_CONTROL = INT_MODE | EHW | EHR | HOSTDP_DATA_SIZE | HOSTDP_EN | extra_flags | BDR;
    else
        *pHOST_CONTROL = INT_MODE | EHW | EHR | HOSTDP_DATA_SIZE | HOSTDP_EN | extra_flags;
    ssync();
}

/**
 * @brief   Defer a callback to guarantee ISR safety and stability.
 * 
 * @param   type      Callback type
 * @param   metadata  Transfer metadata to pass to callback
 */
static inline void _enqueue_event(t_hostdma_event_type type, t_hostdma_metadata metadata) {

    int next_head = (g_event_head + 1) % MAX_EVENTS;
    if (next_head == g_event_tail) {
        // Overflow problem, this could happen when the CPU keeps firing
        // transactions and the DSP main loop stalls for too long, being unable
        // to serve event callbacks.

        // @todo: Handle this situation:
        //        - Reset queue
        //        - Put driver in halting error state, but ignore host's
        //          acknowledgement (HOST IRQ) so HostDMA doesn't turn back on.
        //          Best option would be to stash HOST IRQ as an event callback.
        //        - Force software interrupt to call IPC layer error callback:
        //          IPC layer will fire all it's user callbacks with IPC_FAILED.
        //        - Once DSP breaks out of the bad loop the stashed HOST IRQ is
        //          served, which means HostDMA restarts and greenlights host.
        return;
    }
    g_event_queue[g_event_head].type = type;
    g_event_queue[g_event_head].metadata = metadata;
    g_event_head = next_head;
}

/**
 * @brief   Called from inside ISR when DMA tx/rx failed.
 * @details Policy is to restart HostDP and to tell host we errored.
 *          1. Turn off state machine
 *          2. Disable DMA interrupts
 *          3. Raise BTE status flag to signal error to host
 * 
 * @param   metadata  Transfer metadata to pass to the error callback
 */
static inline void _handle_error(t_hostdma_metadata metadata) {

    g_state = HOSTDMA_OFF;
    *pSIC_IMASK0 &= ~IRQ_DMA1;
    *pSIC_IMASK1 &= ~IRQ_HOSTRD_DONE;
    _host_control(BURST_MODE_ON_BY_DEFAULT, BT_EN);
    _enqueue_event(EVENT_ERROR, metadata);

}

/*----- ISR function implementations ---------------------------------*/


/**
 * @brief   Software interrupt requested by the host.
 */
__attribute__((interrupt_handler)) static void _hostdp_status_isr(void) {

    *pHOST_STATUS |= HIRQ;
    ssync();

    // HostDP errored if suddenly the state machine is off. The host requested
    // this interrupt to tell us to restart HostDMA.
    // 1. Start state machine again
    // 2. Re-enable DMA interrupts
    // 3. Reset HostDMA peripheral
    // 4. Clear BTE flag as greenlight for host to continue operation as usual
    if (HOSTDMA_OFF == g_state) {

        g_state = HOSTDMA_IDLE;
        *pSIC_IMASK0 |= IRQ_DMA1;
        *pSIC_IMASK1 |= IRQ_HOSTRD_DONE;
        *pHOST_CONTROL |= HOSTDP_RST;
        ssync();
        *pHOST_CONTROL &= ~BT_EN;
        ssync();
        return;

    }

}

/**
 * @brief   Happens when the host has read (emptied) the entire FIFO, or when
 *          a DMA error occurred during this process.
 */
__attribute__((interrupt_handler)) static void _host_read_done_isr(void) {
    
    if (HOST_READ_ERROR()) {
        _handle_error(g_tx_state.metadata);
#ifdef SIMULATE_HOST_READ_ERROR
        // Clear IRQ status to prevent retriggering
        volatile uint16_t keep_hshk = (*pHOST_STATUS & HSHK);
        *pHOST_STATUS = HOSTRD_DONE | keep_hshk;
        ssync();
#endif
        return;
    }

    if (1 == g_tx_state.blocks_remaining) {
        
        // For now we just turn off burst mode for every last block,
        // burst mode expects power of two block lengths.
        _host_control(false, 0x0000);
        
    }
    else if (0 == g_tx_state.blocks_remaining) {
        
        g_state = HOSTDMA_IDLE;
        _host_control(BURST_MODE_ON_BY_DEFAULT, 0x0000);
        *pHOST_STATUS &= ~HSHK;
        ssync();
        _enqueue_event(EVENT_HOST_READ_COMPLETE, g_tx_state.metadata);
        
    }
    g_tx_state.blocks_remaining--;
    
    volatile uint16_t keep_hshk = (*pHOST_STATUS & HSHK);
    *pHOST_STATUS = HOSTRD_DONE | DMA_CMPLT | keep_hshk;
    ssync();

}

/**
 * @brief   Fires when HostDMA channel (DMA1) finished moving data from/to FIFO.
 *          Also when a DMA error had occurred.
 */
__attribute__((interrupt_handler)) static void _hostdp_dma1_isr(void) {
    
    if (HOST_WRITE_ERROR()) {
        _handle_error(g_rx_state.metadata);
        // Clear IRQ status to prevent retriggering
#ifdef SIMULATE_HOST_WRITE_ERROR
        *pDMA1_IRQ_STATUS = DMA_DONE;
#else
        *pDMA1_IRQ_STATUS = DMA_ERR; // note: I haven't verified whether this actually raises yet
#endif
        ssync();
        return;
    }

    switch (g_state) {
        
        case HOSTDMA_IDLE: {

            g_state = HOSTDMA_HOST_WRITE;

            // First block of host write operation,
            // we've just received the header.
            g_rx_state.blocks_remaining = READ_HEADER_16(0x00);
            g_rx_state.metadata.meta0   = READ_HEADER_32(0x0C);
            g_rx_state.metadata.meta1   = READ_HEADER_32(0x10);
            g_rx_state.metadata.meta2   = READ_HEADER_32(0x14);
            g_rx_state.metadata.meta3   = READ_HEADER_32(0x18);
            g_rx_state.metadata.meta4   = READ_HEADER_32(0x1C);
            
            bool header_only_transfer = (0 == g_rx_state.blocks_remaining);
            if (header_only_transfer) {

                g_state = HOSTDMA_IDLE;
                _host_control(BURST_MODE_ON_BY_DEFAULT, 0x0000);
                _enqueue_event(EVENT_HOST_WRITE_COMPLETE, g_rx_state.metadata);

            } else {

                bool burst_next_block = (g_rx_state.blocks_remaining > 1);
                _host_control(burst_next_block, 0x0000);

            }

        } break;

        case HOSTDMA_HOST_WRITE: {
            
            bool last_block = (0 == g_rx_state.blocks_remaining);
            if (last_block) {

                _host_control(BURST_MODE_ON_BY_DEFAULT, 0x0000);
                g_state = HOSTDMA_IDLE;
                _enqueue_event(EVENT_HOST_WRITE_COMPLETE, g_rx_state.metadata);

            } else {

                bool burst_next = (g_rx_state.blocks_remaining > 1);
                _host_control(burst_next, 0x0000);

            }

            g_rx_state.blocks_remaining--;

        } break;

        default: break;

    }

    *pDMA1_IRQ_STATUS = DMA_DONE;
    ssync();
    volatile uint16_t keep_hshk = (*pHOST_STATUS & HSHK);
    *pHOST_STATUS = DMA_CMPLT | keep_hshk;
    ssync();

}


/*--------------------------------------------------------------------*/

#ifdef DEBUG
static void _test_buffer() {
    volatile uint16_t *ptr = (uint16_t*)(0x00000060);
    uint16_t number = 0;
    int j;
    for (j = 0; j < 512; j++) {
        *ptr++ = number++;
    }
}
#endif
