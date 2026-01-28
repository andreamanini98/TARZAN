#!/bin/bash

# Shell script to run all games executables and capture their output and memory usage.

# Define directories (resolve to absolute paths before any cd).
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLES_DIR="${SCRIPT_DIR}/../../../executables/games_executables"
OUTPUT_DIR="${SCRIPT_DIR}/../../../output/games_tarzan_results"

# Detect OS and set appropriate /usr/bin/time options.
if [[ "$OSTYPE" == "darwin"* ]]; then
    TIME_FLAG="-l"
    MEMORY_GREP_PATTERN="maximum resident set size"
    MEMORY_MULTIPLIER=1
else
    TIME_FLAG="-v"
    MEMORY_GREP_PATTERN="Maximum resident set size"
    MEMORY_MULTIPLIER=1024
fi

# Verify /usr/bin/time is available.
if ! command -v /usr/bin/time > /dev/null 2>&1; then
    echo "Error: /usr/bin/time not found"
    if [[ "$OSTYPE" != "darwin"* ]]; then
        echo "On Linux, install the time package:"
        echo "  Ubuntu/Debian: sudo apt-get install time"
        echo "  RHEL/CentOS: sudo yum install time"
    fi
    exit 1
fi

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
        output_file="$OUTPUT_DIR/${executable}.txt"
        TEMP_TIME_FILE=$(mktemp)

        echo "Executing: $executable"
        echo "Output file: $output_file"

        # Execute with /usr/bin/time to capture memory usage.
        # stdout goes to output file, stderr (time stats) goes to temp file.
        /usr/bin/time $TIME_FLAG ./"$executable" >> "$output_file" 2> "$TEMP_TIME_FILE"
        exit_code=$?

        # Parse RSS from time output.
        if [[ "$OSTYPE" == "darwin"* ]]; then
            max_rss=$(grep -i "$MEMORY_GREP_PATTERN" "$TEMP_TIME_FILE" | awk '{print $1}')
        else
            max_rss=$(grep -i "$MEMORY_GREP_PATTERN" "$TEMP_TIME_FILE" | awk -F': ' '{print $2}')
        fi

        if [[ -n "$max_rss" ]] && [[ "$max_rss" -gt 0 ]]; then
            max_rss_bytes=$((max_rss * MEMORY_MULTIPLIER))
            max_rss_mb=$(echo "scale=2; $max_rss_bytes / 1048576" | bc | awk '{printf "%.2f", $0}')
            echo "" >> "$output_file"
            echo "Peak Memory (RSS): ${max_rss_mb} MB (${max_rss_bytes} bytes)" >> "$output_file"
        fi

        # Check if process was killed by SIGKILL (signal 9), which typically indicates OOM.
        # Exit code 137 = 128 + 9 (SIGKILL).
        if [ $exit_code -eq 137 ]; then
            echo ""
            echo "!!! Stopping current game test due to memory exhaustion." >> "$output_file"
            echo "  -> Exit code: $exit_code (OOM - SIGKILL)"
        else
            echo "  -> Exit code: $exit_code"
        fi
        echo ""

        rm -f "$TEMP_TIME_FILE"
    fi
done

echo "----------------------------------------"
echo "All executables completed!"
echo "Results saved in: $OUTPUT_DIR"
