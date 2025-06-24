#!/usr/bin/env python3

import rtmidi
import re
import sys
import time

def parse_hex_file(filename):
    try:
        with open(filename, 'r') as file:
            content = file.read()
        
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
    sysex_data = []
    
    for hex_val in hex_values:
        value = int(hex_val, 16)
        
        sysex_data.append(value & 0x7F)
        sysex_data.append((value >> 7) & 0x7F)
        sysex_data.append((value >> 14) & 0x7F)
        sysex_data.append((value >> 21) & 0x7F)
        sysex_data.append((value >> 28) & 0x0F)
        
    return sysex_data

def list_midi_ports():
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

def send_sysex_chunked(midiout, sysex_data, manufacturer_id=0x7D, chunk_size=256):
    total_data_bytes = len(sysex_data)
    print(f"\nPreparando para enviar {total_data_bytes} bytes de datos en fragmentos de {chunk_size} bytes...")

    try:
        num_chunks = (total_data_bytes + chunk_size - 1) // chunk_size
        for i in range(num_chunks):
            start_idx = i * chunk_size
            end_idx = min(start_idx + chunk_size, total_data_bytes)
            data_chunk = sysex_data[start_idx:end_idx]
            
            sysex_message = [0xF0, manufacturer_id] + data_chunk + [0xF7]
            
            midiout.send_message(sysex_message)
            
            progress = ((end_idx) / total_data_bytes) * 100
            print(f"Progreso: {progress:.1f}% (Fragmento {i+1}/{num_chunks}, {end_idx}/{total_data_bytes} bytes de datos enviados)")
            
            # Esperar un poco para evitar saturar el puerto MIDI
            time.sleep(0.2)
        
        print("\nEnvío de SysEx completado exitosamente.")
        return True

    except Exception as e:
        print(f"\nError durante el envío de SysEx: {e}")
        return False

def main():
    print("=== Enviador de SysEx ===")
    
    if len(sys.argv) != 2:
        print("Uso: python sysex_sender.py <archivo_hex>")
        print("Ejemplo: python sysex_sender.py datos.txt")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    hex_values = parse_hex_file(filename)
    if not hex_values:
        return
    
    sysex_data = hex_to_sysex_bytes(hex_values)
    print(f"Datos convertidos a {len(sysex_data)} bytes SysEx (5 bytes por cada valor de 32 bits)")
    
    midiout, ports = list_midi_ports()
    if not midiout:
        return
    
    if not select_midi_port(midiout, ports):
        return
    
    try:
        manufacturer_input = input("Ingresa el Manufacturer ID (hex, default 0x7D): ").strip()
        if manufacturer_input:
            try:
                manufacturer_id = int(manufacturer_input, 16) & 0x7F
            except ValueError:
                print("ID inválido, usando 0x7D por defecto")
                manufacturer_id = 0x7D
        else:
            manufacturer_id = 0x7D
        
        chunk_input = input("Tamaño de fragmento de datos en bytes (default 256): ").strip()
        if chunk_input:
            try:
                chunk_size = int(chunk_input)
                if chunk_size < 32 or chunk_size > 4096:
                    print("Tamaño inválido (debe estar entre 32 y 4096), usando 256 por defecto")
                    chunk_size = 256
            except ValueError:
                print("Valor inválido, usando 256 por defecto")
                chunk_size = 256
        else:
            chunk_size = 256
        
        if send_sysex_chunked(midiout, sysex_data, manufacturer_id, chunk_size):
            print("Operación completada.")
        
        time.sleep(0.5)
        
    except KeyboardInterrupt:
        print("\nOperación cancelada por el usuario")
    
    finally:
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