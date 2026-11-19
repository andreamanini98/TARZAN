#!/bin/bash

# Script to generate the benchmark summary for forward reachability games in TARZAN.

PROJECT_ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUTPUT_DIR="$PROJECT_ROOT/output"
SUMMARY_FILE="$OUTPUT_DIR/forward_reachability_benchmark/benchmark_results_summary.txt"

# Check if output directories exist.
TARZAN_DIR="$OUTPUT_DIR/games_tarzan_results"
#TARZAN_DIR_1="$OUTPUT_DIR/games_tarzan_results/production_cell_hybrid"
#TARZAN_DIR_2="$OUTPUT_DIR/games_tarzan_results/shopping_stocks"

if [[ ! -d "$TARZAN_DIR" ]]; then
    echo "Error: No games result directories found in $OUTPUT_DIR/games_tarzan_results"
    exit 1
fi

echo "Generating benchmark summary..."
echo "Output file: $SUMMARY_FILE"

# Create summary file header.
cat > "$SUMMARY_FILE" << 'EOF'
===============================================================================================================
                     BENCHMARK RESULTS SUMMARY FOR FORWARD REACHABILITY GAMES
===============================================================================================================
This file contains a comprehensive summary of benchmark results from a single verification tool, which is TARZAN.

Results are organized by:
  - Experiment
  - Sub-experiment (specific instance)
===============================================================================

EOF

# Get all unique benchmark names (experiments) from any of the three directories.
experiments=()
for dir in "$TARZAN_DIR"; do
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

    for dir in "$TARZAN_DIR/$experiment"; do
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
                num_regions_setPhi=$(grep "Total regions in setPhi:" "$tarzan_file" | awk '{print $NF}')
                num_regions_setG=$(grep "Total stored regions:" "$tarzan_file" | awk '{print $NF}')
                num_regions_setG_forward=$(grep "Total regions in setG:" "$tarzan_file" | awk '{print $NF}')
                num_regions_start=$(grep "Total starting regions:" "$tarzan_file" | awk '{print $NF}')
                num_regions_setC=$(grep "Total stored regions (setC):" "$tarzan_file" | awk '{print $NF}')
                num_regions_setW=$(grep "Total stored regions (setW):" "$tarzan_file" | awk '{print $NF}')
                num_regions_setC_and_W=$(grep "Total stored regions (setW + setC):" "$tarzan_file" | awk '{print $NF}')
                total_time=$(grep "Total time:" "$tarzan_file" | awk '{print $(NF-1), $NF}')
                num_iterations=$(grep "Total iterations:" "$tarzan_file" | awk '{print $NF}')
                goal_status=$(grep -E '(VICTORY|LOSE)' "$tarzan_file" | awk '{print $NF}')
                peak_memory_mb=$(grep "Peak Memory (RSS):" "$tarzan_file" | awk '{print $4, $5}')
                #peak_memory_bytes=$(grep "Peak Memory (bytes):" "$tarzan_file" | awk '{print $NF}')
                exec_time=$(grep "Total time including region generation:" "$tarzan_file" | awk '{print $(NF-1), $NF}')
                #status=$(grep "Status:" "$tarzan_file" | sed 's/Status: //')

                cat >> "$SUMMARY_FILE" << EOF
  Number of starting regions:    ${num_regions_start:-N/A}
  Number of regions (Sphi):    ${num_regions_setPhi:-N/A}
  Number of regions (G):    ${num_regions_setG:-${num_regions_setG_forward:-N/A}}
  Number of regions (C):    ${num_regions_setC:-N/A}
  Number of regions (W):    ${num_regions_setW:-N/A}
  Number of regions (C + W):    ${num_regions_setC_and_W:-N/A}
  Total Computed Regions:       ${num_regions_setG:-${num_regions_setC_and_W:-N/A}}
  Total time:           ${total_time:-N/A}
  Num iterations:     ${num_iterations:-N/A}
  Goal status:          ${goal_status:-N/A}
  Peak memory:          ${peak_memory_mb:-N/A}
  Total exec time:      ${exec_time:-N/A}
EOF
            fi
            echo "" >> "$SUMMARY_FILE"
        else
            cat >> "$SUMMARY_FILE" << EOF
--- TOOL: TARZAN ---
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
