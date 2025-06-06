#include "sysex_manager.h"
#include "freetribe.h"
#include "gui_task.h"
#include "module_interface.h"

void _sysex_callback(char *msg, unsigned long length) {
    // ft_print("sysex callback");
    //  Verificar que el mensaje sea válido
    // gui_post_param("testing: ", 0);
    if (msg == NULL || length < 2) {
        // gui_post_param("novalido: ", 0);
        return; // Mensaje inválido
    }
    // ft_print("lengh");
    // print_param(length);
    // ft_print("otras cosas");

    // gui_post_param("midi ok: ", 0);

    // Procesar el mensaje SysEx
    // we never get sysex start and end messages
    // msg[0] = Manufacturer ID (primer byte)
    // msg[1...] = Manufacturer ID adicional (si es necesario) + datos
    // msg[length-1] = 0xF7 (fin SysEx)

    // Ejemplo de procesamiento:
    //unsigned char manufacturer_id = msg[0];

    // es de uno siempre
    int data_start_index = 1;
    // Calcula cuántos enteros de 32 bits esperamos
    unsigned long num_32bit_values = length / 5;
    // ft_print("num_32bit_values");
    // print_param(num_32bit_values);

    // printf("Esperando %lu valores de 32 bits...\n", num_32bit_values);

    for (unsigned long i = data_start_index; i <= num_32bit_values; ++i) {
        // Calcula el índice de inicio para el grupo actual de 5 bytes
        unsigned long start_idx = (i - 1) * 5;
        // ft_print("start_idx");
        // print_param(start_idx);
        // ft_print("data---------");

        // Extrae los 5 bytes de 7 bits del arreglo de SysEx
        uint8_t byte1 = (uint8_t)msg[start_idx];     // Bits 0-6
        uint8_t byte2 = (uint8_t)msg[start_idx + 1]; // Bits 7-13
        uint8_t byte3 = (uint8_t)msg[start_idx + 2]; // Bits 14-20
        uint8_t byte4 = (uint8_t)msg[start_idx + 3]; // Bits 21-27
        uint8_t byte5 = (uint8_t)msg[start_idx + 4]; // Bits 28-31
        // print_param(byte1);
        // print_param(byte2);
        // print_param(byte3);
        // print_param(byte4);
        // print_param(byte5);

        // Reconstruye el valor original de 32 bits
        int32_t reconstructed_value = 0;
        reconstructed_value |=
            (int32_t)(byte1 & 0x7F); // Asegúrate de que solo los 7 bits estén
                                      // presentes
        reconstructed_value |= (int32_t)(byte2 & 0x7F) << 7;
        reconstructed_value |= (int32_t)(byte3 & 0x7F) << 14;
        reconstructed_value |= (int32_t)(byte4 & 0x7F) << 21;
        reconstructed_value |= (int32_t)(byte5 & 0x0F)
                               << 28; // Solo los 4 bits más significativos

        // ft_print("reconstructed_value");
        ft_set_module_param(0, SAMPLE_LOAD, reconstructed_value);
        ////print_param(reconstructed_value);
    }
}