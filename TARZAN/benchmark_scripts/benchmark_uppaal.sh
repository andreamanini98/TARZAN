#!/bin/bash

# Benchmarks that should use --search-order 0 instead of 1.
SEARCH_ORDER_0_BENCHMARKS=("trainAHV93")


# Check if UPPAAL path argument is provided.
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <path_to_uppaal_verifyta>"
    echo "Example: $0 ./UPPAAL-5.0.0.app/Contents/Resources/uppaal/bin/verifyta"
    exit 1
fi

UPPAAL_PATH="$1"

# Check if UPPAAL executable exists.
if [[ ! -f "$UPPAAL_PATH" ]]; then
    echo "Error: UPPAAL executable not found at: $UPPAAL_PATH"
    exit 1
fi

# Number of runs for averaging results.
NUM_RUNS="$2"

# Timeout configuration (in seconds).
TIMEOUT="$3"

BENCHMARK_DIR="../benchmarks"
OUTPUT_DIR="../../output/benchmark_uppaal_results"

# Create output directory if it doesn't exist.
mkdir -p "$OUTPUT_DIR"

# Iterate through each benchmark directory.
for benchmark in "$BENCHMARK_DIR"/*; do
    if [[ -d "$benchmark" ]]; then
        benchmark_name=$(basename "$benchmark")
        xta_dir="$benchmark/xta"

        # Check if xta subdirectory exists.
        if [[ -d "$xta_dir" ]]; then
            echo "Processing benchmark: $benchmark_name"

            # Create output directory for this benchmark.
            benchmark_output_dir="$OUTPUT_DIR/$benchmark_name"
            mkdir -p "$benchmark_output_dir"

            # Iterate through each subdirectory in xta.
            for subdir in "$xta_dir"/*; do
                if [[ -d "$subdir" ]]; then
                    subdir_name=$(basename "$subdir")
                    echo "  Processing subdirectory: $subdir_name"

                    # Find the .q file in the subdirectory.
                    q_file=$(find "$subdir" -maxdepth 1 -name "*.q" -type f | head -n 1)

                    if [[ -z "$q_file" ]]; then
                        echo "    Warning: No .q file found in $subdir"
                        continue
                    fi

                    # Find the .xta file in the subdirectory.
                    xta_file=$(find "$subdir" -maxdepth 1 -name "*.xta" -type f | head -n 1)

                    if [[ -z "$xta_file" ]]; then
                        echo "    Warning: No .xta file found in $subdir"
                        continue
                    fi

                    # Define output file path (named after the subdirectory).
                    output_file="$benchmark_output_dir/${subdir_name}.txt"
                    temp_output_dir="$benchmark_output_dir/.temp_${subdir_name}"
                    mkdir -p "$temp_output_dir"

                    echo "    Running: $(basename "$xta_file") with $(basename "$q_file") ($NUM_RUNS times)"
                    echo "    Output will be saved to: $output_file"

                    # Determine search order based on benchmark name.
                    search_order=1
                    for benchmark_pattern in "${SEARCH_ORDER_0_BENCHMARKS[@]}"; do
                        if [[ "$benchmark_name" == "$benchmark_pattern" ]]; then
                            search_order=0
                            break
                        fi
                    done
                    echo "    Using --search-order $search_order"

                    # Initialize accumulator variables.
                    total_states_stored=0
                    total_states_explored=0
                    total_cpu_time=0
                    total_virtual_mem=0
                    total_resident_mem=0
                    successful_runs=0
                    timeout_occurred=false
                    formula_result=""

                    # Run the verification NUM_RUNS times.
                    for ((run=1; run<=NUM_RUNS; run++)); do
                        temp_file="$temp_output_dir/run_${run}.txt"

                        echo "      Run $run/$NUM_RUNS..."

                        # Execute UPPAAL verification with timeout and capture output.
                        # Strip ANSI escape codes from the output using sed.
                        timeout "$TIMEOUT" "$UPPAAL_PATH" --search-order $search_order -u "$xta_file" "$q_file" 2>&1 | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' > "$temp_file"
                        exit_code=${PIPESTATUS[0]}

                        # Check if timeout occurred.
                        if [[ $exit_code -eq 124 ]]; then
                            echo "      WARNING: Timeout expired on run $run"
                            timeout_occurred=true
                            break
                        fi

                        # Check for other error conditions.
                        if [[ $exit_code -ne 0 ]]; then
                            echo "      WARNING: UPPAAL exited with error code $exit_code on run $run"
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
                        if grep -q "Formula is NOT satisfied" "$temp_file"; then
                            formula_result="NOT satisfied"
                        elif grep -q "Formula is satisfied" "$temp_file"; then
                            formula_result="satisfied"
                        fi

                        states_stored=$(grep "States stored" "$temp_file" | sed -E 's/.*States stored : ([0-9]+) states.*/\1/')
                        states_explored=$(grep "States explored" "$temp_file" | sed -E 's/.*States explored : ([0-9]+) states.*/\1/')
                        cpu_time=$(grep "CPU user time used" "$temp_file" | sed -E 's/.*CPU user time used : ([0-9]+) ms.*/\1/')
                        virtual_mem=$(grep "Virtual memory used" "$temp_file" | sed -E 's/.*Virtual memory used : ([0-9]+) KiB.*/\1/')
                        resident_mem=$(grep "Resident memory used" "$temp_file" | sed -E 's/.*Resident memory used : ([0-9]+) KiB.*/\1/')

                        # Accumulate values if all were successfully extracted.
                        if [[ -n "$states_stored" && -n "$states_explored" && -n "$cpu_time" && -n "$virtual_mem" && -n "$resident_mem" ]]; then
                            total_states_stored=$((total_states_stored + states_stored))
                            total_states_explored=$((total_states_explored + states_explored))
                            total_cpu_time=$((total_cpu_time + cpu_time))
                            total_virtual_mem=$((total_virtual_mem + virtual_mem))
                            total_resident_mem=$((total_resident_mem + resident_mem))
                            successful_runs=$((successful_runs + 1))
                        else
                            echo "      WARNING: Could not extract metrics from run $run output"
                        fi
                    done

                    # Write averaged results to output file.
                    if [[ $timeout_occurred == true ]]; then
                        echo "TIMEOUT: Execution exceeded $TIMEOUT seconds" > "$output_file"
                    elif [[ $successful_runs -gt 0 ]]; then
                        # Calculate averages.
                        avg_states_stored=$((total_states_stored / successful_runs))
                        avg_states_explored=$((total_states_explored / successful_runs))
                        avg_cpu_time=$((total_cpu_time / successful_runs))
                        avg_virtual_mem=$((total_virtual_mem / successful_runs))
                        avg_resident_mem=$((total_resident_mem / successful_runs))

                        # Convert KiB to MB and GB.
                        avg_virtual_mem_mb=$(awk "BEGIN {printf \"%.2f\", $avg_virtual_mem / 1024}")
                        avg_virtual_mem_gb=$(awk "BEGIN {printf \"%.2f\", $avg_virtual_mem / 1048576}")
                        avg_resident_mem_mb=$(awk "BEGIN {printf \"%.2f\", $avg_resident_mem / 1024}")
                        avg_resident_mem_gb=$(awk "BEGIN {printf \"%.2f\", $avg_resident_mem / 1048576}")

                        # Convert milliseconds to seconds.
                        avg_cpu_time_sec=$(awk "BEGIN {printf \"%.3f\", $avg_cpu_time / 1000}")

                        # Get the header from the first run.
                        head -n 7 "$temp_output_dir/run_1.txt" > "$output_file"

                        # Append averaged results.
                        echo "" >> "$output_file"
                        echo "Averaged results over $successful_runs runs:" >> "$output_file"
                        if [[ -n "$formula_result" ]]; then
                            echo " -- Formula is $formula_result." >> "$output_file"
                        fi
                        echo " -- States stored : $avg_states_stored states" >> "$output_file"
                        echo " -- States explored : $avg_states_explored states" >> "$output_file"
                        echo " -- CPU user time used : $avg_cpu_time ms ($avg_cpu_time_sec s)" >> "$output_file"
                        echo " -- Virtual memory used : $avg_virtual_mem KiB ($avg_virtual_mem_mb MB, $avg_virtual_mem_gb GB)" >> "$output_file"
                        echo " -- Resident memory used : $avg_resident_mem KiB ($avg_resident_mem_mb MB, $avg_resident_mem_gb GB)" >> "$output_file"
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
