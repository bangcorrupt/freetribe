

#include "wave_generator.h"

// Genera onda cuadrada suave usando square_polyblep (todo en fract32)
void generate_soft_square(int wave_index) {
    if (wave_index >= WAVE_SHAPE_NUM) return;
    
    fract32 duty_cycle = float_to_fract32(0.5f); // duty cycle fijo al 50%
    fract32 softness = float_to_fract32(0.05f);  // suavizado fijo
    
    // Parámetros para square_polyblep
    fract32 dp = float_to_fract32(1.0f);
    
    int i;
    for (i = 0; i < WAVE_TAB_SIZE; i++) {
        // Calcular fase en formato fract32
        fract32 phase_fract = (fract32)((long long)i * 0x7FFFFFFF / WAVE_TAB_SIZE);
        
        // Generar muestra usando square_polyblep
        fract16 raw_square_sample = square_polyblep(phase_fract, dp);
        
        // Convertir fract16 a fract32
        fract32 raw_square = (fract32)raw_square_sample << 16;
        
        // Suavizar los extremos usando aritmética de punto fijo
        fract32 soft_factor = 0x7FFFFFFF; // 1.0 en fract32
        
        // Calcular distancia a los bordes para el suavizado
        fract32 edge_dist_1, edge_dist_2;
        
        if (phase_fract < duty_cycle) {
            // Distancia al borde izquierdo o al duty cycle
            edge_dist_1 = phase_fract;
            edge_dist_2 = sub_fr1x32(duty_cycle, phase_fract);
        } else {
            // Distancia al duty cycle o al borde derecho
            edge_dist_1 = sub_fr1x32(phase_fract, duty_cycle);
            edge_dist_2 = sub_fr1x32(0x7FFFFFFF, phase_fract);
        }
        
        // Tomar la menor distancia
        fract32 min_edge_dist = (edge_dist_1 < edge_dist_2) ? edge_dist_1 : edge_dist_2;
        
        // Aplicar suavizado si estamos cerca de un borde
        if (min_edge_dist < softness) {
            // Calcular factor de suavizado (aproximación de tanh)
            fract32 edge_ratio = mult_fr1x32x32(min_edge_dist, 
                                              0x7FFFFFFF / (softness >> 16));
            
            // Aproximación simple de tanh para suavizado
            soft_factor = mult_fr1x32x32(edge_ratio, edge_ratio);
            soft_factor = mult_fr1x32x32(soft_factor, float_to_fract32(0.8f));
        }
        
        // Aplicar suavizado
        fract32 sample = mult_fr1x32x32(raw_square, soft_factor);
        
        // Aplicar headroom (0.95f)
        sample = mult_fr1x32x32(sample, float_to_fract32(0.95f));
        
        // Almacenar en wavtab global
        wavtab[wave_index][i] = sample;
    }
}
// Genera diente de sierra suave usando saw_polyblep (todo en fract32)
void generate_soft_sawtooth(int wave_index) {
    if (wave_index >= WAVE_SHAPE_NUM) return;

    int direction = 1; // 1 para ascendente, -1 para descendente
    fract32 softness = float_to_fract32(0.05f); // valor fijo de suavizado
    
    // Parámetros para saw_polyblep
    fract32 dp = float_to_fract32(1.0f / WAVE_TAB_SIZE); // incremento de fase
    
    int i;
    for (i = 0; i < WAVE_TAB_SIZE; i++) {
        // Calcular fase en formato fract32
        fract32 phase_fract = (fract32)((long long)i * 0x7FFFFFFF / WAVE_TAB_SIZE);
        
        // Generar muestra usando saw_polyblep
        fract16 raw_saw_sample = saw_polyblep(phase_fract, dp);
        
        // Convertir fract16 a fract32 y aplicar dirección
        fract32 raw_saw = mult_fr1x32x32((fract32)raw_saw_sample << 16, 
                                        direction > 0 ? 0x7FFFFFFF : 0x80000001);
        
        // Suavizar los extremos usando aritmética de punto fijo
        fract32 soft_factor = 0x7FFFFFFF; // 1.0 en fract32
        
        if (phase_fract < softness) {
            // soft_factor = phase / softness
            soft_factor = mult_fr1x32x32(phase_fract, 
                                       0x7FFFFFFF / (softness >> 16)); // aproximación
        } else if (phase_fract > (0x7FFFFFFF - softness)) {
            // soft_factor = (1.0 - phase) / softness  
            fract32 inv_phase = sub_fr1x32(0x7FFFFFFF, phase_fract);
            soft_factor = mult_fr1x32x32(inv_phase, 
                                       0x7FFFFFFF / (softness >> 16)); // aproximación
        }
        
        // Aplicar suavizado
        fract32 sample = mult_fr1x32x32(raw_saw, soft_factor);
        
        // Aplicar headroom (0.95f)
        sample = mult_fr1x32x32(sample, float_to_fract32(0.95f));
        
        // Almacenar en wavtab global
        wavtab[wave_index][i] = sample;
    }
}
