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
 * @file    template_module.c
 *
 * @brief   Template for DSP module source files.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "fract_math.h"
#include "types.h"

#include "module.h"

#include "utils.h"

#include "aleph.h"
#include "aleph_monovoice.h"

#include "delayline.h"

/*----- Macros -------------------------------------------------------*/

#define MEMPOOL_SIZE (0x1000)

#define PARAM_NOTE_FREQ 2
#define PARAM_GATE_TICKS 3
#define PARAM_NEXT_NOTE_FREQ 4
#define PARAM_TICKS_TIL_NEXT 5
#define PARAM_CUTOFF 10
#define PARAM_RESONANCE 11

/*----- Typedefs -----------------------------------------------------*/

typedef struct {
    Aleph_MonoVoice voice[1];
    bool gate_on;
    uint32_t gate_ticks;
    fract32 note_freq;
    uint32_t ticks_til_next;
    fract32 next_note_freq;

    float cutoff;
    float resonance;
} t_module;

/*----- Static variable definitions ----------------------------------*/

__attribute__((section(".l1.data.a")))
__attribute__((aligned(32))) static char g_mempool[MEMPOOL_SIZE];
// __attribute__((aligned(32))) static char g_sample[1024];


// static float TUNING[256] = { // @TODO: move this to param change buffers
//     0.500000, 0.503906, 0.507812, 0.511719, 0.515625, 0.519531, 0.523438, 0.527344,
//     0.531250, 0.535156, 0.539062, 0.542969, 0.546875, 0.550781, 0.554688, 0.558594,
//     0.562500, 0.566406, 0.570312, 0.574219, 0.578125, 0.582031, 0.585938, 0.589844,
//     0.593750, 0.597656, 0.601562, 0.605469, 0.609375, 0.613281, 0.617188, 0.621094,
//     0.625000, 0.628906, 0.632812, 0.636719, 0.640625, 0.644531, 0.648438, 0.652344,
//     0.656250, 0.660156, 0.664062, 0.667969, 0.671875, 0.675781, 0.679688, 0.683594,
//     0.687500, 0.691406, 0.695312, 0.699219, 0.703125, 0.707031, 0.710938, 0.714844,
//     0.718750, 0.722656, 0.726562, 0.730469, 0.734375, 0.738281, 0.742188, 0.746094,
//     0.750000, 0.753906, 0.757812, 0.761719, 0.765625, 0.769531, 0.773438, 0.777344,
//     0.781250, 0.785156, 0.789062, 0.792969, 0.796875, 0.800781, 0.804688, 0.808594,
//     0.812500, 0.816406, 0.820312, 0.824219, 0.828125, 0.832031, 0.835938, 0.839844,
//     0.843750, 0.847656, 0.851562, 0.855469, 0.859375, 0.863281, 0.867188, 0.871094,
//     0.875000, 0.878906, 0.882812, 0.886719, 0.890625, 0.894531, 0.898438, 0.902344,
//     0.906250, 0.910156, 0.914062, 0.917969, 0.921875, 0.925781, 0.929688, 0.933594,
//     0.937500, 0.941406, 0.945312, 0.949219, 0.953125, 0.957031, 0.960938, 0.964844,
//     0.968750, 0.972656, 0.976562, 0.980469, 0.984375, 0.988281, 0.992188, 0.996094,
//     1.000000, 1.007874, 1.015748, 1.023622, 1.031496, 1.039370, 1.047244, 1.055118,
//     1.062992, 1.070866, 1.078740, 1.086614, 1.094488, 1.102362, 1.110236, 1.118110,
//     1.125984, 1.133858, 1.141732, 1.149606, 1.157480, 1.165354, 1.173228, 1.181102,
//     1.188976, 1.196850, 1.204724, 1.212598, 1.220472, 1.228346, 1.236220, 1.244094,
//     1.251969, 1.259843, 1.267717, 1.275591, 1.283465, 1.291339, 1.299213, 1.307087,
//     1.314961, 1.322835, 1.330709, 1.338583, 1.346457, 1.354331, 1.362205, 1.370079,
//     1.377953, 1.385827, 1.393701, 1.401575, 1.409449, 1.417323, 1.425197, 1.433071,
//     1.440945, 1.448819, 1.456693, 1.464567, 1.472441, 1.480315, 1.488189, 1.496063,
//     1.503937, 1.511811, 1.519685, 1.527559, 1.535433, 1.543307, 1.551181, 1.559055,
//     1.566929, 1.574803, 1.582677, 1.590551, 1.598425, 1.606299, 1.614173, 1.622047,
//     1.629921, 1.637795, 1.645669, 1.653543, 1.661417, 1.669291, 1.677165, 1.685039,
//     1.692913, 1.700787, 1.708661, 1.716535, 1.724409, 1.732283, 1.740157, 1.748031,
//     1.755906, 1.763780, 1.771654, 1.779528, 1.787402, 1.795276, 1.803150, 1.811024,
//     1.818898, 1.826772, 1.834646, 1.842520, 1.850394, 1.858268, 1.866142, 1.874016,
//     1.881890, 1.889764, 1.897638, 1.905512, 1.913386, 1.921260, 1.929134, 1.937008,
//     1.944882, 1.952756, 1.960630, 1.968504, 1.976378, 1.984252, 1.992126, 2.000000
// };

