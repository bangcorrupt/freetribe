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
 * @file    dev_cpu_ipc.c.
 *
 * @brief   IPC device layer to transmit, request, and receive data
 *          to and from the CPU. Transmissions are served using a queue
 *          system.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>

#include "macros.h"
#include "ring_buffer.h"

#include "per_hostdma.h"

#include "dev_cpu_ipc.h"

/*----- Macros -------------------------------------------------------*/

#define QUEUE_SIZE 32 // must be power of two

/*----- Typedefs -----------------------------------------------------*/

typedef uint16_t t_ipc_op;
enum {
    IPC_OP_TRANSFER = 0, // standalone data transfer
    IPC_OP_REQUEST  = 1, // part of a read request (initiator)
    IPC_OP_RESPONSE = 2  // response to a read request
};

typedef struct {
    uint32_t            cpu_address;
    const uint32_t     *buffer;
    uint16_t            count;
    void              (*callback)(void *, t_ipc_status);
    void               *user_ctx;
    t_hostdma_metadata  meta;
} t_transfer;

// (Bidirectional) metadata for data read request
typedef struct {
    t_ipc_op            op_type;
    uint16_t            count;
    uint32_t            src;
    uint32_t           *dest;
    void              (*callback)(void *, t_ipc_status);
    void               *user_ctx;
} t_meta_read;

// (Bidirectional) metadata for standalone transfer
typedef struct {
    t_ipc_op            op_type;
    uint16_t            _meta0_hi;
    uint32_t            _meta1;
    uint32_t            _meta2;
    void              (*callback)(void *, t_ipc_status);
    void               *user_ctx;
} t_meta_tx;

STATIC_ASSERT(sizeof(t_hostdma_metadata) == sizeof(t_meta_tx  ), tx_metadata_fits);
STATIC_ASSERT(sizeof(t_hostdma_metadata) == sizeof(t_meta_read), rx_metadata_fits);

/*----- Static variable definitions ----------------------------------*/

static rbd_t        g_rbd;
static t_transfer   g_queue[QUEUE_SIZE];
static t_transfer   g_active_tx;

// Read requests must be bookkept to prevent losing callbacks in case of error
static rbd_t        g_inflight_rbd;
static t_transfer   g_inflight_reads[QUEUE_SIZE];

static bool         g_tx_in_progress;
static bool         g_lock_driver;

/*----- Static function prototypes -----------------------------------*/

static t_ipc_status _transfer(uint32_t           cpu_address, 
                              const uint32_t    *buffer,
                              uint16_t           count,
                              void             (*callback)(void*, t_ipc_status),
                              void              *user_ctx,
                              t_hostdma_metadata meta);

static void         _serve_queued_transfer();
static t_ipc_status _enqueue_transfer   (const t_transfer *tx);
static bool         _try_immediate_send (const t_transfer *tx);
static void         _store_inflight_read(const t_transfer *tx);

// Callback handling
static void         _on_host_write_done(t_hostdma_metadata meta);
static void         _on_host_read_done (t_hostdma_metadata meta);
static void         _on_error          (t_hostdma_metadata meta);

static t_ipc_op _get_op_type(t_hostdma_metadata meta);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialize high speed communication channel with the CPU.
 */
void dev_cpu_ipc_init() {

    rb_attr_t attr = {
        sizeof(g_queue[0]),
        ARRAY_SIZE(g_queue),
        g_queue
    };
    ring_buffer_init(&g_rbd, &attr);

    // The inflight queue is merely here to keep track of data read requests
    // currently waiting for host to respond. When an error occurs we need this
    // queue to safely wrap up these transfers.
    rb_attr_t inflight_attr = {
        sizeof(g_inflight_reads[0]),
        ARRAY_SIZE(g_inflight_reads),
        g_inflight_reads
    };
    ring_buffer_init(&g_inflight_rbd, &inflight_attr);

    per_hostdma_init(&_on_host_write_done, &_on_host_read_done, &_on_error);
}

/**
 * @brief   Process HostDMA events and attempt to start transfer if queued.
 */
void dev_cpu_ipc_tick() {

    per_hostdma_process_events();
    _serve_queued_transfer();

}

