#!/bin/bash

# Shell script to run all backwards executables and capture their output.

# Define directories.
EXECUTABLES_DIR="../../executables/backwards_executables"
OUTPUT_DIR="../../output/backwards_tarzan_results"

# Clear the output directory before execution.
echo "Clearing output directory: $OUTPUT_DIR"
rm -rf "${OUTPUT_DIR:?}"/*

# Create output directory if it doesn't exist.
mkdir -p "$OUTPUT_DIR"

# Check if executables directory exists.
if [ ! -d "$EXECUTABLES_DIR" ]; then
    echo "Error: Executables directory does not exist: $EXECUTABLES_DIR"
    exit 1
fi

# Change to executables directory.
cd "$EXECUTABLES_DIR" || exit 1

# Find and execute all executable files.
echo "Running executables from: $EXECUTABLES_DIR"
echo "Output will be saved to: $OUTPUT_DIR"
echo "----------------------------------------"

for executable in *; do
    # Check if it's a file and executable.
    if [ -f "$executable" ] && [ -x "$executable" ]; then
        # Extract the first word before the first underscore.
        base_name="${executable%%_*}"

        # If there's no underscore, use the whole name.
        if [ "$base_name" = "$executable" ]; then
            base_name="$executable"
        fi

        output_file="$OUTPUT_DIR/${base_name}.txt"

        echo "Executing: $executable"
        echo "Output file: $output_file"

        # Execute and append output to file.
        ./"$executable" >> "$output_file" 2>&1
        exit_code=$?

        # Check if process was killed by SIGKILL (signal 9), which typically indicates OOM.
        # Exit code 137 = 128 + 9 (SIGKILL).
        if [ $exit_code -eq 137 ]; then
            echo ""
            echo "!!! Stopping current backward test due to memory exhaustion." >> "$output_file"
            echo "  -> Exit code: $exit_code (OOM - SIGKILL)"
        else
            echo "  -> Exit code: $exit_code"
        fi
        echo ""
    fi
done

echo "----------------------------------------"
echo "All executables completed!"
echo "Results saved in: $OUTPUT_DIR"