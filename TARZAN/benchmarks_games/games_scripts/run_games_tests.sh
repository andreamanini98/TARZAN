#!/bin/bash

# Shell script to run all games executables N times and average their output.

# Validate command-line argument.
if [[ "$#" -ne 1 ]]; then
    echo "Usage: $0 <num_runs>"
    echo "Example: $0 5"
    exit 1
fi

NUM_RUNS="$1"
if ! [[ "$NUM_RUNS" =~ ^[0-9]+$ ]] || [[ "$NUM_RUNS" -lt 1 ]]; then
    echo "Error: num_runs must be a positive integer"
    exit 1
fi

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
echo "Number of runs per executable: $NUM_RUNS"
echo "----------------------------------------"

for executable in *; do
    # Check if it's a file and executable.
    if [ -f "$executable" ] && [ -x "$executable" ]; then
        output_file="$OUTPUT_DIR/${executable}.txt"

        echo "Executing: $executable ($NUM_RUNS run(s))"
        echo "Output file: $output_file"

        # Accumulators.
        sum_time=0
        sum_iterations=0
        sum_starting_regions=0
        sum_set_phi=0
        sum_stored_regions=0
        sum_time_gen=0
        sum_memory=0
        successful_runs=0
        result_status=""
        consistency_error=0

        for ((run=1; run<=NUM_RUNS; run++)); do
            echo "  Run $run/$NUM_RUNS..."
            TEMP_TIME_FILE=$(mktemp)
            TEMP_OUTPUT_FILE=$(mktemp)

            # Execute with /usr/bin/time to capture memory usage.
            /usr/bin/time $TIME_FLAG ./"$executable" > "$TEMP_OUTPUT_FILE" 2> "$TEMP_TIME_FILE"
            exit_code=$?

            # Check for OOM.
            if [ $exit_code -eq 137 ]; then
                echo "  !!! Run $run killed by OOM (SIGKILL)"
                echo "!!! Run $run killed by OOM (SIGKILL)" >> "$output_file"
                rm -f "$TEMP_TIME_FILE" "$TEMP_OUTPUT_FILE"
                break
            fi

            if [ $exit_code -ne 0 ]; then
                echo "  Warning: Run $run failed with exit code $exit_code"
                rm -f "$TEMP_TIME_FILE" "$TEMP_OUTPUT_FILE"
                continue
            fi

            # Check VICTORY/LOSE consistency.
            run_status=""
            if grep -q "^VICTORY$" "$TEMP_OUTPUT_FILE"; then
                run_status="VICTORY"
            elif grep -q "^LOSE$" "$TEMP_OUTPUT_FILE"; then
                run_status="LOSE"
            fi

            if [ -z "$result_status" ]; then
                result_status="$run_status"
            elif [ "$run_status" != "$result_status" ]; then
                echo "ERROR: Inconsistent result for $executable: previous runs returned $result_status but run $run returned $run_status"
                echo "ERROR: Inconsistent result for $executable: previous runs returned $result_status but run $run returned $run_status" >> "$output_file"
                consistency_error=1
                rm -f "$TEMP_TIME_FILE" "$TEMP_OUTPUT_FILE"
                break
            fi

            # Parse numeric values.
            val=$(grep "^Total time:" "$TEMP_OUTPUT_FILE" | grep -v "including" | awk '{print $(NF-1)}')
            [[ -n "$val" ]] && sum_time=$(echo "$sum_time + $val" | bc)

            val=$(grep "Total iterations:" "$TEMP_OUTPUT_FILE" | awk '{print $NF}')
            [[ -n "$val" ]] && sum_iterations=$(echo "$sum_iterations + $val" | bc)

            val=$(grep "Total starting regions:" "$TEMP_OUTPUT_FILE" | awk '{print $NF}')
            [[ -n "$val" ]] && sum_starting_regions=$(echo "$sum_starting_regions + $val" | bc)

            val=$(grep "Total regions in setPhi:" "$TEMP_OUTPUT_FILE" | awk '{print $NF}')
            [[ -n "$val" ]] && sum_set_phi=$(echo "$sum_set_phi + $val" | bc)

            val=$(grep "Total stored regions:" "$TEMP_OUTPUT_FILE" | awk '{print $NF}')
            [[ -n "$val" ]] && sum_stored_regions=$(echo "$sum_stored_regions + $val" | bc)

            val=$(grep "Total time including region generation:" "$TEMP_OUTPUT_FILE" | awk '{print $(NF-1)}')
            [[ -n "$val" ]] && sum_time_gen=$(echo "$sum_time_gen + $val" | bc)

            # Parse RSS from time output.
            if [[ "$OSTYPE" == "darwin"* ]]; then
                max_rss=$(grep -i "$MEMORY_GREP_PATTERN" "$TEMP_TIME_FILE" | awk '{print $1}')
            else
                max_rss=$(grep -i "$MEMORY_GREP_PATTERN" "$TEMP_TIME_FILE" | awk -F': ' '{print $2}')
            fi
            if [[ -n "$max_rss" ]] && [[ "$max_rss" -gt 0 ]]; then
                max_rss_bytes=$((max_rss * MEMORY_MULTIPLIER))
                sum_memory=$(echo "$sum_memory + $max_rss_bytes" | bc)
            fi

            successful_runs=$((successful_runs + 1))
            rm -f "$TEMP_TIME_FILE" "$TEMP_OUTPUT_FILE"
        done

        # Skip writing averages if there was a consistency error.
        if [ $consistency_error -eq 1 ]; then
            echo ""
            continue
        fi

        # Write averaged results to output file.
        if [ $successful_runs -gt 0 ]; then
            avg_time=$(echo "scale=2; $sum_time / $successful_runs" | bc | awk '{printf "%.2f", $0}')
            avg_iterations=$(echo "scale=0; $sum_iterations / $successful_runs" | bc)
            avg_starting_regions=$(echo "scale=0; $sum_starting_regions / $successful_runs" | bc)
            avg_set_phi=$(echo "scale=0; $sum_set_phi / $successful_runs" | bc)
            avg_stored_regions=$(echo "scale=0; $sum_stored_regions / $successful_runs" | bc)
            avg_time_gen=$(echo "scale=2; $sum_time_gen / $successful_runs" | bc | awk '{printf "%.2f", $0}')
            avg_memory_bytes=$(echo "scale=0; $sum_memory / $successful_runs" | bc)
            avg_memory_mb=$(echo "scale=2; $avg_memory_bytes / 1048576" | bc | awk '{printf "%.2f", $0}')

            {
                echo "Averaged Results (over $successful_runs run(s)):"
                echo ""
                echo "Total time:              $avg_time microseconds"
                echo "Total iterations:        $avg_iterations"
                echo "Total starting regions:  $avg_starting_regions"
                echo "Total regions in setPhi: $avg_set_phi"
                echo "Total stored regions:    $avg_stored_regions"
                echo "$result_status"
                echo "Total time including region generation: $avg_time_gen microseconds"
                echo "-----"
                echo ""
                echo "Peak Memory (RSS): ${avg_memory_mb} MB (${avg_memory_bytes} bytes)"
            } >> "$output_file"

            echo "  -> Completed: $successful_runs/$NUM_RUNS runs successful, result: $result_status"
        else
            echo "  -> All runs failed for $executable" | tee -a "$output_file"
        fi
        echo ""
    fi
done

echo "----------------------------------------"
echo "All executables completed!"
echo "Results saved in: $OUTPUT_DIR"
