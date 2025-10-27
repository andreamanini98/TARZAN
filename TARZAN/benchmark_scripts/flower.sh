#!/bin/bash

# Check if correct number of arguments provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <executable_path> <root_directory>"
    echo "Example: $0 ./flower /path/to/benchmarks"
    exit 1
fi

EXECUTABLE="$1"
ROOT_DIR="$2"

# Check if executable exists and is executable
if [ ! -x "$EXECUTABLE" ]; then
    echo "Error: Executable '$EXECUTABLE' not found or not executable"
    exit 1
fi

# Check if root directory exists
if [ ! -d "$ROOT_DIR" ]; then
    echo "Error: Root directory '$ROOT_DIR' does not exist"
    exit 1
fi

# Ensure ROOT_DIR ends with a trailing slash
if [[ ! "$ROOT_DIR" == */ ]]; then
    ROOT_DIR="${ROOT_DIR}/"
fi

# Set output file path
OUTPUT_FILE="/Users/echo/Desktop/PhD/Tools/TARZAN/output/benchmark_results.txt"

echo "Running executable: $EXECUTABLE"
echo "Root directory: $ROOT_DIR"
echo "Memory usage will be logged to: $OUTPUT_FILE"
echo "----------------------------------------"

# Append header to output file
echo "" >> "$OUTPUT_FILE"
echo "========================================" >> "$OUTPUT_FILE"
echo "Executable: $EXECUTABLE" >> "$OUTPUT_FILE"
echo "Root directory: $ROOT_DIR" >> "$OUTPUT_FILE"
echo "Timestamp: $(date)" >> "$OUTPUT_FILE"
echo "========================================" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Iterate through each subdirectory in the root directory
for dir in "$ROOT_DIR"*/; do
    # Check if it's actually a directory
    if [ -d "$dir" ]; then
        # Get the folder name (basename)
        folder_name=$(basename "$dir")

        echo "Processing: $folder_name"
        echo "Path: $dir"

        # Log to file
        echo "----------------------------------------" >> "$OUTPUT_FILE"
        echo "Benchmark: $folder_name" >> "$OUTPUT_FILE"
        echo "Path: $dir" >> "$OUTPUT_FILE"
        echo "$OUTPUT_FILE"

        # Call the executable with time measurement and capture memory stats
        /usr/bin/time -l "$EXECUTABLE" "$dir" "$folder_name" 2>&1 | tee -a "$OUTPUT_FILE"

        exit_code=${PIPESTATUS[0]}
        echo "Exit code: $exit_code" >> "$OUTPUT_FILE"

        if [ $exit_code -ne 0 ]; then
            echo "Warning: Executable returned non-zero exit code ($exit_code) for $folder_name"
            echo "Warning: Non-zero exit code ($exit_code)" >> "$OUTPUT_FILE"
        fi

        echo "" >> "$OUTPUT_FILE"
        echo "----------------------------------------"
    fi
done

echo "Done processing all subdirectories"
echo "Memory usage report saved to: $OUTPUT_FILE"