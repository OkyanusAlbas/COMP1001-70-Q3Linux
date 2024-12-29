#!/bin/bash

# Check if exactly 3 arguments are passed
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <input_images_path> <output_images_path> <output_images2_path>"
    exit 1
fi

input_images_path=$1
output_images_path=$2
output_images2_path=$3

# Compile the C program
gcc -o Q3B Q3B.c -lm

# Run the program with the passed paths
./Q3B "$input_images_path" "$output_images_path" "$output_images2_path"
