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
 * @file    dev_dsp_ipc.c.
 *
 * @brief   IPC device layer to transmit, request, and receive data
 *          to and from the DSP. Transmissions are served using a queue
 *          system.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>

#include "macros.h"
#include "ring_buffer.h"

#include "per_emifa.h"

#include "dev_dsp_ipc.h"

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
    uint32_t          dsp_address;
    const uint32_t   *buffer;
    uint16_t          count;
    void            (*callback)(void *, t_ipc_status);
    void             *user_ctx;
    t_emifa_metadata  metadata;
} t_transfer;

// (Bidirectional) metadata for data read request
typedef struct {
    t_ipc_op          op_type;
    uint16_t          count;
    uint32_t          src;
    uint32_t         *dest;
    void            (*callback)(void *, t_ipc_status);
    void             *user_ctx;
} t_meta_read;

// (Bidirectional) metadata for standalone transfer
typedef struct {
    t_ipc_op          op_type;
    uint16_t          _meta0_hi;
    uint32_t          _meta1;
    uint32_t          _meta2;
    void            (*callback)(void *, t_ipc_status);
    void             *user_ctx;
} t_meta_tx;

_Static_assert(sizeof(t_emifa_metadata) == sizeof(t_meta_read));
_Static_assert(sizeof(t_emifa_metadata) == sizeof(t_meta_tx  ));

/*----- Static variable definitions ----------------------------------*/

static rbd_t        g_rbd;
static t_transfer   g_active_tx;
static t_transfer   g_queue[QUEUE_SIZE];

// Read requests must be bookkept to prevent losing callbacks in case of error
static rbd_t        g_inflight_rbd;
static t_transfer   g_inflight_reads[QUEUE_SIZE];

static bool         g_lock_driver;
static bool         g_tx_in_progress;

/*----- Static function prototypes -----------------------------------*/

static t_ipc_status _transfer(uint32_t         dsp_address,
                              const uint32_t  *buffer,
                              uint16_t         count,
                              void           (*callback)(void *, t_ipc_status),
                              void            *user_ctx,
                              t_emifa_metadata metadata);

static void         _serve_queued_transfer();
static t_ipc_status _enqueue_transfer(const t_transfer *tx);
static bool         _try_immediate_send(const t_transfer *tx);
static void         _store_inflight_read(const t_transfer *tx);

// Callback handling
static void _on_idle();
static void _on_hostwr_done();
static void _on_hostrd_done(t_emifa_metadata meta);
static void _on_error(t_emifa_metadata meta);

static t_ipc_op _get_op_type(t_emifa_metadata meta);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialize high speed communication channel with the DSP.
 */
void dev_dsp_ipc_init() {

    rb_attr_t attr = {
        sizeof(g_queue[0]),
        ARRAY_SIZE(g_queue),
        g_queue
    };
    ring_buffer_init(&g_rbd, &attr);

    rb_attr_t inflight_attr = {
        sizeof(g_inflight_reads[0]),
        ARRAY_SIZE(g_inflight_reads),
        g_inflight_reads
    };
    ring_buffer_init(&g_inflight_rbd, &inflight_attr);

    per_emifa_init(&_on_idle, &_on_hostwr_done, &_on_hostrd_done, &_on_error);

}

/**
 * @brief   Request to transfer data to the DSP memory.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   buffer        Pointer to the buffer of 32-bit values to write
 * @param   count         Number of 32-bit values to write.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 */
t_ipc_status dev_dsp_ipc_transfer(uint32_t dsp_address,
                                  const uint32_t *buffer,
                                  uint16_t count,
                                  void (*callback)(void *, t_ipc_status),
                                  void *user_ctx) {

    t_meta_tx meta = {
        .op_type  = IPC_OP_TRANSFER,
        .callback = callback,
        .user_ctx = user_ctx
    };

    return _transfer(dsp_address,
                     buffer,
                     count,
                     callback,
                     user_ctx,
                     STRUCT_CAST(t_emifa_metadata, meta));
}

/**
 * @brief   Request a buffer of data from the DSP's memory.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to read from.
 * @param   destination   Pointer to buffer of 32-bit values to write read memory from.
 * @param   count         Number of 32-bit values to request.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 */
t_ipc_status dev_dsp_ipc_read(uint32_t dsp_address,
                              uint32_t *destination,
                              uint16_t count,
                              void (*callback)(void *, t_ipc_status),
                              void *user_ctx) {

    // Request an empty host write transfer with only metadata in the
    // header that instructs the DSP side IPC layer to send back data.
    t_meta_read meta = {
        .op_type    = IPC_OP_REQUEST,
        .count      = count,
        .src        = dsp_address,
        .dest       = destination,
        .callback   = callback,
        .user_ctx   = user_ctx
    };

    return _transfer(HOST_TO_DSP_HEADER_BASE,
                     NULL,
                     0,
                     NULL,
                     NULL,
                     STRUCT_CAST(t_emifa_metadata, meta));

}

