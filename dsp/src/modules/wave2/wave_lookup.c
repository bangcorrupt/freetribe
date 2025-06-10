#include "wave_lookup.h"
#include "types.h"
#include "custom_aleph_monovoice.h"


/**
 * @brief Versión simplificada usando delta de fase para ajuste de índice
 * @param p Fase actual
 * @param dp Delta de fase (usado para ajuste de índice)
 * @return Muestra directa de 16-bit fractional
 */
fract16 wavetable_lookup_delta(fract32 p, fract32 dp) {
    
    uint32_t phase_norm = (uint32_t)(p);

    uint32_t index = (phase_norm >> (32 - 10)); // this index is ok up to 1024 samples, because it inside the cycle
    uint32_t morph_offset = (uint32_t)(dp );  // morph / 2 testing
    
    index +=  morph_offset;  // Ajustar índice con delta de fase
    /*if (index>=wavtab_index-WAVE_TAB_CYCLE_IN_SAMPLES){
        index = wavtab_index-WAVE_TAB_CYCLE_IN_SAMPLES; // Evitar overflow
    }*/
    
    fract32 sample0 = data_sdram[index]; // FOR SDRAM

    
    // Convertir a 16-bit y retornar
    return (fract16)shr_fr1x32(sample0, 16);
}

