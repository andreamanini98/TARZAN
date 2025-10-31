#!/bin/bash

# Script to generate a comprehensive benchmark summary from TARZAN, TChecker, and UPPAAL results.

OUTPUT_DIR="../../output"
SUMMARY_FILE="$OUTPUT_DIR/benchmark_summary.txt"

# Check if output directories exist.
TARZAN_DIR="$OUTPUT_DIR/benchmark_tarzan_results"
TCHECKER_DIR="$OUTPUT_DIR/benchmark_tchecker_results"
UPPAAL_DIR="$OUTPUT_DIR/benchmark_uppaal_results"

if [[ ! -d "$TARZAN_DIR" && ! -d "$TCHECKER_DIR" && ! -d "$UPPAAL_DIR" ]]; then
    echo "Error: No benchmark result directories found in $OUTPUT_DIR"
    exit 1
fi

echo "Generating benchmark summary..."
echo "Output file: $SUMMARY_FILE"

# Create summary file header.
cat > "$SUMMARY_FILE" << 'EOF'
===============================================================================
                     BENCHMARK RESULTS SUMMARY
===============================================================================
This file contains a comprehensive summary of benchmark results from three
verification tools: TARZAN, TChecker, and UPPAAL.

Results are organized by:
  - Experiment (benchmark suite)
  - Sub-experiment (specific instance)
  - Tool (TARZAN, TChecker, UPPAAL)

Generated on: $(date)
===============================================================================

EOF

# Get all unique benchmark names (experiments) from any of the three directories.
experiments=()
for dir in "$TARZAN_DIR" "$TCHECKER_DIR" "$UPPAAL_DIR"; do
    if [[ -d "$dir" ]]; then
        for exp in "$dir"/*; do
            if [[ -d "$exp" ]]; then
                exp_name=$(basename "$exp")
                if [[ ! " ${experiments[@]} " =~ " ${exp_name} " ]]; then
                    experiments+=("$exp_name")
                fi
            fi
        done
    fi
done

# Sort experiments alphabetically.
IFS=$'\n' experiments=($(sort <<<"${experiments[*]}"))
unset IFS

echo "Found ${#experiments[@]} experiments to process"

# Process each experiment.
for experiment in "${experiments[@]}"; do
    echo "Processing experiment: $experiment"

    cat >> "$SUMMARY_FILE" << EOF

===============================================================================
EXPERIMENT: $experiment
===============================================================================

EOF

    # Get all sub-experiments (result files) for this experiment.
    sub_experiments=()

    for dir in "$TARZAN_DIR/$experiment" "$TCHECKER_DIR/$experiment" "$UPPAAL_DIR/$experiment"; do
        if [[ -d "$dir" ]]; then
            for result_file in "$dir"/*.txt; do
                if [[ -f "$result_file" ]]; then
                    sub_exp_name=$(basename "$result_file" .txt)
                    if [[ ! " ${sub_experiments[@]} " =~ " ${sub_exp_name} " ]]; then
                        sub_experiments+=("$sub_exp_name")
                    fi
                fi
            done
        fi
    done

    # Sort sub-experiments.
    IFS=$'\n' sub_experiments=($(sort <<<"${sub_experiments[*]}"))
    unset IFS

    # Process each sub-experiment.
    for sub_exp in "${sub_experiments[@]}"; do
        echo "  Processing sub-experiment: $sub_exp"

        cat >> "$SUMMARY_FILE" << EOF
-------------------------------------------------------------------------------
SUB-EXPERIMENT: $sub_exp
-------------------------------------------------------------------------------

EOF

        # Process TARZAN results.
        tarzan_file="$TARZAN_DIR/$experiment/${sub_exp}.txt"
        if [[ -f "$tarzan_file" ]]; then
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: TARZAN ---
EOF
            # Check for timeout or error.
            if grep -q "TIMEOUT:" "$tarzan_file" 2>/dev/null; then
                echo "TIMEOUT: Execution exceeded time limit" >> "$SUMMARY_FILE"
            elif grep -q "ERROR:" "$tarzan_file" 2>/dev/null; then
                echo "ERROR: Execution failed" >> "$SUMMARY_FILE"
                grep "ERROR:" "$tarzan_file" >> "$SUMMARY_FILE"
            else
                # Extract key metrics.
                num_regions=$(grep "Number of regions:" "$tarzan_file" | awk '{print $NF}')
                total_time=$(grep "Total time" "$tarzan_file" | awk '{print $(NF-1), $NF}')
                goal_status=$(grep "Goal status:" "$tarzan_file" | awk '{print $NF}')
                peak_memory_mb=$(grep "Peak Memory (RSS):" "$tarzan_file" | awk '{print $4, $5}')
                peak_memory_bytes=$(grep "Peak Memory (bytes):" "$tarzan_file" | awk '{print $NF}')
                status=$(grep "Status:" "$tarzan_file" | sed 's/Status: //')

                cat >> "$SUMMARY_FILE" << EOF
  Number of regions:    ${num_regions:-N/A}
  Total time:           ${total_time:-N/A}
  Goal status:          ${goal_status:-N/A}
  Peak memory:          ${peak_memory_mb:-N/A}
  Peak memory (bytes):  ${peak_memory_bytes:-N/A}
  Status:               ${status:-N/A}
EOF
            fi
            echo "" >> "$SUMMARY_FILE"
        else
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: TARZAN ---
  No results available

EOF
        fi

        # Process TChecker results.
        tchecker_file="$TCHECKER_DIR/$experiment/${sub_exp}.txt"
        if [[ -f "$tchecker_file" ]]; then
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: TChecker ---
EOF
            # Check for timeout or error.
            if grep -q "TIMEOUT:" "$tchecker_file" 2>/dev/null; then
                echo "TIMEOUT: Execution exceeded time limit" >> "$SUMMARY_FILE"
            elif grep -q "ERROR:" "$tchecker_file" 2>/dev/null; then
                echo "ERROR: Execution failed" >> "$SUMMARY_FILE"
                grep "ERROR:" "$tchecker_file" >> "$SUMMARY_FILE"
            else
                # Extract key metrics.
                covered_states=$(grep "COVERED_STATES" "$tchecker_file" | awk '{print $2}')
                memory_mb=$(grep "MEMORY_MAX_RSS" "$tchecker_file" | sed 's/.*(\(.*\) MB.*/\1/')
                memory_gb=$(grep "MEMORY_MAX_RSS" "$tchecker_file" | sed 's/.*, \(.*\) GB).*/\1/')
                memory_bytes=$(grep "MEMORY_MAX_RSS" "$tchecker_file" | awk '{print $2}')
                reachable=$(grep "REACHABLE" "$tchecker_file" | awk '{print $2}')
                running_time=$(grep "RUNNING_TIME_SECONDS" "$tchecker_file" | awk '{print $2}')
                stored_states=$(grep "STORED_STATES" "$tchecker_file" | awk '{print $2}')
                visited_states=$(grep "VISITED_STATES" "$tchecker_file" | awk '{print $2}')
                visited_transitions=$(grep "VISITED_TRANSITIONS" "$tchecker_file" | awk '{print $2}')

                cat >> "$SUMMARY_FILE" << EOF
  Covered states:       ${covered_states:-N/A}
  Memory (MB):          ${memory_mb:-N/A}
  Memory (GB):          ${memory_gb:-N/A}
  Memory (bytes):       ${memory_bytes:-N/A}
  Reachable:            ${reachable:-N/A}
  Running time (s):     ${running_time:-N/A}
  Stored states:        ${stored_states:-N/A}
  Visited states:       ${visited_states:-N/A}
  Visited transitions:  ${visited_transitions:-N/A}