/*----- Static function implementations ------------------------------*/


/**
 * @brief   Internal helper to start or queue a host write transfer.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   buffer        Pointer to the buffer of 32-bit values to write
 * @param   count         Number of 32-bit values to write.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 * @param   metadata      Gets packed into the header.
 */
static t_ipc_status _transfer(uint32_t         dsp_address,
                              const uint32_t  *buffer,
                              uint16_t         count,
                              void           (*callback)(void *, t_ipc_status),
                              void            *user_ctx,
                              t_emifa_metadata metadata) {
    
    if (g_lock_driver)
        return IPC_DRIVER_LOCKED;

    // Max transfer length is 65535 16-bit words
    if (count >= 32768)
        return IPC_INVALID_ARGUMENT;
    
    t_transfer tx = {
        .dsp_address = dsp_address,
        .buffer      = buffer,
        .count       = count,
        .callback    = callback,
        .user_ctx    = user_ctx,
        .metadata    = metadata
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

    g_tx_in_progress  = true;
    g_active_tx = *tx;

    const uint16_t words16 = 2 * tx->count; // HostDMA expects 16-bit words

    t_emifa_status st = per_emifa_transfer(tx->dsp_address,
                                           (const uint16_t*)tx->buffer,
                                           words16,
                                           tx->metadata);
    
    if (EMIFA_SUCCESS != st) {
        g_tx_in_progress = false;
        return false;
    }
    
    if (IPC_OP_REQUEST == _get_op_type(tx->metadata)) {
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
 * @brief   Try to start and consume next queued transfer.
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
        DEBUG_LOG("Inflight FIFO full!");
    }

}

/*----- Callback handling --------------------------------------------*/

/**
 * @brief   When EMIFA state machine is truly idle it calls this function so
 *          that we can perform a queued up transfer.
 */
static void _on_idle() {
    
    _serve_queued_transfer();

}

/**
 * @brief   Called when a host write (CPU->DSP) completes or fails.
 */
static void _on_hostwr_done() {

    DEBUG_LOG("_on_hostwr_done");

    g_tx_in_progress = false;

    if (g_active_tx.callback) {
        g_active_tx.callback(g_active_tx.user_ctx, IPC_SUCCESS);
    }

}

/**
 * @brief   Called once incoming data (DSP->CPU) has been written to RAM.
 * 
 * @param   meta  Metadata associated with the transfer.
 */
static void _on_hostrd_done(t_emifa_metadata meta) {

    DEBUG_LOG("_on_hostrd_done");

    // CPU->DSP data request; finalize by firing user callback and cleaning inflight
    if (IPC_OP_RESPONSE == _get_op_type(meta)) {

        t_meta_read *read_meta = (t_meta_read*)&meta;
        if (read_meta->callback)
            read_meta->callback(read_meta->user_ctx, IPC_SUCCESS);
        
        // forget oldest inflight read request
        t_transfer discard;
        ring_buffer_get(g_inflight_rbd, &discard);

    }

    // DSP->CPU data request; respond with requested data
    t_meta_read *read_meta = (t_meta_read*)&meta;
    if (read_meta->op_type == IPC_OP_REQUEST) {

        read_meta->op_type = IPC_OP_RESPONSE;
        _transfer((uint32_t)       read_meta->dest,
                  (const uint32_t*)read_meta->src,
                                   read_meta->count,
                  NULL,
                  NULL,
                  STRUCT_CAST(t_emifa_metadata, *read_meta));

    }

    _serve_queued_transfer();

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
static void _on_error(t_emifa_metadata meta) {

    DEBUG_LOG("_on_error");

    g_lock_driver = false;

    // Empty the queue; fire callbacks of remaining transfers
    t_transfer tx;
    while (SUCCESS == ring_buffer_get(g_rbd, &tx)) {
        if (tx.callback)
            tx.callback(tx.user_ctx, IPC_FAILED);
    }

    // CPU->DSP transfer
    if (g_tx_in_progress) {
        g_tx_in_progress = false;
        if (g_active_tx.callback)
            g_active_tx.callback(g_active_tx.user_ctx, IPC_FAILED);
    }

    // In-flight CPU->DSP data read requests
    while (SUCCESS == ring_buffer_get(g_inflight_rbd, &tx)) {
        if (tx.callback)
            tx.callback(tx.user_ctx, IPC_FAILED);
    }

    g_lock_driver = true;

}


/*----- Helpers ------------------------------------------------------*/

/**
 * @note   Currently just casts to a data req struct for clarity purposes
 *         but it could be any metadata struct type in the future.
 */
static t_ipc_op _get_op_type(t_emifa_metadata meta) {
    t_meta_read *read_meta = (t_meta_read*)&meta;
    return read_meta->op_type;
}


/*----- End of file --------------------------------------------------*/
