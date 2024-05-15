/* Original work by Francois Best, modified by bangcorrupt 2023. */

/*
 *  file       MIDI.cpp
 *  Project    Arduino MIDI Library
 *  brief      MIDI Library for the Arduino
 *  author     Francois Best
 *  date       24/02/11
 *  license    MIT - Copyright (c) 2015 Francois Best
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * @file    sysex_codec.c
 *
 * @brief   Encode and decode MIDI system exclusinve data bytes.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "sysex_codec.h"

/*
 * @brief Encode System Exclusive messages.
 *          SysEx messages are encoded to guarantee transmission of data bytes
 *          higher than 127 without breaking the MIDI protocol. Use this static
 *          method to convert the data you want to send.
 *
 * @param in_data The data to encode.
 *
 * @param out_sysex The output buffer where to store the encoded message.
 *
 * @param length The length of the input buffer.
 *
 * @return The length of the encoded output buffer.
 *
 * Code inspired from Ruin & Wesen's SysEx encoder/decoder -
 * http://ruinwesen.com
 *
 */
uint32_t sysex_encode(const uint8_t *in_data, uint8_t *out_sysex,
                      uint32_t in_length) {

    uint32_t out_length = 0; // Num bytes in output array.
    uint8_t count = 0;       // Num 7bytes in a block.

    out_sysex[0] = 0;

    for (uint32_t i = 0; i < in_length; ++i) {
        const uint8_t data = in_data[i];
        const uint8_t msb = data >> 7;
        const uint8_t body = data & 0x7f;

        out_sysex[0] |= (msb << count);
        out_sysex[1 + count] = body;

        if (count++ == 6) {
            out_sysex += 8;
            out_length += 8;
            out_sysex[0] = 0;
            count = 0;
        }
    }
    return out_length + count + (count != 0 ? 1 : 0);
}

/*
 * @brief Decode System Exclusive messages.
 *          SysEx messages are encoded to guarantee transmission of data bytes
 *          higher than 127 without breaking the MIDI protocol. Use this static
 *          method to reassemble your received message.
 *
 * @param in_sysex The sysex data received from MIDI in.
 *
 * @param out_data The output buffer where to store the decrypted message.
 *
 * @param in_length The length of the input buffer.
 *
 * @return The length of the output buffer.
 *
 * Code inspired from Ruin & Wesen's SysEx encoder/decoder -
 * http://ruinwesen.com
 *
 */
uint32_t sysex_decode(const uint8_t *in_sysex, uint8_t *out_data,
                      uint32_t in_length) {
    unsigned count = 0;
    uint8_t msb_storage = 0;
    uint8_t byte_index = 0;

    for (uint32_t i = 0; i < in_length; ++i) {
        if ((i % 8) == 0) {
            msb_storage = in_sysex[i];
            byte_index = 6;
        } else {
            const uint8_t body = in_sysex[i];
            const uint8_t shift = 6 - byte_index;
            const uint8_t msb = ((msb_storage >> shift) & 1) << 7;
            byte_index--;
            out_data[count++] = msb | body;
        }
    }
    return count;
}
