/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
                  (at your option) any later version.

     Freetribe is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty
        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
          See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.

                       Copyright bangcorrupt 2024

----------------------------------------------------------------------*/

/**
 * @file    template_app.c
 *
 * @brief   Template for CPU application source files.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"
#include "gui_task.h"
#include "midi_fsm.h"

#ifndef NULL
#define NULL ((void *)0)
#endif


#define BUTTON_EXIT 0x0d

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static bool g_shutdown = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _button_callback(uint8_t index, bool state);



/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/
void print_param(char value){
        char val_string[4];
    itoa(value, val_string, 10);
    ft_print(val_string);

}


/*----- Extern function implementations ------------------------------*/


static void _sysex_callback(char *msg, unsigned long length) ;

/**
 * @brief   Initialise application.
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {
        // Initialise GUI.
    //gui_task();

    

midi_register_sysex_handler(_sysex_callback);
ft_register_panel_callback(BUTTON_EVENT, _button_callback);


    t_status status = ERROR;

    status = SUCCESS;

    return status;
}
/**
 * @brief   Callback triggered by panel button events.
 *
 * @param[in]   index   Index of button.
 * @param[in]   state   State of button.
 */
void _button_callback(uint8_t index, bool state) {

    switch (index) {

    case BUTTON_EXIT:
        if (state == 1) {
            g_shutdown = true;
        }
        break;

    default:
        break;
    }
}
/**
 * @brief   Run application.
 */
void app_run(void) {

        if (g_shutdown) {

        ft_shutdown();

        // Should never reach here.
    }
}


// Función de callback modificada
static void _sysex_callback(char *msg, unsigned long length) {
    // Verificar que el mensaje sea válido
    if (msg == NULL || length < 2) {
        gui_post_param("novalido: ", 0);
        return; // Mensaje inválido
    }
    ft_print("lengh");
    print_param(length);
    ft_print("otras cosas");

    gui_post_param("midi ok: ", 0);
    
    // Procesar el mensaje SysEx
    // we never get sysex start and end messages
    // msg[0] = Manufacturer ID (primer byte)
    // msg[1...] = Manufacturer ID adicional (si es necesario) + datos
    // msg[length-1] = 0xF7 (fin SysEx)
    
    // Ejemplo de procesamiento:
    unsigned char manufacturer_id = msg[0];
    
    // es de uno siempre
    int data_start_index = 1;
    // Calcula cuántos enteros de 32 bits esperamos
    unsigned long num_32bit_values = length / 5;
        ft_print("num_32bit_values");
    print_param(num_32bit_values);

    //printf("Esperando %lu valores de 32 bits...\n", num_32bit_values);

    for (unsigned long i = data_start_index; i <= num_32bit_values; ++i) {
        // Calcula el índice de inicio para el grupo actual de 5 bytes
        unsigned long start_idx = (i-1) * 5;
        ft_print("start_idx");
    print_param(start_idx);
    ft_print("data---------");

        // Extrae los 5 bytes de 7 bits del arreglo de SysEx
        uint8_t byte1 = (uint8_t)msg[start_idx];     // Bits 0-6
        uint8_t byte2 = (uint8_t)msg[start_idx + 1]; // Bits 7-13
        uint8_t byte3 = (uint8_t)msg[start_idx + 2]; // Bits 14-20
        uint8_t byte4 = (uint8_t)msg[start_idx + 3]; // Bits 21-27
        uint8_t byte5 = (uint8_t)msg[start_idx + 4]; // Bits 28-31
        print_param(byte1);
        print_param(byte2);
        print_param(byte3);
        print_param(byte4);
        print_param(byte5);
        

        // Reconstruye el valor original de 32 bits
        uint32_t reconstructed_value = 0;
        reconstructed_value |= (uint32_t)(byte1 & 0x7F);        // Asegúrate de que solo los 7 bits estén presentes
        reconstructed_value |= (uint32_t)(byte2 & 0x7F) << 7;
        reconstructed_value |= (uint32_t)(byte3 & 0x7F) << 14;
        reconstructed_value |= (uint32_t)(byte4 & 0x7F) << 21;
        reconstructed_value |= (uint32_t)(byte5 & 0x0F) << 28;  // Solo los 4 bits más significativos

                ft_print("reconstructed_value");
    //print_param(reconstructed_value);

    }
    
    // Procesar los datos específicos del dispositivo
   /* for (unsigned long i = data_start_index; i < length ; i++) {
        // Procesar cada byte de datos
        // msg[i] contiene los datos específicos del mensaje SysEx
        gui_post_param("sysex: ", msg[i]);
        print_param(msg[i]);
    }*/
}


// Asumiendo que esta es la función callback para SysEx
static void _sysex_callback_2(char *msg, unsigned long length) {
    //printf("Mensaje SysEx recibido. Longitud total: %lu bytes\n", length);

    // Los datos SysEx comienzan DESPUÉS del F0 (status) y el Manufacturer ID.
    // Y terminan ANTES del F7 (end of exclusive).
    // Si tu 'msg' puntero ya te da los datos entre F0 y F7 (excluyendo F0 y F7),
    // entonces 'length' sería la longitud de los datos útiles.
    // Si 'msg' incluye F0 y F7, necesitarías ajustar 'length' y el inicio del puntero.

    // Para este ejemplo, asumiremos que 'msg' apunta directamente a los datos
    // después del Manufacturer ID y antes del 0xF7.
    // Y 'length' es la cantidad de bytes de datos útiles.

    // Cada valor de 32 bits se codifica en 5 bytes MIDI (7 bits cada uno).
    // Por lo tanto, la longitud total de datos útiles DEBE ser un múltiplo de 5.
    if (length % 5 != 0) {
        /*printf("Advertencia: La longitud de los datos (%lu) no es un múltiplo de 5. "
               "Esto puede indicar un mensaje incompleto o corrupto.\n", length);*/
        // Podrías manejar este error de forma más robusta, por ejemplo, ignorando
        // los bytes finales que no forman un grupo completo.
    }

    // Calcula cuántos enteros de 32 bits esperamos
    unsigned long num_32bit_values = length / 5;
    //printf("Esperando %lu valores de 32 bits...\n", num_32bit_values);

    for (unsigned long i = 0; i < num_32bit_values; ++i) {
        // Calcula el índice de inicio para el grupo actual de 5 bytes
        unsigned long start_idx = i * 5;

        // Extrae los 5 bytes de 7 bits del arreglo de SysEx
        uint8_t byte1 = (uint8_t)msg[start_idx];     // Bits 0-6
        uint8_t byte2 = (uint8_t)msg[start_idx + 1]; // Bits 7-13
        uint8_t byte3 = (uint8_t)msg[start_idx + 2]; // Bits 14-20
        uint8_t byte4 = (uint8_t)msg[start_idx + 3]; // Bits 21-27
        uint8_t byte5 = (uint8_t)msg[start_idx + 4]; // Bits 28-31

        // Reconstruye el valor original de 32 bits
        uint32_t reconstructed_value = 0;
        reconstructed_value |= (uint32_t)(byte1 & 0x7F);        // Asegúrate de que solo los 7 bits estén presentes
        reconstructed_value |= (uint32_t)(byte2 & 0x7F) << 7;
        reconstructed_value |= (uint32_t)(byte3 & 0x7F) << 14;
        reconstructed_value |= (uint32_t)(byte4 & 0x7F) << 21;
        reconstructed_value |= (uint32_t)(byte5 & 0x0F) << 28;  // Solo los 4 bits más significativos

        //printf("  Valor %lu: 0x%08lX\n", i, (unsigned long)reconstructed_value);
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
