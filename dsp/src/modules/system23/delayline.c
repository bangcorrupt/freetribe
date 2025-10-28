#include "fix.h"
#include "types.h"
#include "fract_math.h"
#include <string.h>

#include "delayline.h"

// #define FR32_ZERO_POINT_FIVE 0x80000000

#define MEM_OFFSET (16*1024*1024)
extern void sdram_start;



void delayline_init(t_delayline *const me, int samples, fract32 feedback, fract32 mix) {
    me->buffer = (fract32*)((uint8_t*)&sdram_start+MEM_OFFSET);
    memset(me->buffer, 0, samples*sizeof(fract32));
    me->index = 0;
    me->size = samples;
    me->feedback = feedback;
    me->mix = mix;
    me->one_minus_mix = sub_fr1x32(FR32_MAX, mix);
}

fract32 delayline_process(t_delayline *const me, fract32 input) {

    fract32 delayed_sample = me->buffer[me->index];
    me->buffer[me->index] = input + mult_fr1x32x32(delayed_sample, me->feedback);
    me->index = (me->index + 1) % me->size;

    fract32 output = add_fr1x32(mult_fr1x32x32(input, me->one_minus_mix), mult_fr1x32x32(delayed_sample, me->mix));
    return output;
}

