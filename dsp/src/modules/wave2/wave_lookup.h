#ifndef WAVE_LOOKUP_H
#define WAVE_LOOKUP_H
#include "custom_aleph_monovoice.h"

fract16 wavetable_lookup_simple(fract32 p, fract32 dp, uint8_t wave_shape) ;
fract16 wavetable_lookup(fract32 p, fract32 dp, uint8_t wave_shape) ;
fract16 wavetable_morph(fract32 p, fract32 dp, uint8_t wave_shape1, uint8_t wave_shape2, fract32 morph_amount) ;

#endif
