#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <blackfin.h>
#include <builtins.h>

#include "per_hostdma.h"
#include "per_gpio.h"

/*----- Macros -------------------------------------------------------*/

// #define PGMUX_HOSTDP 0x2800
// #define PGFER_SPI 0x001e
// #define PGFER_UART 0x0180
// #define PGFER_HOSTDP 0xf800
// #define PHFER_HOSTDP 0xffff

/*----- Static function prototypes -----------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static bool g_initialised = false;
static bool g_reject_host_read = false;

/*----- Extern variable definitions ----------------------------------*/

__attribute__((interrupt_handler)) static void _hostdp_status_isr(void);
__attribute__((interrupt_handler)) static void _hostdp_dma1_isr(void);
__attribute__((interrupt_handler)) static void _host_read_done_isr(void);
static inline bool _is_bus_available_hardware();
static void _test_buffer();

/*----- Extern function implementations ------------------------------*/

void per_hostdma_init() {

    // Register HOSTDP status interrupt
    *pSIC_IAR6 |= P49_IVG(10);
    ssync();
    *pEVT10 = &_hostdp_status_isr;
    ssync();
    *pSIC_IMASK1 |= IRQ_HOSTDP_STATUS;
    ssync();
    int i;
    asm volatile("cli %0; bitset(%0, 10); sti %0; csync;" : "=d"(i)); // Unmask in the core event processor
    ssync();

    // Register HOST READ DONE interrupt
    *pSIC_IAR6 |= P50_IVG(13);
    ssync();
    *pEVT13 = &_host_read_done_isr;
    ssync();
    *pSIC_IMASK1 |= IRQ_HOSTRD_DONE;
    ssync();
    asm volatile("cli %0; bitset(%0, 13); sti %0; csync;" : "=d"(i)); // Unmask in the core event processor
    ssync();

    // Register DMA1 HOSTDP interrupt
    *pSIC_IAR3 |= P28_IVG(12);
    ssync();
    *pEVT12 = &_hostdp_dma1_isr;
    ssync();
    *pSIC_IMASK0 |= IRQ_DMA1;
    ssync();
    asm volatile("cli %0; bitset(%0, 12); sti %0; csync;" : "=d"(i)); // Unmask in the core event processor
    ssync();

    _test_buffer();

    per_hostdma_reset();
    g_initialised = true;
}

void per_hostdma_reset() {
    // Configure HostDMA
    // *pHOST_STATUS = 0x000C; // reset: DMA_CMPLT | FIFOEMPTY
    // ssync();
    *pHOST_CONTROL = BDR | EHW | EHR | HOSTDP_DATA_SIZE | HOSTDP_EN; // @TODO: this necessary every time during DSP transfer?
    ssync();
}

/**
 * @brief   Transfers a block of data to the host.
 * 
 * @param   host_address Address in the host's virtual memory map to write to.
 * @param   words        Buffer of data
 * @param   word_count   Number of words, must be atleast 2
 * 
 * @return  0 on success, otherwise an error
 */
