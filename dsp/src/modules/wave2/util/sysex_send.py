#!/usr/bin/env python3
"""
Script para enviar datos SysEx desde un archivo usando rtmidi
"""

import rtmidi
import re
import sys
import time

def parse_hex_file(filename):
    """
    Lee un archivo con formato { 0x00003FFF, 0x001127BA, ... }
    y extrae los valores hexadecimales
    """
    try:
        with open(filename, 'r') as file:
            content = file.read()
        
        # Buscar todos los valores hexadecimales en el formato 0x########
        hex_pattern = r'0x[0-9A-Fa-f]+'
        hex_values = re.findall(hex_pattern, content)
        
        if not hex_values:
            print("No se encontraron valores hexadecimales en el archivo")
            return None
        
        print(f"Se encontraron {len(hex_values)} valores hexadecimales")
        return hex_values
        
    except FileNotFoundError:
        print(f"Error: No se pudo encontrar el archivo '{filename}'")
        return None
    except Exception as e:
        print(f"Error al leer el archivo: {e}")
        return None

def hex_to_sysex_bytes(hex_values):
    """
    Convierte los valores hexadecimales (32 bits) a bytes para SysEx,
    asegurando un tamaño fijo de 5 bytes por cada valor.
    """
    sysex_data = []
    
    for hex_val in hex_values:
        value = int(hex_val, 16)
        
        # Codificar el valor de 32 bits en 5 bytes de 7 bits
        # Byte 1: bits 0-6
        sysex_data.append(value & 0x7F)
        # Byte 2: bits 7-13
        sysex_data.append((value >> 7) & 0x7F)
        # Byte 3: bits 14-20
        sysex_data.append((value >> 14) & 0x7F)
        # Byte 4: bits 21-27
        sysex_data.append((value >> 21) & 0x7F)
        # Byte 5: bits 28-31 (los 4 bits más significativos)
        sysex_data.append((value >> 28) & 0x0F) # Solo 4 bits
        
    return sysex_data

def list_midi_ports():
    """
    Lista todos los puertos MIDI disponibles
    """
    midiout = rtmidi.MidiOut()
    ports = midiout.get_ports()
    
    if not ports:
        print("No se encontraron puertos MIDI disponibles")
        return None, None
    
    print("\nPuertos MIDI disponibles:")
    for i, port in enumerate(ports):
        print(f"{i}: {port}")
    
    return midiout, ports

def select_midi_port(midiout, ports):
    """
    Permite al usuario seleccionar un puerto MIDI
    """
    while True:
        try:
            port_num = int(input(f"\nSelecciona el puerto MIDI (0-{len(ports)-1}): "))
            if 0 <= port_num < len(ports):
                midiout.open_port(port_num)
                print(f"Puerto abierto: {ports[port_num]}")
                return True
            else:
                print("Número de puerto inválido")
        except ValueError:
            print("Por favor, ingresa un número válido")
        except KeyboardInterrupt:
            print("\nOperación cancelada")
            return False

