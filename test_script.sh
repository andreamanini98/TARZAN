#!/bin/bash

run_all_tests_in_dir() {
    local source_dir="$1"
    local base_output_dir="$2"
    local variant_name="$3"

    # Controllo argomenti
    if [ -z "$source_dir" ] || [ -z "$base_output_dir" ] || [ -z "$variant_name" ]; then
        echo "Errore: Mancano degli argomenti."
        echo "Uso: run_all_tests_in_dir <source_dir> <base_output_dir> <variant_name>"
        return 1
    fi

    local start_dir
    start_dir="$(pwd)"

    for cpp_file in "$source_dir"/*.cpp; do
        [ -e "$cpp_file" ] || continue

        local exec_name
        exec_name="$(basename "$cpp_file" .cpp)"

        local test_dir="${base_output_dir}/${exec_name}"
        mkdir -p "$test_dir"

        local log_file="${start_dir}/${test_dir}/${variant_name}.txt"

        > "$log_file"

        cd "executables/games_executables/med_app" || exit 1

        echo "=================================================="
        echo "Test: $exec_name | Variante: $variant_name"
        echo "Output in: $log_file"
        echo "=================================================="

        for i in {1..5}; do
            echo "--- Esecuzione $i per $exec_name ---" | tee -a "$log_file"

            if [ -x "./$exec_name" ]; then
                "./$exec_name" 2>&1 | tee -a "$log_file"
            else
                echo "Errore: Eseguibile ./$exec_name non trovato." | tee -a "$log_file"
            fi

            sleep 10
        done

        cd "$start_dir" || exit 1
    done
}

SOURCE_DIR="/home/andrea/projects/TARZAN/TARZAN/benchmarks_games/games_executables/med_app"
RESULTS_DIR="res"

run_all_tests_in_dir "$SOURCE_DIR" "$RESULTS_DIR" "tbb_46aab9e"

# run_all_tests_in_dir "$SOURCE_DIR" "$RESULTS_DIR" "var2"
