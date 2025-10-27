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

echo "Running executable: $EXECUTABLE"
echo "Root directory: $ROOT_DIR"
echo "----------------------------------------"

# Iterate through each subdirectory in the root directory
for dir in "$ROOT_DIR"*/; do
    # Check if it's actually a directory
    if [ -d "$dir" ]; then
        # Get the folder name (basename)
        folder_name=$(basename "$dir")

        echo "Processing: $folder_name"
        echo "Path: $dir"

        # Call the executable with the folder path and folder name
        "$EXECUTABLE" "$dir" "$folder_name"

        exit_code=$?
        if [ $exit_code -ne 0 ]; then
            echo "Warning: Executable returned non-zero exit code ($exit_code) for $folder_name"
        fi

        echo "----------------------------------------"
    fi
done

echo "Done processing all subdirectories"