def send_sysex(midiout, sysex_data, manufacturer_id=0x7D, max_chunk_size=1024):
    """
    Envía los datos como un solo mensaje SysEx, pero fragmentando el envío
    para evitar problemas de buffer
    """
    # Crear el mensaje SysEx completo: 0xF0 [manufacturer_id] [data...] 0xF7
    full_sysex_message = [0xF0, manufacturer_id] + sysex_data + [0xF7]
    total_bytes = len(full_sysex_message)
    
    print(f"\nPreparando mensaje SysEx de {total_bytes} bytes...")
    print(f"Primeros 16 bytes: {[hex(b) for b in full_sysex_message[:16]]}")
    
    # Intentar enviar el mensaje completo primero
    try:
        print("Intentando enviar mensaje completo...")
        midiout.send_message(full_sysex_message)
        print("Mensaje SysEx enviado exitosamente")
        return True
        
    except Exception as e:
        print(f"Error al enviar mensaje completo: {e}")
        print("Intentando envío fragmentado...")
        
        # Si falla, intentar envío fragmentado con pequeñas pausas
        try:
            # Dividir en chunks pequeños para el envío, pero manteniendo la estructura SysEx
            chunk_size = min(max_chunk_size, total_bytes // 10)  # Dividir en al menos 10 partes
            if chunk_size < 256:
                chunk_size = 256
                
            print(f"Enviando en fragmentos de {chunk_size} bytes con pausas...")
            
            for i in range(0, total_bytes, chunk_size):
                end_idx = min(i + chunk_size, total_bytes)
                chunk = full_sysex_message[i:end_idx]
                
                # Enviar fragmento
                midiout.send_message(chunk)
                progress = ((end_idx) / total_bytes) * 100
                print(f"Progreso: {progress:.1f}% ({end_idx}/{total_bytes} bytes)")
                
                # Pausa más larga entre fragmentos
                time.sleep(0.05)
            
            print("Mensaje SysEx enviado exitosamente (modo fragmentado)")
            return True
            
        except Exception as e2:
            print(f"Error en envío fragmentado: {e2}")
            
            # Último intento: envío byte por byte (muy lento pero seguro)
            try:
                print("Último intento: envío byte por byte...")
                for i, byte_val in enumerate(full_sysex_message):
                    midiout.send_message([byte_val])
                    if i % 100 == 0:
                        progress = ((i + 1) / total_bytes) * 100
                        print(f"Progreso: {progress:.1f}% ({i+1}/{total_bytes} bytes)")
                    time.sleep(0.001)  # Pausa muy pequeña
                
                print("Mensaje SysEx enviado exitosamente (modo byte por byte)")
                return True
                
            except Exception as e3:
                print(f"Error en envío byte por byte: {e3}")
                return False

def main():
    """
    Función principal
    """
    print("=== Enviador de SysEx ===")
    
    # Verificar argumentos de línea de comandos
    if len(sys.argv) != 2:
        print("Uso: python sysex_sender.py <archivo_hex>")
        print("Ejemplo: python sysex_sender.py datos.txt")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    # Leer y parsear el archivo
    hex_values = parse_hex_file(filename)
    if not hex_values:
        return
    
    # Convertir a bytes SysEx
    sysex_data = hex_to_sysex_bytes(hex_values)
    print(f"Datos convertidos a {len(sysex_data)} bytes SysEx (5 bytes por cada valor de 32 bits)")
    
    # Listar puertos MIDI
    midiout, ports = list_midi_ports()
    if not midiout:
        return
    
    # Seleccionar puerto
    if not select_midi_port(midiout, ports):
        return
    
    try:
        # Preguntar por el Manufacturer ID (opcional)
        manufacturer_input = input("Ingresa el Manufacturer ID (hex, default 0x7D): ").strip()
        if manufacturer_input:
            try:
                manufacturer_id = int(manufacturer_input, 16) & 0x7F  # Solo 7 bits válidos
            except ValueError:
                print("ID inválido, usando 0x7D por defecto")
                manufacturer_id = 0x7D
        else:
            manufacturer_id = 0x7D
        
        # Preguntar por el tamaño máximo de fragmento
        chunk_input = input("Tamaño máximo de fragmento en bytes (default 1024): ").strip()
        if chunk_input:
            try:
                max_chunk_size = int(chunk_input)
                if max_chunk_size < 256 or max_chunk_size > 8192:
                    print("Tamaño inválido, usando 1024 bytes por defecto")
                    max_chunk_size = 1024
            except ValueError:
                print("Valor inválido, usando 1024 bytes por defecto")
                max_chunk_size = 1024
        else:
            max_chunk_size = 1024
        
        # Enviar datos
        if send_sysex(midiout, sysex_data, manufacturer_id, max_chunk_size):
            print("Operación completada exitosamente")
        
        # Pausa para asegurar que todos los mensajes se envíen
        time.sleep(0.5)
        
    except KeyboardInterrupt:
        print("\nOperación cancelada por el usuario")
    
    finally:
        # Cerrar puerto MIDI
        if midiout:
            midiout.close_port()
            print("Puerto MIDI cerrado")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nScript terminado por el usuario")
    except Exception as e:
        print(f"Error inesperado: {e}")
        sys.exit(1)