/**
 * @brief   Queue a request to transfer data from the DSP to CPU memory.
 * 
 * @param   cpu_address   Address in the CPU's virtual memory map to write to.
 * @param   buffer        Pointer to source buffer of 32-bit words.
 * @param   count         Number of 32-bit values to write.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 *
 * @return  IPC_SUCCESS          - transfer started or queued
 *          IPC_INVALID_ARGUMENT - arguments are out of range
 *          IPC_QUEUE_FULL       - the request queue is full
 *          IPC_DRIVER_LOCKED    - the driver cannot accept any new requests
 */
t_ipc_status dev_cpu_ipc_transfer(uint32_t cpu_address, 
                                  const uint32_t *buffer,
                                  uint16_t count,
                                  void (*callback)(void*, t_ipc_status),
                                  void *user_ctx) {

    t_meta_tx meta = {
        .op_type     = IPC_OP_TRANSFER,
        .callback    = callback,
        .user_ctx    = user_ctx
    };
    return _transfer(cpu_address,
                     buffer,
                     count,
                     callback,
                     user_ctx,
                     STRUCT_CAST(t_hostdma_metadata, meta));

}

/**
 * @brief   Request a buffer of data from the CPU's memory.
 * 
 * @details Request an empty host read transfer with only metadata in the
 *          header that instructs the CPU side IPC layer to send back data.
 * 
 * @param   cpu_address   Address in the CPU's virtual memory map to read from.
 * @param   destination   Destination buffer to write requested data in.   
 * @param   count         Number of 32-bit values to request.
 * @param   callback      Function called on failure or completion.
 * @param   user_ctx      User provided context for callback function.
 * 
 * @return  IPC_SUCCESS          - transfer started or queued
 *          IPC_INVALID_ARGUMENT - arguments are out of range
 *          IPC_QUEUE_FULL       - the request queue is full
 *          IPC_DRIVER_LOCKED    - the driver cannot accept any new requests
 */
t_ipc_status dev_cpu_ipc_request_data(uint32_t   cpu_address,
                                      uint32_t  *destination,
                                      uint16_t   count,
                                      void     (*callback)(void*, t_ipc_status),
                                      void      *user_ctx) {

    t_meta_read meta = {
        .op_type    = IPC_OP_REQUEST,
        .count      = count,
        .src        = cpu_address,
        .dest       = destination,
        .callback   = callback,
        .user_ctx   = user_ctx
    };

    return _transfer(DSP_TO_HOST_HEADER_BASE,
                     NULL,
                     0,
                     NULL,
                     NULL,
                     STRUCT_CAST(t_hostdma_metadata, meta)); 
    
}



/*----- Static function implementations ------------------------------*/



/**
 * @brief   Queue a request to transfer data from the DSP to CPU memory.
 * 
 * @param   cpu_address   Address in the CPU's virtual memory map to write to.
 * @param   buffer        Pointer to source buffer of 32-bit words.
 * @param   count         Number of 32-bit values to write.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 * @param   metadata      Gets passed to header.
 *
 * @return  IPC_SUCCESS          - transfer started or queued
 *          IPC_INVALID_ARGUMENT - arguments are out of range
 *          IPC_QUEUE_FULL       - the request queue is full
 *          IPC_DRIVER_LOCKED    - the driver cannot accept any new requests
 */
static t_ipc_status _transfer(uint32_t           cpu_address, 
                              const uint32_t    *buffer,
                              uint16_t           count,
                              void             (*callback)(void*, t_ipc_status),
                              void              *user_ctx,
                              t_hostdma_metadata meta) {
    
    if (g_lock_driver)
        return IPC_DRIVER_LOCKED;

    // Max transfer length is 65535 16-bit words
    if (count >= 32768)
        return IPC_INVALID_ARGUMENT;

    t_transfer tx = {
        .cpu_address = cpu_address,
        .buffer      = buffer,
        .count       = count,
        .callback    = callback,
        .user_ctx    = user_ctx,
        .meta        = meta
    };

    if (_try_immediate_send(&tx))
        return IPC_SUCCESS;

    return _enqueue_transfer(&tx);
}

/**
 * @brief   Attempt to start the transfer immediately if driver/bus is idle.
 * 
 * @return  true if started, false if request must be queued.
 */
static bool _try_immediate_send(const t_transfer *tx) {
    
    if (g_tx_in_progress)
        return false;

    g_tx_in_progress = true;
    g_active_tx      = *tx;

    const uint16_t words16 = 2 * tx->count; // HostDMA expects 16-bit word count
    
    t_hostdma_status st = per_hostdma_transfer(tx->cpu_address,
                                               (const uint16_t*)tx->buffer,
                                               words16,
                                               tx->meta);
    
    if (HOSTDMA_SUCCESS != st) {
        g_tx_in_progress = false;
        return false;
    }
    
    if (IPC_OP_REQUEST == _get_op_type(tx->meta)) {
        _store_inflight_read(tx);
    }

    return true;
}

