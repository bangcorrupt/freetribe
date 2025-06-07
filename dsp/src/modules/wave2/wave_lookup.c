#include "wave_lookup.h"
#include "types.h"
#include "custom_aleph_monovoice.h"

/**
 * @brief Lee una muestra de la tabla de ondas usando interpolación lineal
 * @param p Fase actual (32-bit fractional, rango completo)
 * @param dp Delta de fase (incremento por muestra) - usado para antialiasing futuro
 * @param wave_shape Índice de la forma de onda (0 a WAVE_SHAPE_NUM-1)
 * @return Muestra interpolada de 16-bit fractional
 */
fract16 wavetable_lookup(fract32 p, fract32 dp, uint8_t wave_shape) {
    
    // Trabajar directamente con p sin normalización adicional
    // p ya está en el rango correcto de fract32
    
    // Usar los 10 bits más significativos para el índice (sin contar el bit de signo)
    uint32_t phase_uint = (uint32_t)p;
    uint32_t index = (phase_uint >> (32 - 10)) & (WAVE_TAB_CYCLE_IN_SAMPLES - 1);
    
    // Calcular índice siguiente (con wrap-around)
    uint32_t next_index = (index + 1) & (WAVE_TAB_CYCLE_IN_SAMPLES - 1);
    
    // Obtener fracción para interpolación
    // Usar los bits restantes después del índice
    uint32_t frac_bits = phase_uint << 10; // Desplazar hacia la izquierda para obtener fracción
    fract32 fraction = (fract32)frac_bits; // Ya está en formato fract32
    
    // Leer muestras de la tabla
    fract32 sample1 = wavtab[index];
    fract32 sample2 = wavtab[next_index];
    
    // Interpolación lineal: sample1 + (sample2 - sample1) * fraction
    fract32 diff = sub_fr1x32(sample2, sample1);
    fract32 interpolated = add_fr1x32(sample1, mult_fr1x32x32(diff, fraction));
    
    // Convertir a 16-bit de manera más precisa
    return (fract16)(interpolated >> 16);
}
/**
 * @brief Versión simplificada sin interpolación (más rápida, menos calidad)
 * @param p Fase actual
 * @param wave_shape Índice de la forma de onda
 * @return Muestra directa de 16-bit fractional
 */
fract16 wavetable_lookup_simple(fract32 p, uint8_t wave_shape) {
    
    // Convertir fase a índice
    uint32_t phase_norm = (uint32_t)(p + FR32_MAX);
    uint32_t index = (phase_norm >> (32 - 10)) & (WAVE_TAB_CYCLE_IN_SAMPLES - 1);
    
    // Leer directamente de la tabla
    fract32 sample = wavtab[index];
    
    // Convertir a 16-bit y retornar
    return (fract16)shr_fr1x32(sample, 16);
}

uint32_t mapear_fract32_int(fract32 p) {
    // Adaptando tu fórmula para rango 0-262144 (que es 2^18)
    uint32_t phase_norm = (uint32_t)(p + FR32_MAX);
    uint32_t index = phase_norm >> (32 - 18);  // Tomar 18 bits superiores
    return index;
}

/**
 * @brief Versión simplificada usando delta de fase para ajuste de índice
 * @param p Fase actual
 * @param dp Delta de fase (usado para ajuste de índice)
 * @return Muestra directa de 16-bit fractional
 */
