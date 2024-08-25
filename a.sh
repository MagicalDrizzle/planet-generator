#!/bin/sh

# Function to convert RGB to HEX
rgb_to_hex() {
  r=$1
  g=$2
  b=$3

  # Check if the input values are within the valid range
  if [ "$r" -lt 0 ] || [ "$r" -gt 255 ] || [ "$g" -lt 0 ] || [ "$g" -gt 255 ] || [ "$b" -lt 0 ] || [ "$b" -gt 255 ]; then
    echo "Error: RGB values must be between 0 and 255."
    return 1
  fi

  # Convert RGB to HEX
  printf "#%02X%02X%02X\n" "$r" "$g" "$b"
}

# Check if a file argument is provided
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

input_file="$1"

# Check if the file exists
if [ ! -f "$input_file" ]; then
  echo "Error: File not found!"
  exit 1
fi

# Read the file line by line
while IFS=' ' read -r r g b; do
  # Convert and print the color code for each line
  rgb_to_hex "$r" "$g" "$b"
done < "$input_file"
