#include "wave_lookup.h"
#include "types.h"
#include "custom_aleph_monovoice.h"

#define FRACT32_MAX     ((fract32)0x7fffffff)    /* max value of a fract32 */

/**
 * @brief Versión simplificada usando delta de fase para ajuste de índice
 * @param p Fase actual
 * @param dp Delta de fase (usado para ajuste de índice)
 * @return Muestra directa de 16-bit fractional
 */
fract16 wavetable_lookup_delta(fract32 phase, fract32 dp) {
    
    uint32_t phase_norm = (uint32_t)(phase);

    int index = (phase_norm >> (32 - 10)); // this index is ok up to 1024 samples, because it inside the cycle
    //int morph_offset = (int)(dp) ; // x 10 or x even x 100 works
    // int morph_offset = (int)(dp) ; // x 10 or x even x 100 works
    
    int morph_offset = fract32_smul(1000000, dp); // scale the delta phase to 1000 for better resolution
    index += morph_offset;
    
    fract32 sample0 = data_sdram[index]; 

    
    // Convertir a 16-bit y retornar
    return (fract16)shr_fr1x32(sample0, 16);
}