fract16 wavetable_lookup_delta(fract32 p, fract32 dp) {
    
    // Convertir fase ajustada a índice
    uint32_t phase_norm = (uint32_t)(p + FR32_MAX);
    //uint32_t index = (phase_norm >> (32 - 10)) & (WAVE_TAB_CYCLE_IN_SAMPLES - 1); // esto lo limita a 1024 muestras creo

     // La parte entera de la fase nos da el primer índice (idx0)
    uint32_t index = (phase_norm >> (32 - 10));

    // La parte fraccionaria son los bits restantes, que usaremos para la interpolación
    fract32 frac = (fract32)(phase_norm << 10);

       // Convertir fract32 a entero
    
    //int32_t valor_entero = fix16_to_int(dp);  // fix16 a entero // para morph amoount con pitch, valores de 0 a 255
    uint32_t morph_offset = fix16_to_int(dp);  // fix16 a entero // cutoff tambien usa fix16, no se que valores aun
    
    // map to DATA SIZE
    //valor_entero = MAP_255_TO_WAVE_TAB(valor_entero);  // Mapear a rango de tabla de ondas // FOR STATIC MEMORY
    //valor_entero = MAP_255_TO_WAVE_TAB_SDRAM(valor_entero);   // FOR SDRAM
    index +=  morph_offset;  // Ajustar índice con delta de fase
    if (index>=wavtab_index-WAVE_TAB_CYCLE_IN_SAMPLES){
        index = wavtab_index-WAVE_TAB_CYCLE_IN_SAMPLES; // Evitar overflow
    }
    uint32_t index2 = (index + 1);
    
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

/**
 * @brief Versión con morfing entre dos formas de onda
 * @param p Fase actual
 * @param dp Delta de fase
 * @param wave_shape1 Primera forma de onda
 * @param wave_shape2 Segunda forma de onda
 * @param morph_amount Cantidad de morfing fract32 (0 = solo shape1, FR32_MAX = solo shape2)
 * @return Muestra mezclada de 16-bit fractional
 */
fract16 _wavetable_morph(fract32 p, fract32 dp, uint8_t wave_shape1, uint8_t wave_shape2, fract32 morph_amount) {
    // Obtener muestras de ambas formas de onda
    fract16 sample1 = wavetable_lookup(p, dp, wave_shape1);
    fract16 sample2 = wavetable_lookup(p, dp, wave_shape2);
    
    // Limitar morph_amount al rango válido [0, FR32_MAX]
    if (morph_amount < 0) {
        morph_amount = 0;
    } else if (morph_amount > FR32_MAX) {
        morph_amount = FR32_MAX;
    }
    
    // Método 1: Usando interpolación lineal con aritmética fract32
    // Convertir samples a fract32 para mayor precisión en el cálculo
    fract32 sample1_32 = (fract32)sample1 << 16;  // Extender fract16 a fract32
    fract32 sample2_32 = (fract32)sample2 << 16;  // Extender fract16 a fract32
    
    // Interpolación: result = sample1 + (sample2 - sample1) * morph_amount
    fract32 diff_32 = sample2_32 - sample1_32;
    fract32 morphed_32 = sample1_32 + mult_fr1x32(diff_32, morph_amount);
    
    // Convertir de vuelta a fract16
    fract16 morphed = (fract16)(morphed_32 >> 16);
    
    return morphed;
}

fract16 wavetable_morph(fract32 p, fract32 dp, uint8_t wave_shape1, uint8_t wave_shape2, fract32 morph_amount) {
    // Obtener muestras de ambas formas de onda
    fract16 sample1 = wavetable_lookup(p, dp, wave_shape1);
    fract16 sample2 = wavetable_lookup(p, dp, wave_shape2);
    
    // DEBUG: Imprimir valores para diagnosticar
    // printf("sample1: %d, sample2: %d, morph: %ld\n", sample1, sample2, morph_amount);
    
    // Verificar que tenemos muestras diferentes
    if (sample1 == sample2) {
        return sample1; // Si son iguales, no hay nada que morphear
    }
    
    // Limitar morph_amount al rango válido [0, FR32_MAX]
    if (morph_amount <= 0) {
        return sample1;  // 100% forma de onda 1
    }
    if (morph_amount >= FR32_MAX) {
        return sample2;  // 100% forma de onda 2
    }
    
    // VERSIÓN SIMPLE: Interpolación lineal usando solo enteros
    // Convertir morph_amount de [0, FR32_MAX] a [0, 32767] para simplificar
    int32_t morph_scaled = morph_amount >> 16; // Tomar los 16 bits superiores
    
    // Interpolación: result = sample1 + ((sample2 - sample1) * morph_scaled) / 32767
    int32_t diff = (int32_t)sample2 - (int32_t)sample1;
    int32_t morphed_temp = (int32_t)sample1 + ((diff * morph_scaled) / 32767);
    
    // Saturar el resultado al rango fract16
    if (morphed_temp > 32767) morphed_temp = 32767;
    if (morphed_temp < -32768) morphed_temp = -32768;
    
    return (fract16)morphed_temp;
}

