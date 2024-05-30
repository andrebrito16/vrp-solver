#!/bin/bash

# Set the input directory, output file, vehicle capacity, and max cities per route
INPUT_DIR="../inputs"
OUTPUT_FILE="./results.txt"
VEHICLE_CAPACITY=20
MAX_CITIES_PER_ROUTE=3

# Create the output directory if it doesn't exist
mkdir -p "$(dirname "$OUTPUT_FILE")"

# Clear the output file if it exists
> "$OUTPUT_FILE"

# Iterate over each input file in the input directory
for input_file in "$INPUT_DIR"/graph_*_nodes.txt; do
    # Extract the base name of the input file
    base_name=$(basename "$input_file" .txt)
    
    # Append the header for this input file's results to the output file
    echo "Processing $input_file with vehicle capacity $VEHICLE_CAPACITY and max cities per route $MAX_CITIES_PER_ROUTE" >> "$OUTPUT_FILE"
    
    # Run the binary with the input file, vehicle capacity, and max cities per route, and append the output to the output file
    ./local_search_omp "$input_file" "$VEHICLE_CAPACITY" "$MAX_CITIES_PER_ROUTE" >> "$OUTPUT_FILE"
    
    # Append a separator between results of different input files
    echo "-------------------------------------------------------------" >> "$OUTPUT_FILE"
    
    echo "Processed $input_file and appended output to $OUTPUT_FILE"
done
