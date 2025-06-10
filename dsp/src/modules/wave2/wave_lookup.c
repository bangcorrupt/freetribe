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
    
    // Convertir fase ajustada a índice
    //uint32_t phase_norm = (uint32_t)(p + FR32_MAX);
    uint32_t phase_norm = (uint32_t)(p);
    //uint32_t index = (phase_norm >> (32 - 10)) & (WAVE_TAB_CYCLE_IN_SAMPLES - 1); // esto lo limita a 1024 muestras creo

     // La parte entera de la fase nos da el primer índice (idx0)
    uint32_t index = (phase_norm >> (32 - 10)); // this index is ok up to 1024 samples, because it inside the cycle


    // La parte fraccionaria son los bits restantes, que usaremos para la interpolación
    //fract32 frac = (fract32)(phase_norm << 10);

       // Convertir fract32 a entero
    
    //int32_t valor_entero = fix16_to_int(dp);  // fix16 a entero // para morph amoount con pitch, valores de 0 a 255
    //uint32_t morph_offset = fix16_to_int(dp);  // fix16 a entero // cutoff tambien usa fix16, no se que valores aun
    //int32_t morph_offset = (int32_t)(dp); // testing
    //int32_t morph_offset = (int32_t)(dp & 0x7FFFFFFF );
    int32_t morph_offset = (int32_t)(dp );
    
    // map to DATA SIZE
    //valor_entero = MAP_255_TO_WAVE_TAB(valor_entero);  // Mapear a rango de tabla de ondas // FOR STATIC MEMORY
    //valor_entero = MAP_255_TO_WAVE_TAB_SDRAM(valor_entero);   // FOR SDRAM
    index +=  morph_offset;  // Ajustar índice con delta de fase
    /*if (index>=wavtab_index-WAVE_TAB_CYCLE_IN_SAMPLES){
        index = wavtab_index-WAVE_TAB_CYCLE_IN_SAMPLES; // Evitar overflow
    }*/
    //uint32_t index2 = (index + 1);
    
    // Leer directamente de la tabla
    //fract32 sample = wavtab[index]; // FOR STATIC MEMORY
    fract32 sample0 = data_sdram[index]; // FOR SDRAM

    // todo interpolate  maybe
/*        fract32 sample1 = data_sdram[index2];

    // --- ESTA ES LA SECCIÓN ACTUALIZADA ---
    // Interpolar usando las funciones de tu librería de punto fijo.
    // Fórmula: sample0 + (sample1 - sample0) * frac

    // Calcular la diferencia: (sample1 - sample0)
    fract32 diff = sub_fr1x32(sample1, sample0);

    // Multiplicar la diferencia por la fracción: diff * frac
    fract32 scaled_diff = mult_fr1x32x32(diff, frac);

    // Sumar el resultado a la primera muestra: sample0 + scaled_diff
    fract32 result = add_fr1x32(sample0, scaled_diff);
*/
    
    // Convertir a 16-bit y retornar
    return (fract16)shr_fr1x32(sample0, 16);
}