EOF
            fi
            echo "" >> "$SUMMARY_FILE"
        else
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: TChecker ---
  No results available

EOF
        fi

        # Process UPPAAL results.
        uppaal_file="$UPPAAL_DIR/$experiment/${sub_exp}.txt"
        if [[ -f "$uppaal_file" ]]; then
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: UPPAAL ---
EOF
            # Check for timeout or error.
            if grep -q "TIMEOUT:" "$uppaal_file" 2>/dev/null; then
                echo "TIMEOUT: Execution exceeded time limit" >> "$SUMMARY_FILE"
            elif grep -q "ERROR:" "$uppaal_file" 2>/dev/null; then
                echo "ERROR: Execution failed" >> "$SUMMARY_FILE"
                grep "ERROR:" "$uppaal_file" >> "$SUMMARY_FILE"
            else
                # Extract key metrics.
                formula_result=$(grep "Formula is" "$uppaal_file" | sed 's/.*Formula is //')
                states_stored=$(grep "States stored" "$uppaal_file" | sed 's/.*: \([0-9]*\) states.*/\1/')
                states_explored=$(grep "States explored" "$uppaal_file" | sed 's/.*: \([0-9]*\) states.*/\1/')
                cpu_time_ms=$(grep "CPU user time used" "$uppaal_file" | sed 's/.*: \([0-9]*\) ms.*/\1/')
                cpu_time_s=$(grep "CPU user time used" "$uppaal_file" | sed 's/.*(\(.*\) s).*/\1/')
                virtual_mem_mb=$(grep "Virtual memory used" "$uppaal_file" | sed 's/.*(\(.*\) MB.*/\1/')
                virtual_mem_gb=$(grep "Virtual memory used" "$uppaal_file" | sed 's/.*, \(.*\) GB).*/\1/')
                resident_mem_mb=$(grep "Resident memory used" "$uppaal_file" | sed 's/.*(\(.*\) MB.*/\1/')
                resident_mem_gb=$(grep "Resident memory used" "$uppaal_file" | sed 's/.*, \(.*\) GB).*/\1/')

                cat >> "$SUMMARY_FILE" << EOF
  Formula result:       ${formula_result:-N/A}
  States stored:        ${states_stored:-N/A}
  States explored:      ${states_explored:-N/A}
  CPU time (ms):        ${cpu_time_ms:-N/A}
  CPU time (s):         ${cpu_time_s:-N/A}
  Virtual memory (MB):  ${virtual_mem_mb:-N/A}
  Virtual memory (GB):  ${virtual_mem_gb:-N/A}
  Resident memory (MB): ${resident_mem_mb:-N/A}
  Resident memory (GB): ${resident_mem_gb:-N/A}
EOF
            fi
            echo "" >> "$SUMMARY_FILE"
        else
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: UPPAAL ---
  No results available

EOF
        fi

        echo "" >> "$SUMMARY_FILE"
    done
done

# Add footer.
cat >> "$SUMMARY_FILE" << EOF

===============================================================================
                           END OF SUMMARY
===============================================================================
Summary generated on: $(date)
Total experiments processed: ${#experiments[@]}
===============================================================================
EOF

echo "Benchmark summary generation completed!"
echo "Summary saved to: $SUMMARY_FILE"
echo ""
echo "Summary statistics:"
echo "  Total experiments: ${#experiments[@]}"
