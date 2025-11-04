#!/bin/bash

# Check if correct number of arguments provided.
if [[ "$#" -ne 7 ]]; then
    echo "Usage: $0 <executable_path> <root_directory> <num_runs> <output_directory> <output_filename> <timeout_seconds> <benchmark_key>"
    echo "Example: $0 ./flower /path/to/benchmarks 5 /path/to/output results 300 1"
    echo "Note: The output file will be created as <output_directory>/<output_filename>.txt"
    echo "      timeout_seconds: Maximum time in seconds for each executable run (0 for no timeout)"
    echo "      benchmark_key: Additional argument passed to the executable as the last argument"
    exit 1
fi

EXECUTABLE="$1"
ROOT_DIR="$2"
NUM_RUNS="$3"
OUTPUT_DIR="$4"
OUTPUT_FILENAME="$5"
TIMEOUT="$6"
BENCHMARK_KEY="$7"

# Validate NUM_RUNS is a positive integer.
if ! [[ "$NUM_RUNS" =~ ^[0-9]+$ ]] || [[ "$NUM_RUNS" -lt 1 ]]; then
    echo "Error: num_runs must be a positive integer"
    exit 1
fi

# Validate TIMEOUT is a non-negative integer.
if ! [[ "$TIMEOUT" =~ ^[0-9]+$ ]]; then
    echo "Error: timeout_seconds must be a non-negative integer"
    exit 1
fi

# Check if executable exists and is executable.
if ! [[ -x "$EXECUTABLE" ]]; then
    echo "Error: Executable '$EXECUTABLE' not found or not executable"
    exit 1
fi

# Check if root directory exists.
if ! [[ -d "$ROOT_DIR" ]]; then
    echo "Error: Root directory '$ROOT_DIR' does not exist"
    exit 1
fi

# Ensure ROOT_DIR ends with a trailing slash.
if ! [[ "$ROOT_DIR" == */ ]]; then
    ROOT_DIR="${ROOT_DIR}/"
fi

# Create output directory if it doesn't exist.
if [[ ! -d "$OUTPUT_DIR" ]]; then
    echo "Creating output directory: $OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
    if [[ $? -ne 0 ]]; then
        echo "Error: Failed to create output directory"
        exit 1
    fi
fi

# Ensure OUTPUT_DIR ends with a trailing slash.
if ! [[ "$OUTPUT_DIR" == */ ]]; then
    OUTPUT_DIR="${OUTPUT_DIR}/"
fi

# Remove .txt extension if user provided it, we'll add it ourselves.
OUTPUT_FILENAME="${OUTPUT_FILENAME%.txt}"

# Set output file path.
OUTPUT_FILE="${OUTPUT_DIR}${OUTPUT_FILENAME}.txt"

# Create the output file (overwrites if exists).
> "$OUTPUT_FILE"
if [[ $? -ne 0 ]]; then
    echo "Error: Failed to create output file: $OUTPUT_FILE"
    exit 1
fi

echo "Running executable: $EXECUTABLE"
echo "Root directory: $ROOT_DIR"
echo "Number of runs per benchmark: $NUM_RUNS"
if [[ "$TIMEOUT" -gt 0 ]]; then
    echo "Timeout per run: $TIMEOUT seconds"
else
    echo "Timeout: disabled"
fi

# Write header to output file.
echo "===============================================================" >> "$OUTPUT_FILE"
echo "Benchmark Results: ${OUTPUT_FILENAME}" >> "$OUTPUT_FILE"
echo "===============================================================" >> "$OUTPUT_FILE"
echo "Executable:     $EXECUTABLE" >> "$OUTPUT_FILE"
echo "Root directory: $ROOT_DIR" >> "$OUTPUT_FILE"
echo "Number of runs: $NUM_RUNS" >> "$OUTPUT_FILE"
if [[ "$TIMEOUT" -gt 0 ]]; then
    echo "Timeout:        $TIMEOUT seconds" >> "$OUTPUT_FILE"
else
    echo "Timeout:       disabled" >> "$OUTPUT_FILE"
fi
echo "Output file:    $OUTPUT_FILE" >> "$OUTPUT_FILE"
echo "Timestamp:      $(date)" >> "$OUTPUT_FILE"
echo "========================================" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Check if ROOT_DIR contains subdirectories or is itself a benchmark directory.
shopt -s nullglob
subdirs=("$ROOT_DIR"*/)
shopt -u nullglob