t_hostdma_status per_hostdma_transfer(uint32_t host_address, uint16_t *words, uint16_t word_count) {

    if (!g_initialised)
        return HOSTDMA_UNINITIALISED;

    // First check to quickly see if the host is occupied right now.
    // This does not mean we are free to go yet since we need a handshake
    // to occur between us and the host in order to safely send data.
    if (!_is_bus_available_hardware())
        return HOSTDMA_BUS_OCCUPIED;
    
    // Calculate block count; Each block is a full FIFO unless there
    // are less words remaining. Only exception is when the block has
    // only one word left, then we add an additional word for padding.
    // This is to make burst mode happy.
    uint16_t block_count = ((uint32_t)word_count + 15) / 16;

    // Prepare a header for transfer first
    *(volatile uint16_t*)0x00000000 = word_count;
    *(volatile uint16_t*)0x00000002 = block_count;
    *(volatile uint16_t*)0x00000004 =  host_address        & 0xFFFF;
    *(volatile uint16_t*)0x00000006 = (host_address >> 16) & 0xFFFF;
    *(volatile uint16_t*)0x00000008 =  (uint32_t)words        & 0xFFFF;
    *(volatile uint16_t*)0x0000000A = ((uint32_t)words >> 16) & 0xFFFF;

    // @TODO: Make it unable to handshake if handshake bit is already set by host!

    // Signal to host using handshake bit that we want to initiate a host_read.
    *pHOST_STATUS |= HSHK;
    ssync();
    while (*pHOST_STATUS & HSHK) {

        // ... Wait for host to acknowledge our request by clearing HSHK bit,
        //     this happens when the last data word is read by the host.

        // If the host issues a HOST STATUS IRQ it means the host already claimed
        // the bus.
        if (g_reject_host_read) {
            *pHOST_STATUS &= ~HSHK;
            ssync();
            g_reject_host_read = false;
            return HOSTDMA_BUS_OCCUPIED;
        }
    }

    while (!(*pHOST_STATUS & DMA_CMPLT)) {
        // @TODO: Handle host-side errors that cause DSP to infinitely stall here!
        //        (maybe using bus timeout?!)
        //        (or detect a HOSTDP reset command??? (dma finish))
    }
    per_hostdma_reset();

    // Block transfer loop; All we have to do is wait for the DMA
    // to happen, and reset the HostDMA peripheral afterwards.
    uint16_t i;
    for (i = 0; i < block_count; i++) {
        while (!(*pHOST_STATUS & DMA_CMPLT)) {
            // @TODO: Handle host-side errors that cause DSP to infinitely stall here!
            //        (maybe using bus timeout?!)
            //        (or detect a HOSTDP reset command??? (dma finish))
        }
        per_hostdma_reset();
    }

    return HOSTDMA_SUCCESS;
}

/**
 * @brief
 */
bool per_hostdma_is_bus_available() {
    uint16_t status = *pHOST_STATUS;
    return !(status & DMA_RDY) && (status & ALLOW_CNFG);
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Software interrupt requested by the host.
 *          In our implementation the host uses to tell us that it's
 *          busy doing a host write operation. Hence, we cancel our pending
 *          operation.
 */
__attribute__((interrupt_handler)) static void _hostdp_status_isr(void) {

    *pHOST_STATUS |= HIRQ; // write 1 to clear
    ssync();
    g_reject_host_read = true;
    // *pHOST_STATUS &= ~HSHK;
    // ssync();
}

/**
 *
 */
__attribute__((interrupt_handler)) static void _hostdp_dma1_isr(void) {
    
    // Error check
    if (!(*pDMA1_IRQ_STATUS & DMA_DONE)) {
        // *pHOST_STATUS |= HSHK; // signal error to host
        // ssync();
        return; // @TODO: HANDLE ERRORS
    }
    
    *pDMA1_IRQ_STATUS = DMA_DONE; // write 1 to clear
    ssync();
    *pHOST_STATUS = DMA_CMPLT;
    ssync();
}

/**
 * @brief   Happens when the host has read the entire FIFO. We signal
 *          DMA_CMPLT so that the host can break out of it's blocking
 *          operation.
 */
__attribute__((interrupt_handler)) static void _host_read_done_isr(void) {
    
    // // Error check
    // if (!(*pHOST_STATUS & HOSTRD_DONE)) {
    //     // *pHOST_STATUS |= HSHK; // signal error to host
    //     // ssync();
    //     return; // @TODO: HANDLE ERRORS
    // }

    *pHOST_STATUS = HOSTRD_DONE; // write 1 to clear
    ssync();
    *pHOST_STATUS = DMA_CMPLT;
    ssync();
}

static inline bool _is_bus_available_hardware() {
    uint16_t status = *pHOST_STATUS;
    return !(status & DMA_RDY) && (status & ALLOW_CNFG);
}

static void _test_buffer() {
    volatile uint16_t *ptr = (uint16_t*)(0x00000010);
    uint16_t number = 0;
    int j;
    for (j = 0; j < 64; j++) {
        *ptr++ = number++;
    }
    *(volatile uint16_t*)0x00000002 = 0x1234;
    *(volatile uint16_t*)0x00000004 = 0xAABB;
    *(volatile uint16_t*)0x00000006 = 0xCCDD;
    *(volatile uint16_t*)0x00000008 = 0xEEFF;
}