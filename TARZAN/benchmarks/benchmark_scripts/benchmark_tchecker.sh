#!/bin/bash

# Force C locale to ensure consistent number formatting (period as decimal separator).
export LC_ALL=C
export LANG=C


# Benchmarks that should use bfs instead of dfs.
SEARCH_ORDER_0_BENCHMARKS=("trainAHV93")


# Check if TChecker path argument is provided.
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <path_to_tchecker>"
    echo "Example: $0 /usr/local/bin/tchecker"
    exit 1
fi

TCHECKER_PATH="$1"

# Check if TChecker executable exists.
if [[ ! -f "$TCHECKER_PATH" ]]; then
    echo "Error: TChecker executable not found at: $TCHECKER_PATH"
    exit 1
fi

# Number of runs for averaging results.
NUM_RUNS="$2"

# Timeout configuration (in seconds).
TIMEOUT="$3"

BENCHMARK_DIR="../models"
OUTPUT_DIR="../../../output/benchmark_tchecker_results"

# Create output directory if it doesn't exist.
mkdir -p "$OUTPUT_DIR"

# Iterate through each benchmark directory.
for benchmark in "$BENCHMARK_DIR"/*; do
    if [[ -d "$benchmark" ]]; then
        benchmark_name=$(basename "$benchmark")
        tck_dir="$benchmark/tck"

        # Check if tck subdirectory exists.
        if [[ -d "$tck_dir" ]]; then
            echo "Processing benchmark: $benchmark_name"

            # Create output directory for this benchmark.
            benchmark_output_dir="$OUTPUT_DIR/$benchmark_name"
            mkdir -p "$benchmark_output_dir"

            # Iterate through each subdirectory in tck.
            for subdir in "$tck_dir"/*; do
                if [[ -d "$subdir" ]]; then
                    subdir_name=$(basename "$subdir")
                    echo "  Processing subdirectory: $subdir_name"

                    # Find the .tck file in the subdirectory.
                    tck_file=$(find "$subdir" -maxdepth 1 -name "*.tck" -type f | head -n 1)

                    if [[ -z "$tck_file" ]]; then
                        echo "    Warning: No .tck file found in $subdir"
                        continue
                    fi

                    # Find the labels.txt file in the subdirectory (optional).
                    labels_file=$(find "$subdir" -maxdepth 1 -name "labels.txt" -type f | head -n 1)

                    # Define output file path (named after the subdirectory).
                    output_file="$benchmark_output_dir/${subdir_name}.txt"
                    temp_output_dir="$benchmark_output_dir/.temp_${subdir_name}"
                    mkdir -p "$temp_output_dir"

                    if [[ -n "$labels_file" ]]; then
                        echo "    Running: $(basename "$tck_file") with $(basename "$labels_file") ($NUM_RUNS times)"
                    else
                        echo "    Running: $(basename "$tck_file") (no labels) ($NUM_RUNS times)"
                    fi
                    echo "    Output will be saved to: $output_file"

                    # Determine search order based on benchmark name.
                    search_order=1
                    for benchmark_pattern in "${SEARCH_ORDER_0_BENCHMARKS[@]}"; do
                        if [[ "$benchmark_name" == "$benchmark_pattern" ]]; then
                            search_order=0
                            break
                        fi
                    done
                    # Determine search algorithm based on search order.
                    if [[ $search_order -eq 0 ]]; then
                        search_algorithm="bfs"
                    else
                        search_algorithm="dfs"
                    fi
                    echo "    Using search algorithm: $search_algorithm"

                    # Read labels from labels.txt file if it exists.
                    labels_content=""
                    if [[ -f "$labels_file" ]]; then
                        labels_content=$(cat "$labels_file" | tr -d '\n')
                        echo "    Labels: $labels_content"
                    else
                        echo "    Labels: none"
                    fi

                    # Initialize accumulator variables.
                    total_covered_states=0
                    total_memory_max_rss=0
                    total_running_time=0
                    total_exec_time=0
                    total_stored_states=0
                    total_visited_states=0
                    total_visited_transitions=0
                    successful_runs=0
                    exec_time_count=0
                    timeout_occurred=false
                    reachable_result=""

                    # Warm-up run to eliminate cold start effects (not measured)
                    echo "  Warm-up run (not measured)..."
                    if [[ "$TIMEOUT" -gt 0 ]]; then
                        timeout 1 "$TCHECKER_PATH" -a covreach -s "$search_algorithm" -l "$labels_content" "$tck_file" > /dev/null 2>&1
                    else
                        "$TCHECKER_PATH" -a covreach -s "$search_algorithm" "$tck_file" > /dev/null 2>&1
                    fi

                    # Run the verification NUM_RUNS times.
                    for ((run=1; run<=NUM_RUNS; run++)); do
                        temp_file="$temp_output_dir/run_${run}.txt"
                        temp_timing_file="$temp_output_dir/timing_${run}.txt"

                        echo "      Run $run/$NUM_RUNS..."

                        # Execute TChecker verification with timeout and capture output.
                        # Capture total execution time with millisecond precision inside redirected context
                        {
                            start=$(gdate +%s%3N)
                            if [[ -n "$labels_content" ]]; then
                                timeout "$TIMEOUT" "$TCHECKER_PATH" -a covreach -s "$search_algorithm" -l "$labels_content" "$tck_file"
                            else
                                timeout "$TIMEOUT" "$TCHECKER_PATH" -a covreach -s "$search_algorithm" "$tck_file"
                            fi
                            exit_code=${PIPESTATUS[0]}
                            end=$(gdate +%s%3N)
                            exec_time_ms=$((end - start))
                            echo "$exec_time_ms" > "$temp_timing_file"
                            echo "$exit_code" >> "$temp_timing_file"
                        } > "$temp_file"

                        # Read back the timing data
                        exec_time_ms=$(head -n 1 "$temp_timing_file")
                        exit_code=$(tail -n 1 "$temp_timing_file")

                        # Check if timeout occurred.
                        if [[ $exit_code -eq 124 ]]; then
                            echo "      WARNING: Timeout expired on run $run"
                            timeout_occurred=true
                            break
                        fi

                        # Check for other error conditions.
                        if [[ $exit_code -ne 0 ]]; then
                            echo "      WARNING: TChecker exited with error code $exit_code on run $run"
                            # Check for common error patterns.
                            if grep -qi "out of memory\|OutOfMemoryError\|Cannot allocate memory" "$temp_file"; then
                                echo "      ERROR: Out of memory detected"
                                echo "ERROR: Out of memory during execution" > "$output_file"
                                cat "$temp_file" >> "$output_file"
                                rm -rf "$temp_output_dir"
                                break 2  # Break out of both loops
                            elif grep -qi "error\|exception\|failed" "$temp_file"; then
                                echo "      ERROR: Execution error detected"
                                echo "ERROR: Execution failed on run $run with exit code $exit_code" > "$output_file"
                                cat "$temp_file" >> "$output_file"
                                rm -rf "$temp_output_dir"
                                break 2  # Break out of both loops.
                            fi
                            # If no specific error pattern, skip this run and continue.
                            continue
                        fi

                        # Extract metrics from the output.
                        covered_states=$(grep "COVERED_STATES" "$temp_file" | awk '{print $2}')
                        memory_max_rss=$(grep "MEMORY_MAX_RSS" "$temp_file" | awk '{print $2}')
                        reachable=$(grep "REACHABLE" "$temp_file" | awk '{print $2}')
                        running_time=$(grep "RUNNING_TIME_SECONDS" "$temp_file" | awk '{print $2}')
                        stored_states=$(grep "STORED_STATES" "$temp_file" | awk '{print $2}')
                        visited_states=$(grep "VISITED_STATES" "$temp_file" | awk '{print $2}')
                        visited_transitions=$(grep "VISITED_TRANSITIONS" "$temp_file" | awk '{print $2}')

                        # Store reachable result (should be same across runs).
                        if [[ -n "$reachable" ]]; then
                            reachable_result="$reachable"
                        fi

                        # Accumulate values if all were successfully extracted.
                        if [[ -n "$covered_states" && -n "$memory_max_rss" && -n "$running_time" && -n "$stored_states" && -n "$visited_states" && -n "$visited_transitions" ]]; then
                            total_covered_states=$((total_covered_states + covered_states))
                            total_memory_max_rss=$((total_memory_max_rss + memory_max_rss))
                            total_running_time=$(awk "BEGIN {printf \"%.10f\", $total_running_time + $running_time}")
                            total_stored_states=$((total_stored_states + stored_states))
                            total_visited_states=$((total_visited_states + visited_states))
                            total_visited_transitions=$((total_visited_transitions + visited_transitions))
                            successful_runs=$((successful_runs + 1))
                        else
                            echo "      WARNING: Could not extract metrics from run $run output"
                        fi

                        # Accumulate execution time for all runs (excluding timeouts)
                        if [[ $exit_code -ne 124 ]] && [[ -n "$exec_time_ms" ]]; then
                            total_exec_time=$((total_exec_time + exec_time_ms))
                            exec_time_count=$((exec_time_count + 1))
                        fi
                    done

                    # Write averaged results to output file.
                    if [[ $timeout_occurred == true ]]; then
                        echo "TIMEOUT: Execution exceeded $TIMEOUT seconds" > "$output_file"
                    elif [[ $successful_runs -gt 0 ]]; then
                        # Calculate averages.
                        avg_covered_states=$((total_covered_states / successful_runs))
                        avg_memory_max_rss=$((total_memory_max_rss / successful_runs))
                        avg_running_time=$(awk "BEGIN {printf \"%.10f\", $total_running_time / $successful_runs}")
                        avg_stored_states=$((total_stored_states / successful_runs))
                        avg_visited_states=$((total_visited_states / successful_runs))
                        avg_visited_transitions=$((total_visited_transitions / successful_runs))

                        # Convert bytes to MB and GB.
                        avg_memory_mb=$(awk "BEGIN {printf \"%.2f\", $avg_memory_max_rss / 1024 / 1024}")
                        avg_memory_gb=$(awk "BEGIN {printf \"%.4f\", $avg_memory_max_rss / 1024 / 1024 / 1024}")

                        # Write results to output file.
                        echo "Averaged results over $successful_runs runs:" > "$output_file"
                        echo "COVERED_STATES $avg_covered_states" >> "$output_file"
                        echo "MEMORY_MAX_RSS $avg_memory_max_rss bytes ($avg_memory_mb MB, $avg_memory_gb GB)" >> "$output_file"
                        if [[ -n "$reachable_result" ]]; then
                            echo "REACHABLE $reachable_result" >> "$output_file"
                        fi
                        echo "RUNNING_TIME_SECONDS $avg_running_time" >> "$output_file"
                        echo "STORED_STATES $avg_stored_states" >> "$output_file"
                        echo "VISITED_STATES $avg_visited_states" >> "$output_file"
                        echo "VISITED_TRANSITIONS $avg_visited_transitions" >> "$output_file"

                        # Calculate and write execution time average
                        if [[ $exec_time_count -gt 0 ]]; then
                            avg_exec_time_ms=$(echo "scale=0; $total_exec_time / $exec_time_count" | bc)
                            avg_exec_time_sec=$(awk "BEGIN {printf \"%.3f\", $avg_exec_time_ms / 1000}")
                            echo "TOTAL_EXECUTION_TIME_MS $avg_exec_time_ms" >> "$output_file"
                            echo "TOTAL_EXECUTION_TIME_SECONDS $avg_exec_time_sec" >> "$output_file"
                        fi
                    else
                        echo "ERROR: No successful runs completed" > "$output_file"
                    fi

                    # Clean up temporary files.
                    rm -rf "$temp_output_dir"

                    echo "-------"
                fi
            done

            echo ""
        fi
    fi
done

echo "Benchmark execution completed."