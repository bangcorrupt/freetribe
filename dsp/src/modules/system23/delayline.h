#ifndef DELAYLINE_H
#define DELAYLINE_H

#include "types.h"

typedef struct {
    fract32 *buffer;
    int index;
    int size;
    fract32 feedback;
    fract32 mix;
    fract32 one_minus_mix;
} t_delayline;

void delayline_init(t_delayline *const, int samples, fract32 feedback, fract32 mix);
fract32 delayline_process(t_delayline *const, fract32 input);

// void delayline_free();

#endif