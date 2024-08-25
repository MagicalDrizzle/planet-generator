#!/bin/sh

# Check if an input file was provided
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

input_file="$1"

# Check if the input file exists
if [ ! -f "$input_file" ]; then
  echo "File not found!"
  exit 1
fi

# Read and upscale the ASCII art
while IFS= read -r line; do
  # Double the width of each character
  doubled_width=$(echo "$line" | sed 's/./&&/g')

  # Print the doubled-width line twice to double the height
  echo "$doubled_width"
  echo "$doubled_width"
done < "$input_file"
