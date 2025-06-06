import mido
import numpy as np
import wave
import sys
import time

# --- Configuración ---
# Tamaño del chunk (en número de muestras de 32 bits) para cada mensaje SysEx.
# Un valor más pequeño es más seguro para evitar sobrecargar el búfer del receptor.
SAMPLES_PER_CHUNK = 128 

def seleccionar_puerto_midi():
    """
    Muestra los puertos MIDI de salida disponibles y pide al usuario que seleccione uno.
    Devuelve el nombre del puerto seleccionado.
    """
    try:
        port_names = mido.get_output_names()
    except Exception as e:
        print(f"Error al obtener la lista de puertos MIDI: {e}")
        print("Asegúrate de que los drivers de tu interfaz MIDI (como rtmidi) estén bien instalados.")
        sys.exit(1)


    if not port_names:
        print("Error: No se encontraron puertos MIDI de salida.")
        print("Asegúrate de que tu interfaz MIDI esté conectada y los drivers instalados.")
        sys.exit(1)

    print("\nPor favor, selecciona un puerto MIDI de salida:")
    for i, name in enumerate(port_names):
        print(f"  {i + 1}: {name}")

    while True:
        try:
            selection = input(f"Introduce el número del puerto (1-{len(port_names)}): ")
            port_index = int(selection) - 1
            if 0 <= port_index < len(port_names):
                selected_port_name = port_names[port_index]
                return selected_port_name
            else:
                print("Selección fuera de rango. Inténtalo de nuevo.")
        except ValueError:
            print("Entrada no válida. Por favor, introduce solo un número.")
        except (KeyboardInterrupt, EOFError):
            print("\nSelección cancelada. Saliendo.")
            sys.exit(0)


def leer_wav_mono32(ruta_archivo):
    """
    Lee un archivo WAV y devuelve sus datos como un array de numpy.
    Verifica que el archivo sea mono y de 32 bits.
    """
    print(f"Cargando archivo WAV: {ruta_archivo}")
    try:
        with wave.open(ruta_archivo, 'rb') as wf:
            # Validar formato del WAV
            if wf.getnchannels() != 1:
                raise ValueError("El archivo WAV debe ser mono.")
            if wf.getsampwidth() != 4:
                raise ValueError("El archivo WAV debe tener una profundidad de 32 bits.")
            
            n_frames = wf.getnframes()
            frames = wf.readframes(n_frames)
            
            # Convierte los bytes a un array de enteros de 32 bits
            samples = np.frombuffer(frames, dtype=np.int32)
            print(f"Archivo cargado exitosamente: {len(samples)} muestras.")
            return samples
            
    except FileNotFoundError:
        print(f"Error: No se encontró el archivo en '{ruta_archivo}'")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)

def codificar_muestra_a_sysex(muestra_int32):
    """
    Codifica una única muestra de 32 bits en 5 bytes de 7 bits.
    Esta es la operación inversa a la del código C.
    """
    byte1 = muestra_int32 & 0x7F           # Bits 0-6
    byte2 = (muestra_int32 >> 7) & 0x7F    # Bits 7-13
    byte3 = (muestra_int32 >> 14) & 0x7F   # Bits 14-20
    byte4 = (muestra_int32 >> 21) & 0x7F   # Bits 21-27
    byte5 = (muestra_int32 >> 28) & 0x0F   # Bits 28-31 (solo 4 bits)

    return [byte1, byte2, byte3, byte4, byte5]

def main():
    """
    Función principal del script.
    """
    if len(sys.argv) < 2:
        print(f"Uso: python {sys.argv[0]} <ruta_del_archivo.wav>")
        sys.exit(1)

    # 1. Seleccionar el puerto MIDI de forma interactiva
    midi_port_name = seleccionar_puerto_midi()

    # 2. Leer y procesar el archivo WAV
    ruta_wav = sys.argv[1]
    samples = leer_wav_mono32(ruta_wav)

    # 3. Codificar todas las muestras
    datos_sysex_completos = []
    print("Codificando muestras a formato SysEx...")
    for sample in samples:
        datos_sysex_completos.extend(codificar_muestra_a_sysex(sample))
    
    print(f"Codificación completa. Total de bytes a enviar: {len(datos_sysex_completos)}")

    try:
        # 4. Abrir el puerto MIDI seleccionado y enviar los datos
        with mido.open_output(midi_port_name) as port:
            print(f"\nPuerto MIDI '{midi_port_name}' abierto correctamente.")
            
            bytes_per_chunk = SAMPLES_PER_CHUNK * 5
            
            total_chunks = (len(datos_sysex_completos) + bytes_per_chunk - 1) // bytes_per_chunk
            
            for i in range(0, len(datos_sysex_completos), bytes_per_chunk):
                chunk = datos_sysex_completos[i:i + bytes_per_chunk]
                
                mensaje = mido.Message('sysex', data=chunk)
                
                chunk_num = (i // bytes_per_chunk) + 1
                print(f"Enviando chunk {chunk_num}/{total_chunks}... ({len(chunk)} bytes)")
                port.send(mensaje)
                
                time.sleep(0.2)

            print("\n¡Envío de datos SysEx completado!")

    except (IOError, ValueError) as e:
        print(f"Error al abrir o usar el puerto MIDI: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()