// static float t_tb303pattern g_pattern_notes[255]; // full module transmit buffer

static t_Aleph g_aleph;
static t_module g_module;
static t_delayline g_delayline;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

// static uint32_t xorshift32(uint32_t *state) {
//     uint32_t x = *state;
//     x ^= x << 13;
//     x ^= x >> 17;
//     x ^= x << 5;
//     return *state = x;
// }

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise module.
 */
void module_init(void) {

    Aleph_init(&g_aleph, SAMPLERATE, g_mempool, MEMPOOL_SIZE, NULL);
    Aleph_MonoVoice_init(&g_module.voice[0], &g_aleph);
    Aleph_MonoVoice_set_amp(&g_module.voice[0], FR32_MAX);
    Aleph_MonoVoice_set_amp_slew(&g_module.voice[0], SLEW_1MS);
    Aleph_MonoVoice_set_shape(&g_module.voice[0], WAVEFORM_SHAPE_SAW);
    Aleph_MonoVoice_set_freq_offset(&g_module.voice[0], FIX16_ONE);
    Aleph_MonoVoice_set_cutoff(&g_module.voice[0], 0x326f6abb);
    Aleph_MonoVoice_set_res(&g_module.voice[0], FR32_MAX);

    delayline_init(&g_delayline, 19200, 0x43000000, 0x33333333); // ((60000/150)/1000)*48000
    
}

/**
 * @brief   Process audio.
 *
 * @param[in]   in  Pointer to input buffer.
 * @param[out]  out Pointer to input buffer.
 */
void module_process(fract32 *in, fract32 *out) {

    if (g_module.gate_ticks > 0)
        g_module.gate_ticks--;
    
    if (g_module.gate_on) {
        // Wait til next note has to play when gate hold is over
        if (g_module.gate_ticks == 0) {
            g_module.gate_on = false;
            g_module.gate_ticks = g_module.ticks_til_next;
        }
    } else {
        // Start playing cached note if the DSP is earlier than the CPU can feed us notes
        if (g_module.gate_ticks == 0) {
            g_module.gate_on = true;
            g_module.note_freq = g_module.next_note_freq;
        }
    }


    Aleph_MonoVoice_set_amp(&g_module.voice[0], g_module.gate_on ? FR32_MAX : 0x00000000);
    Aleph_MonoVoice_set_freq(&g_module.voice[0], g_module.note_freq);
    fract32 sample = Aleph_MonoVoice_next(&g_module.voice[0]);

    // fract32 sample;
    // if (g_module.gate_on) {
    //     Aleph_MonoVoice_set_freq(&g_module.voice[0], g_module.note_freq);
    //     sample = Aleph_MonoVoice_next(&g_module.voice[0]);
    // } else {
    //     sample = 0x00000000;
    // }
    
    // Apply delay
    sample = shr_fr1x32(sample, 1); // temporarily lower amplitude to prevent clipping
    sample = delayline_process(&g_delayline, sample);


    out[0] = sample;
    out[1] = sample;
}

/**
 * @brief   Set parameter.
 *
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   value      Value of parameter.
 */
void module_set_param(uint16_t param_index, int32_t value) {

    switch (param_index) {

    case PARAM_NOTE_FREQ:
        g_module.note_freq = value;
        break;
    case PARAM_GATE_TICKS:
        g_module.gate_ticks = value;
        g_module.gate_on = true; // reset to make sure
        break;
    case PARAM_NEXT_NOTE_FREQ:
        g_module.next_note_freq = value;
        break;
    case PARAM_TICKS_TIL_NEXT:
        g_module.ticks_til_next = value;
        break;

    case PARAM_CUTOFF:
        g_module.cutoff = value;
        Aleph_MonoVoice_set_cutoff(&g_module.voice[0], value);
        break;
    case PARAM_RESONANCE:
        g_module.resonance = value;
        Aleph_MonoVoice_set_res(&g_module.voice[0], value);
        break;
    
    }
}

/**
 * @brief   Get parameter.
 *
 * @param[in]   param_index Index of parameter to get.
 *
 * @return      value       Value of parameter.
 */
int32_t module_get_param(uint16_t param_index) {

    int32_t value = 0;

    switch (param_index) {
    default:
        break;
    }

    return value;
}

/**
 * @brief   Get number of parameters.
 *
 * @return  Number of parameters
 */
uint32_t module_get_param_count(void) { return 0; }

/**
 * @brief   Get name of parameter at index.
 *
 * @param[in]   param_index     Index pf parameter.
 * @param[out]  text            Buffer to store string.
 *                              Must provide 'MAX_PARAM_NAME_LENGTH'
 *                              bytes of storage.
 */
void module_get_param_name(uint16_t param_index, char *text) {

    switch (param_index) {

    default:
        copy_string(text, "Unknown", MAX_PARAM_NAME_LENGTH);
        break;
    }
}


/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