# If no subdirectories found, treat ROOT_DIR itself as the benchmark directory.
if [[ ${#subdirs[@]} -eq 0 ]]; then
    subdirs=("$ROOT_DIR")
fi

# Iterate through each subdirectory (or the root directory itself).
for dir in "${subdirs[@]}"; do
    # Check if it's actually a directory.
    if [[ -d "$dir" ]]; then
        # Get the folder name (basename).
        folder_name=$(basename "$dir")

        echo "Processing: $folder_name (running $NUM_RUNS times)"
        echo "Path: $dir"

        # Log to file.
        echo "----------------------------------------" >> "$OUTPUT_FILE"
        echo "Benchmark: $folder_name" >> "$OUTPUT_FILE"
        echo "Path:      $dir" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"

        # Initialize accumulators
        total_regions=0
        total_time=0
        total_memory=0
        total_exec_time=0
        successful_runs=0
        timeout_count=0
        memory_data_count=0
        exec_time_count=0
        goal_reachable=""

        # Run the executable NUM_RUNS times.
        for ((run=1; run<=NUM_RUNS; run++)); do
            echo "  Run $run/$NUM_RUNS..."

            # Temporary files to capture time output and program output.
            TEMP_TIME_FILE=$(mktemp)
            TEMP_OUTPUT_FILE=$(mktemp)

            # Run the executable with or without timeout.
            timed_out=0
            if [[ "$TIMEOUT" -gt 0 ]]; then
                # Wrap timeout with /usr/bin/time so we can capture memory even on timeout.
                # Capture total execution time with millisecond precision
                start=$(gdate +%s%3N)
                /usr/bin/time -l timeout "$TIMEOUT" "$EXECUTABLE" "$dir" "$BENCHMARK_KEY" > "$TEMP_OUTPUT_FILE" 2> "$TEMP_TIME_FILE"
                exit_code=$?
                end=$(gdate +%s%3N)
                exec_time_ms=$((end - start))

                # Exit code 124 means timeout was reached.
                if [[ $exit_code -eq 124 ]]; then
                    timed_out=1
                    echo "  Timeout: Run $run exceeded $TIMEOUT seconds"
                fi
            else
                # No timeout - run normally.
                start=$(gdate +%s%3N)
                /usr/bin/time -l "$EXECUTABLE" "$dir" "$BENCHMARK_KEY" > "$TEMP_OUTPUT_FILE" 2> "$TEMP_TIME_FILE"
                exit_code=$?
                end=$(gdate +%s%3N)
                exec_time_ms=$((end - start))
            fi

            if [[ $timed_out -eq 1 ]]; then
                # Timeout occurred - don't count as successful, but collect memory if available.
                timeout_count=$((timeout_count + 1))
                echo "  Warning: Run $run timed out after $TIMEOUT seconds"
                # Stop executing further runs on timeout (like TChecker and UPPAAL)
                break
            elif [[ $exit_code -eq 0 ]]; then
                successful_runs=$((successful_runs + 1))

                # Parse the executable output.
                if [[ -s "$TEMP_OUTPUT_FILE" ]]; then
                    num_regions=$(grep "Number of regions:" "$TEMP_OUTPUT_FILE" | awk '{print $4}')
                    exec_time=$(grep "Total time" "$TEMP_OUTPUT_FILE" | awk '{print $4}')

                    if [[ -n "$num_regions" ]]; then
                        total_regions=$((total_regions + num_regions))
                    fi
                    if [[ -n "$exec_time" ]]; then
                        total_time=$((total_time + exec_time))
                    fi

                    # Check if goal is reachable (only capture on first successful run).
                    if [[ -z "$goal_reachable" ]]; then
                        if grep -q "Goal is reachable" "$TEMP_OUTPUT_FILE"; then
                            goal_reachable="reachable"
                        elif grep -q "Goal is not reachable" "$TEMP_OUTPUT_FILE"; then
                            goal_reachable="not reachable"
                        fi
                    fi
                fi
            else
                echo "  Warning: Run $run failed with exit code $exit_code"
            fi

            # Parse memory usage for all runs (successful, timed-out, and failed).
            if [[ -f "$TEMP_TIME_FILE" ]]; then
                max_rss=$(grep "maximum resident set size" "$TEMP_TIME_FILE" | awk '{print $1}')
                if [[ -n "$max_rss" ]] && [[ "$max_rss" -gt 0 ]]; then
                    total_memory=$((total_memory + max_rss))
                    memory_data_count=$((memory_data_count + 1))
                fi
            fi

            # Accumulate execution time for all runs (excluding timeouts)
            if [[ $timed_out -eq 0 ]] && [[ -n "$exec_time_ms" ]]; then
                total_exec_time=$((total_exec_time + exec_time_ms))
                exec_time_count=$((exec_time_count + 1))
            fi

            # Clean up temp files.
            rm -f "$TEMP_TIME_FILE" "$TEMP_OUTPUT_FILE"
        done

        # Calculate and write averages.
        if [[ $successful_runs -gt 0 ]]; then
            # Use bc for floating point division to avoid precision loss.
            avg_regions=$(echo "scale=0; $total_regions / $successful_runs" | bc)
            avg_time=$(echo "scale=0; $total_time / $successful_runs" | bc)

            # Convert time from microseconds to seconds with 6 decimal precision.
            # Add leading zero if result is less than 1.
            avg_time_seconds=$(echo "scale=6; $avg_time / 1000000" | bc | awk '{printf "%.6f", $0}')

            echo "Averaged Results (over $successful_runs successful run(s)):" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            echo "State space exploration statistics:" >> "$OUTPUT_FILE"
            echo "  Number of regions: $avg_regions" >> "$OUTPUT_FILE"
            echo "  Total time       : $avg_time_seconds s" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"

            # Add goal reachability information if available.
            if [[ -n "$goal_reachable" ]]; then
                if [[ "$goal_reachable" == "reachable" ]]; then
                    echo "Goal status: REACHABLE" >> "$OUTPUT_FILE"
                else
                    echo "Goal status: UNREACHABLE" >> "$OUTPUT_FILE"
                fi
                echo "" >> "$OUTPUT_FILE"
            fi

            # Calculate memory average from all runs with memory data.
            if [[ $memory_data_count -gt 0 ]]; then
                avg_memory=$(echo "scale=0; $total_memory / $memory_data_count" | bc)
                avg_mem_mb=$(echo "scale=2; $avg_memory / 1048576" | bc | awk '{printf "%.2f", $0}')
                avg_mem_gb=$(echo "scale=2; $avg_memory / 1073741824" | bc | awk '{printf "%.2f", $0}')

                echo "Memory Usage (averaged over $memory_data_count run(s)):" >> "$OUTPUT_FILE"
                echo "  Peak Memory (RSS):   ${avg_mem_mb} MB (${avg_mem_gb} GB)" >> "$OUTPUT_FILE"
                echo "  Peak Memory (bytes): ${avg_memory}" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi

            # Calculate execution time average from all runs with timing data.
            if [[ $exec_time_count -gt 0 ]]; then
                avg_exec_time_ms=$(echo "scale=0; $total_exec_time / $exec_time_count" | bc)
                avg_exec_time_sec=$(echo "scale=3; $avg_exec_time_ms / 1000" | bc | awk '{printf "%.3f", $0}')

                echo "Total Execution Time (averaged over $exec_time_count run(s)):" >> "$OUTPUT_FILE"
                echo "  Total Execution Time: ${avg_exec_time_ms} ms (${avg_exec_time_sec} s)" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi

            # Build status message with run statistics.
            status_msg="Status: SUCCESS ($successful_runs/$NUM_RUNS runs successful"
            if [[ $timeout_count -gt 0 ]]; then
                status_msg="$status_msg, $timeout_count timed out"
            fi
            failed_runs=$((NUM_RUNS - successful_runs - timeout_count))
            if [[ $failed_runs -gt 0 ]]; then
                status_msg="$status_msg, $failed_runs failed"
            fi
            status_msg="$status_msg)"
            echo "$status_msg" >> "$OUTPUT_FILE"
        elif [[ $timeout_count -eq $NUM_RUNS ]]; then
            # All runs timed out.
            echo "Timeout expired" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"

            # Calculate memory average if we have data
            if [[ $memory_data_count -gt 0 ]]; then
                avg_memory=$(echo "scale=0; $total_memory / $memory_data_count" | bc)
                avg_mem_mb=$(echo "scale=2; $avg_memory / 1048576" | bc | awk '{printf "%.2f", $0}')
                avg_mem_gb=$(echo "scale=2; $avg_memory / 1073741824" | bc | awk '{printf "%.2f", $0}')

                echo "Memory Usage (averaged over $memory_data_count run(s)):" >> "$OUTPUT_FILE"
                echo "  Peak Memory (RSS):   ${avg_mem_mb} MB (${avg_mem_gb} GB)" >> "$OUTPUT_FILE"
                echo "  Peak Memory (bytes): ${avg_memory}" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi

            # Calculate execution time average if we have data (before timeout)
            if [[ $exec_time_count -gt 0 ]]; then
                avg_exec_time_ms=$(echo "scale=0; $total_exec_time / $exec_time_count" | bc)
                avg_exec_time_sec=$(echo "scale=3; $avg_exec_time_ms / 1000" | bc | awk '{printf "%.3f", $0}')

                echo "Total Execution Time (averaged over $exec_time_count run(s) before timeout):" >> "$OUTPUT_FILE"
                echo "  Total Execution Time: ${avg_exec_time_ms} ms (${avg_exec_time_sec} s)" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi

            echo "Status: TIMEOUT (all $NUM_RUNS runs exceeded $TIMEOUT seconds)" >> "$OUTPUT_FILE"
            echo "Warning: All runs timed out for $folder_name"
        else
            # Mix of timeouts and/or failures - no successful runs.
            failed_runs=$((NUM_RUNS - timeout_count))

            if [[ $timeout_count -gt 0 ]] && [[ $failed_runs -gt 0 ]]; then
                echo "Mixed failure (timeouts and errors)" >> "$OUTPUT_FILE"
            elif [[ $timeout_count -gt 0 ]]; then
                echo "Timeout expired" >> "$OUTPUT_FILE"
            else
                echo "Execution failed" >> "$OUTPUT_FILE"
            fi
            echo "" >> "$OUTPUT_FILE"

            # Calculate memory average if we have data.
            if [[ $memory_data_count -gt 0 ]]; then
                avg_memory=$(echo "scale=0; $total_memory / $memory_data_count" | bc)
                avg_mem_mb=$(echo "scale=2; $avg_memory / 1048576" | bc | awk '{printf "%.2f", $0}')
                avg_mem_gb=$(echo "scale=2; $avg_memory / 1073741824" | bc | awk '{printf "%.2f", $0}')

                echo "Memory Usage (averaged over $memory_data_count run(s)):" >> "$OUTPUT_FILE"
                echo "  Peak Memory (RSS):   ${avg_mem_mb} MB (${avg_mem_gb} GB)" >> "$OUTPUT_FILE"
                echo "  Peak Memory (bytes): ${avg_memory}" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi

            # Calculate execution time average if we have data
            if [[ $exec_time_count -gt 0 ]]; then
                avg_exec_time_ms=$(echo "scale=0; $total_exec_time / $exec_time_count" | bc)
                avg_exec_time_sec=$(echo "scale=3; $avg_exec_time_ms / 1000" | bc | awk '{printf "%.3f", $0}')

                echo "Total Execution Time (averaged over $exec_time_count run(s)):" >> "$OUTPUT_FILE"
                echo "  Total Execution Time: ${avg_exec_time_ms} ms (${avg_exec_time_sec} s)" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi

            if [[ $failed_runs -gt 0 ]]; then
                echo "Status: FAILED ($timeout_count timed out, $failed_runs failed)" >> "$OUTPUT_FILE"
            else
                echo "Status: TIMEOUT ($timeout_count timed out)" >> "$OUTPUT_FILE"
            fi
            echo "Warning: All runs failed for $folder_name"
        fi

        echo "" >> "$OUTPUT_FILE"
        echo "----------------------------------------"
    fi
done

echo "Done processing all subdirectories"
echo "Memory usage report saved to: $OUTPUT_FILE"
echo "Exiting Benchmark ${OUTPUT_FILENAME}"