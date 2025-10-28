#ifndef LUT_H
#define LUT_H
#include <stdint.h>

extern int32_t g_cutoff_lut[128];
extern int32_t g_resonance_lut[256];

void lut_init();

#endif