
#include "lut.h"
#include "param_scale.h"

int32_t g_cutoff_lut[128];
int32_t g_resonance_lut[256];

void lut_init() {
    // float_to_fract32(1.0 - (value / 255.0f))

    for (int i = 0; i < 128; i++) {
        g_cutoff_lut[i] = float_to_fix16(cv_to_filter_freq_oversample(note_to_cv(i)));
    }

    for (int i = 0; i <= 255; i++) {
        g_resonance_lut[i] = 0x7fffffff - (i * (1 << 23));
    }

}