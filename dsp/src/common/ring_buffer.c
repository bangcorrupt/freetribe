/**
 * \file ring_buffer.c
 * \author Chris Karaplis
 * \brief Ring buffer implementation
 *
 * Copyright (c) 2015, simplyembedded.org
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Modified by bangcorrupt 2023 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ft_error.h"
#include "ring_buffer.h"

#define RING_BUFFER_MAX 64

struct ring_buffer {
    size_t s_elem;
    size_t n_elem;
    uint8_t *buf;
    volatile size_t head;
    volatile size_t tail;
};

static struct ring_buffer _rb[RING_BUFFER_MAX];

static int _ring_buffer_full(struct ring_buffer *rb);
static int _ring_buffer_empty(struct ring_buffer *rb);

/**
 * \brief Initialize a ring buffer
 * \param[out] rb - pointer to a ring buffer descriptor
 * \param[in] attr - ring buffer attributes
 * \return 0 on success, -1 otherwise
 *
 * The attributes must contain a ring buffer which is sized
 * to an even power of 2. This should be reflected by the
 * attribute n_elem.
 */
int ring_buffer_init(rbd_t *rbd, rb_attr_t *attr) {
    static int idx = 0;
    int result = RING_BUFFER_INIT_ERROR;

    if ((idx < RING_BUFFER_MAX) && (rbd != NULL) && (attr != NULL)) {
        if ((attr->buffer != NULL) && (attr->s_elem > 0)) {
            /* Check that the size of the ring buffer is a power of
             * 2 */
            if (((attr->n_elem - 1) & attr->n_elem) == 0) {
                /* Initialize the ring buffer internal variables
                 */
                _rb[idx].head = 0;
                _rb[idx].tail = 0;
                _rb[idx].buf = attr->buffer;
                _rb[idx].s_elem = attr->s_elem;
                _rb[idx].n_elem = attr->n_elem;

                *rbd = idx++;
                result = SUCCESS;
            }
        }
    }

    return result;
}

/**
 * \brief Add an element to the ring buffer
 * \param[in] rbd - the ring buffer descriptor
 * \param[in] data - the data to add
 * \return 0 on success, -1 otherwise
 */
int ring_buffer_put(rbd_t rbd, const void *data) {
    int result = RING_BUFFER_PUT_ERROR;

    if ((rbd < RING_BUFFER_MAX) && (_ring_buffer_full(&_rb[rbd]) == 0)) {
        const size_t offset =
            (_rb[rbd].head & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(&(_rb[rbd].buf[offset]), data, _rb[rbd].s_elem);
        _rb[rbd].head++;

        result = SUCCESS;
    }

    return result;
}

int ring_buffer_put_force(rbd_t rbd, const void *data) {

    int result = RING_BUFFER_PUT_ERROR;

    if (rbd < RING_BUFFER_MAX) {
        // Clear oldest element if ring buffer full.
        if (_ring_buffer_full(&_rb[rbd])) {
            _rb[rbd].tail++;
            result = WARNING;
        }

        const size_t offset =
            (_rb[rbd].head & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(&(_rb[rbd].buf[offset]), data, _rb[rbd].s_elem);
        _rb[rbd].head++;

        result = result != WARNING ? SUCCESS : WARNING;
    }

    return result;
}

/**
 * \brief Get (and remove) an element from the ring buffer
 * \param[in] rb - the ring buffer descriptor
 * \param[in] data - pointer to store the data
 * \return 0 on success, -1 otherwise
 */
int ring_buffer_get(rbd_t rbd, void *data) {
    int result = RING_BUFFER_GET_ERROR;

    if ((rbd < RING_BUFFER_MAX) && (_ring_buffer_empty(&_rb[rbd]) == 0)) {
        const size_t offset =
            (_rb[rbd].tail & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(data, &(_rb[rbd].buf[offset]), _rb[rbd].s_elem);
        _rb[rbd].tail++;

        result = SUCCESS;
    }
    return result;
}

// TODO: Return bool?
static int _ring_buffer_full(struct ring_buffer *rb) {
    return ((rb->head - rb->tail) == rb->n_elem) ? 1 : 0;
}

static int _ring_buffer_empty(struct ring_buffer *rb) {
    return ((rb->head - rb->tail) == 0U) ? 1 : 0;
}

// TODO: Refactor and delete.
int rb_data_ready(rbd_t rbd) { return !_ring_buffer_empty(&_rb[rbd]); }

int rb_buffer_full(rbd_t rbd) { return _ring_buffer_full(&_rb[rbd]); }
