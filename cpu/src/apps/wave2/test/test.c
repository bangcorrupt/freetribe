
#include "test.h"
#include "freetribe.h"
#include "types.h"
#include "params.h"

fract32 wavtab[WAVE_TAB_SIZE] =
#include "sylenthva.data"

void sendTestData() {
    unsigned long i;
    for (i = 0; i < WAVE_TAB_SIZE; i++) {
        ft_set_module_param(0, SAMPLE_LOAD, wavtab[i]);
    }
}