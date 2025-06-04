#!/bin/bash

# Script to convert WAV file to fract32 hexadecimal array values for synthesizer
# Usage: ./wav_to_fract32.sh input.wav [output.txt]

show_help() {
    echo "Usage: $0 <input.wav> [output.txt]"
    echo ""
    echo "Converts a WAV file to fract32 array format for synthesizer"
    echo "fract32: 32-bit fixed-point fractional format from -1.0 to 1.0"
    echo ""
    echo "Parameters:"
    echo "  input.wav     Input WAV file (required)"
    echo "  output.txt    Output file (optional, default: stdout)"
    echo ""
    echo "Example:"
    echo "  $0 audio.wav"
    echo "  $0 audio.wav wavetable_output.txt"
}

# Check parameters
if [ $# -eq 0 ] || [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    show_help
    exit 1
fi

WAV_FILE="$1"
OUTPUT_FILE="$2"

# Check if WAV file exists
if [ ! -f "$WAV_FILE" ]; then
    echo "Error: File '$WAV_FILE' does not exist."
    exit 1
fi

# Check for required tools
for tool in xxd bc; do
    if ! command -v $tool &> /dev/null; then
        echo "Error: Required tool '$tool' is not installed."
        exit 1
    fi
done

echo "Processing file: $WAV_FILE" >&2
echo "Converting to fract32 hexadecimal array..." >&2

# Function to convert little-endian hex to signed decimal
hex_le_to_signed() {
    local hex_le="$1"
    local bytes="$2"
    
    # Validate input
    if [ -z "$hex_le" ] || [ -z "$bytes" ]; then
        echo "0"
        return
    fi
    
    # Remove any whitespace
    hex_le=$(echo "$hex_le" | tr -d ' \n\r')
    
    # Pad to expected length
    while [ ${#hex_le} -lt $((bytes * 2)) ]; do
        hex_le="0${hex_le}"
    done
    
    # Reverse byte order for little-endian to big-endian
    local result=""
    for ((i=$((bytes*2-2)); i>=0; i-=2)); do
        if [ $i -ge 0 ] && [ $((i+2)) -le ${#hex_le} ]; then
            result+="${hex_le:$i:2}"
        fi
    done
    
    if [ -z "$result" ]; then
        echo "0"
        return
    fi
    
    # Convert to decimal
    local decimal
    if [[ "$result" =~ ^[0-9a-fA-F]+$ ]]; then
        decimal=$((0x$result))
    else
        decimal=0
    fi
    
    # Convert to signed based on bit width
    local max_positive=$((1 << (bytes*8-1)))
    
    if [ $decimal -ge $max_positive ]; then
        decimal=$((decimal - (1 << (bytes*8))))
    fi
    
    echo $decimal
}

# Function to convert sample to fract32
sample_to_fract32() {
    local sample="$1"
    local bits_per_sample="$2"
    
    # Validate inputs
    if [ -z "$sample" ] || ! [[ "$sample" =~ ^-?[0-9]+$ ]]; then
        sample=0
    fi
    if [ -z "$bits_per_sample" ]; then
        bits_per_sample=16
    fi
    
    local fract32_value="0"
    
    # Convert PCM to normalized float (-1.0 to 1.0)
    case $bits_per_sample in
        8)
            # 8-bit unsigned 0-255, convert to signed first
            local signed_sample=$((sample - 128))
            fract32_value=$(echo "scale=10; $signed_sample / 128.0" | bc -l 2>/dev/null || echo "0")
            ;;
        16)
            # 16-bit signed -32768 to 32767
            fract32_value=$(echo "scale=10; $sample / 32768.0" | bc -l 2>/dev/null || echo "0")
            ;;
        24)
            # 24-bit signed -8388608 to 8388607
            fract32_value=$(echo "scale=10; $sample / 8388608.0" | bc -l 2>/dev/null || echo "0")
            ;;
        32)
            # 32-bit signed -2147483648 to 2147483647
            fract32_value=$(echo "scale=10; $sample / 2147483648.0" | bc -l 2>/dev/null || echo "0")
            ;;
    esac
    
    # Validate bc result
    if [ -z "$fract32_value" ] || [ "$fract32_value" = "" ]; then
        fract32_value="0"
    fi
    
    # Clamp to range -1.0 to 1.0
    local clamped
    if (( $(echo "$fract32_value > 1.0" | bc -l 2>/dev/null || echo "0") )); then
        clamped="1.0"
    elif (( $(echo "$fract32_value < -1.0" | bc -l 2>/dev/null || echo "0") )); then
        clamped="-1.0"
    else
        clamped="$fract32_value"
    fi
    
    # Convert to fract32 Q31 format
    # -1.0 maps to 0x80000000 (-2147483648)
    # 1.0 maps to 0x7FFFFFFF (2147483647)
    local fract32_int
    if (( $(echo "$clamped >= 0" | bc -l 2>/dev/null || echo "0") )); then
        # Positive values: multiply by 2147483647
        fract32_int=$(echo "scale=0; $clamped * 2147483647 / 1" | bc 2>/dev/null)
    else
        # Negative values: multiply by 2147483648
        fract32_int=$(echo "scale=0; $clamped * 2147483648 / 1" | bc 2>/dev/null)
    fi
    
    # Validate conversion result
    if [ -z "$fract32_int" ] || ! [[ "$fract32_int" =~ ^-?[0-9]+$ ]]; then
        fract32_int=0
    fi
    
    # Convert to unsigned 32-bit for hex representation
    if [ "$fract32_int" -lt 0 ]; then
        fract32_int=$((4294967296 + fract32_int))
    fi
    
    # Convert to 32-bit hex
    printf "0x%08X" "$fract32_int" 2>/dev/null || printf "0x00000000"
}

# Function to analyze WAV header
analyze_wav_header() {
    local file="$1"
    
    # Read first 44 bytes of WAV header
    local header_hex=$(xxd -l 44 -p "$file" 2>/dev/null | tr -d '\n')
    
    if [ ${#header_hex} -lt 88 ]; then
        echo "Error: Invalid WAV file header" >&2
        exit 1
    fi
    
    # Extract format info (little-endian) - corrected byte positions
    local audio_format_hex="${header_hex:42:2}${header_hex:40:2}"
    local audio_format
    if [[ "$audio_format_hex" =~ ^[0-9a-fA-F]+$ ]] && [ ${#audio_format_hex} -eq 4 ]; then
        audio_format=$((0x$audio_format_hex))
    else
        audio_format=1
    fi
    
    local channels_hex="${header_hex:46:2}${header_hex:44:2}"
    local channels
    if [[ "$channels_hex" =~ ^[0-9a-fA-F]+$ ]] && [ ${#channels_hex} -eq 4 ]; then
        channels=$((0x$channels_hex))
    else
        channels=1
    fi
    
    local sample_rate_hex="${header_hex:54:2}${header_hex:52:2}${header_hex:50:2}${header_hex:48:2}"
    local sample_rate
    if [[ "$sample_rate_hex" =~ ^[0-9a-fA-F]+$ ]] && [ ${#sample_rate_hex} -eq 8 ]; then
        sample_rate=$((0x$sample_rate_hex))
    else
        sample_rate=44100
    fi
    
    local bits_per_sample_hex="${header_hex:70:2}${header_hex:68:2}"
    local bits_per_sample
    if [[ "$bits_per_sample_hex" =~ ^[0-9a-fA-F]+$ ]] && [ ${#bits_per_sample_hex} -eq 4 ]; then
        bits_per_sample=$((0x$bits_per_sample_hex))
    else
        bits_per_sample=16
    fi
    
    # Show format information
    echo "WAV Format Analysis:" >&2
    echo "Audio Format: $audio_format (1=PCM, 3=IEEE Float)" >&2
    echo "Channels: $channels" >&2
    echo "Sample Rate: $sample_rate Hz" >&2
    echo "Bits per Sample: $bits_per_sample" >&2
    echo "Converting to: fract32 Q31 fixed-point" >&2
    echo "" >&2
    
    echo "$bits_per_sample $audio_format $channels"
}

# Function to find data chunk start
find_data_start() {
    local file="$1"
    
    # Use a more robust method to find the data chunk
    local data_offset=$(xxd "$file" | grep -m1 "data" | cut -d: -f1)
    
    if [ -n "$data_offset" ]; then
        # Convert hex offset to decimal and add 8 bytes (4 for "data" + 4 for size)
        local decimal_offset=$((0x$data_offset))
        echo $((decimal_offset + 8))
    else
        # Fallback: try to find data chunk by searching for the hex pattern
        local hex_dump=$(xxd -p "$file" | tr -d '\n')
        local data_pos=$(echo "$hex_dump" | grep -o -b "64617461" | head -1 | cut -d: -f1)
        
        if [ -n "$data_pos" ]; then
            echo $((data_pos / 2 + 8))
        else
            echo 44  # Standard WAV header size fallback
        fi
    fi
}

# Function to process audio data
process_audio_data() {
    local file="$1"
    local data_start="$2"
    local bits_per_sample="$3"
    local channels="$4"
    
    local bytes_per_sample=$((bits_per_sample / 8))
    local bytes_per_frame=$((bytes_per_sample * channels))
    local count=0
    local values_per_line=8
    
    echo "{"
    
    # Create temporary file for processing
    local temp_file=$(mktemp)
    
    # Extract raw audio data and convert to hex lines
    dd if="$file" bs=1 skip=$data_start 2>/dev/null | xxd -p -c $bytes_per_frame > "$temp_file"
    
    if [ ! -s "$temp_file" ]; then
        echo "Error: No audio data extracted" >&2
        rm -f "$temp_file"
        echo "}"
        return
    fi
    
    local total_lines=$(wc -l < "$temp_file")
    echo "Debug: Found $total_lines lines of audio data" >&2
    
    # Process each line
    while IFS= read -r line || [ -n "$line" ]; do
        if [ -n "$line" ] && [ ${#line} -gt 0 ]; then
            # Extract first channel sample (first bytes_per_sample*2 hex chars)
            local sample_hex="${line:0:$((bytes_per_sample * 2))}"
            
            if [ ${#sample_hex} -eq $((bytes_per_sample * 2)) ]; then
                # Convert to signed decimal
                local sample_value=$(hex_le_to_signed "$sample_hex" $bytes_per_sample)
                
                # Convert to fract32
                local fract32_hex=$(sample_to_fract32 "$sample_value" "$bits_per_sample")
                
                # Format output
                if [ $count -eq 0 ]; then
                    printf "    %s" "$fract32_hex"
                else
                    printf ", %s" "$fract32_hex"
                fi
                
                count=$((count + 1))
                
                # New line every values_per_line values
                if [ $((count % values_per_line)) -eq 0 ]; then
                    echo ""
                    printf "    "
                fi
                
            fi
        fi
    done < "$temp_file"
    
    # Close array properly
    if [ $((count % values_per_line)) -ne 0 ]; then
        echo ""
    fi
    echo "},"
    
    rm -f "$temp_file"
    
    echo "" >&2
    echo "Total samples processed: $count" >&2
    echo "Format: Q31 fixed-point fract32" >&2
}

# Main processing function
process_file() {
    # Analyze WAV format
    local format_info=$(analyze_wav_header "$WAV_FILE")
    
    # Parse the format info with better error handling
    local bits_per_sample=$(echo "$format_info" | cut -d' ' -f1)
    local audio_format=$(echo "$format_info" | cut -d' ' -f2) 
    local channels=$(echo "$format_info" | cut -d' ' -f3)
    
    # Validate extracted values and show debug info
    if [ -z "$bits_per_sample" ] || ! [[ "$bits_per_sample" =~ ^[0-9]+$ ]]; then
        echo "Warning: Invalid bits_per_sample, using default 16" >&2
        bits_per_sample=16
    fi
    if [ -z "$audio_format" ] || ! [[ "$audio_format" =~ ^[0-9]+$ ]]; then
        echo "Warning: Invalid audio_format, using default 1" >&2
        audio_format=1
    fi
    if [ -z "$channels" ] || ! [[ "$channels" =~ ^[0-9]+$ ]]; then
        echo "Warning: Invalid channels, using default 1" >&2
        channels=1
    fi
    
    echo "Extracted values - Bits: $bits_per_sample, Format: $audio_format, Channels: $channels" >&2
    
    # Find audio data start
    local data_start=$(find_data_start "$WAV_FILE")
    
    # Get file size info
    local total_size=$(stat -f%z "$WAV_FILE" 2>/dev/null || stat -c%s "$WAV_FILE" 2>/dev/null || echo "0")
    local data_size=$((total_size - data_start))
    
    echo "File size: $total_size bytes" >&2
    echo "Audio data starts at: $data_start bytes" >&2
    echo "Audio data size: $data_size bytes" >&2
    
    if [ -n "$channels" ] && [ "$channels" -gt 1 ] 2>/dev/null; then
        echo "Warning: Multi-channel audio detected. Using first channel only." >&2
    fi
    
    echo "" >&2
    
    # Process audio data
    process_audio_data "$WAV_FILE" "$data_start" "$bits_per_sample" "$channels"
}

# Execute conversion
if [ -n "$OUTPUT_FILE" ]; then
    echo "Saving result to: $OUTPUT_FILE" >&2
    process_file > "$OUTPUT_FILE"
    echo "Conversion completed!" >&2
else
    process_file
fi