/**
 * @brief   Put host write transfer in the queue.
 */
static t_ipc_status _enqueue_transfer(const t_transfer *tx) {

    if (ring_buffer_put(g_rbd, tx)) {
        return IPC_QUEUE_FULL;
    }
    return IPC_SUCCESS;
    
}

/**
 * @brief   Try to start and consume next transfer.
 */
static void _serve_queued_transfer() {

    if (!g_tx_in_progress && rb_data_ready(g_rbd)) {

        t_transfer tx;
        ring_buffer_get(g_rbd, &tx);
        if (!_try_immediate_send(&tx))
            _enqueue_transfer(&tx);

    }

}

/**
 * @brief   Data read requests must be bookkept to prevent losing callbacks in
 *          case of error.
 */
static void _store_inflight_read(const t_transfer *tx) {

    if (ring_buffer_put(g_inflight_rbd, tx)) {
        // @todo: Handle error
    }

}


/*----- Callbacks ----------------------------------------------------*/


/**
 * @brief   Fires once HostDMA FIFO filled with data by DMA controller.
 */
static void _on_host_read_done(t_hostdma_metadata meta) {

    g_tx_in_progress = false;

    if (g_active_tx.callback)
        g_active_tx.callback(g_active_tx.user_ctx, IPC_SUCCESS);

}

/**
 * @brief   Fires once the incoming was written into memory.
 */
static void _on_host_write_done(t_hostdma_metadata meta) {

    // DSP->CPU data request; finalize by firing user callback and cleaning inflight
    if (IPC_OP_RESPONSE == _get_op_type(meta)) {

        t_meta_read *read_meta = (t_meta_read*)&meta;
        if (read_meta->callback)
            read_meta->callback(read_meta->user_ctx, IPC_SUCCESS);
        
        // forget oldest inflight read request
        t_transfer discard;
        ring_buffer_get(g_inflight_rbd, &discard);

    }

    // CPU->DSP data request; respond with requested data
    t_meta_read *read_meta = (t_meta_read*)&meta;
    if (read_meta->op_type == IPC_OP_REQUEST) {

        read_meta->op_type = IPC_OP_RESPONSE;
        _transfer((uint32_t)       read_meta->dest,
                  (const uint32_t*)read_meta->src,
                                   read_meta->count,
                  NULL,
                  NULL,
                  STRUCT_CAST(t_hostdma_metadata, *read_meta));

    }

}

/**
 * @brief   Callback invoked when the driver encountered an error and the active
 *          transaction had been flushed.
 * 
 * @details All error callbacks of active and queued transfers are invoked and
 *          the driver gets reset to idle state.
 *          Driver gets locked to prevent user callbacks from initiating new
 *          transfer requests to prevent corrupting the queue ring buffer.
 * 
 * @param   meta  Metadata of current transfer that failed the driver.
 */
static void _on_error(t_hostdma_metadata meta) {

    g_lock_driver = true;

    // Empty the queue; fire callbacks of remaining transfers
    t_transfer tx;
    while (SUCCESS == ring_buffer_get(g_rbd, &tx)) {
        if (tx.callback)
            tx.callback(tx.user_ctx, IPC_FAILED);
    }

    // DSP->CPU transfer
    if (g_tx_in_progress) {
        g_tx_in_progress = false;
        if (g_active_tx.callback)
            g_active_tx.callback(g_active_tx.user_ctx, IPC_FAILED);
    }

    // In-flight DSP->CPU data read requests
    while (SUCCESS == ring_buffer_get(g_inflight_rbd, &tx)) {
        if (tx.callback)
            tx.callback(tx.user_ctx, IPC_FAILED);
    }

    g_lock_driver = false;

}


/*----- Helpers ------------------------------------------------------*/

/**
 * @note   Currently just casts to a data req struct for clarity purposes
 *         but it could be any metadata struct type in the future.
 */
static t_ipc_op _get_op_type(t_hostdma_metadata meta) {
    t_meta_read *read_meta = (t_meta_read*)&meta;
    return read_meta->op_type;
}


/*----- End of file --------------------------------------------------*/
