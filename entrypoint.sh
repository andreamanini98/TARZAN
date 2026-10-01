#!/usr/bin/env bash
set -e

# Valori di default
REPEATS=1
SLEEP_TIME=0
MIN_SLEEP=-1
MAX_SLEEP=-1
LOG_TAG=""
MODE=""
SUFFIX=""
TARGET=""
FILE_OUTPUT=false
FILE_EXT="log"
LOG_DIR_NAME="output_logs"
IGNORE_ERRORS=false
SKIP_EXISTING=false
USE_VTUNE=false
CLEAN_MODE=false
EXTRA_ARGS=()

# Variabile globale per tracciare la durata dell'ultima esecuzione
LAST_RUN_DURATION=0

show_help() {
    echo "================================================================="
    echo "                    TARZAN Docker Entrypoint                     "
    echo "================================================================="
    echo "Usage: $0 <--seq|--omp|--tbb> <file_or_folder_path> [options]"
    echo "       $0 --clean"
    echo ""
    echo "Modes:"
    echo "  --seq               Runs Sequential version (No OpenMP, No TBB)"
    echo "  --omp               Runs OpenMP version"
    echo "  --tbb               Runs TBB + OpenMP version"
    echo "  --clean             Deletes all VTune analysis folders from output_logs/ and exits"
    echo ""
    echo "Benchmark Options:"
    echo "  -r, --repeats NUM   Number of times to repeat each test (default: 1)"
    echo "  -s, --sleep SEC     Fixed sleep time in seconds (default: 0)"
    echo "  --min-sleep SEC     Lower bound for dynamic cooldown based on run time"
    echo "  --max-sleep SEC     Upper bound for dynamic cooldown based on run time"
    echo "  -f, --file-output [EXT] Save outputs to 'output_logs/<base_name>/<log_name>.[EXT]'"
    echo "  -t, --tag STR      Append a tag to log name (e.g., 'dynamic' -> _omp_dynamic.log)"
    echo "  -i, --ignore-errors Continue batch execution on exception/crash"
    echo "  -k, --skip-existing Skip tests that already have an output log"
    echo "  --vtune             Run executable with Intel VTune Profiler (Threading Analysis)"
    echo ""
    echo "Examples:"
    echo "  $0 --omp executables/games_executables/med_app -r 5 --min-sleep 5 --max-sleep 60 -f -t dynamic -k --vtune"
    echo "  $0 --clean"
    echo "================================================================="
}

# Prepara la cartella principale dei log
ROOT_DIR="$(pwd)"
OUTPUT_LOG_DIR="$ROOT_DIR/$LOG_DIR_NAME"

# Parsing completo degli argomenti
while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)
            CLEAN_MODE=true
            shift
            ;;
        --seq)
            MODE="--seq"
            SUFFIX="_seq"
            shift
            ;;
        --omp)
            MODE="--omp"
            SUFFIX="_omp"
            shift
            ;;
        --tbb)
            MODE="--tbb"
            SUFFIX="_tbb"
            shift
            ;;
        -r|--repeats)
            REPEATS="$2"
            shift 2
            ;;
        -s|--sleep)
            SLEEP_TIME="$2"
            shift 2
            ;;
        --min-sleep)
            MIN_SLEEP="$2"
            shift 2
            ;;
        --max-sleep)
            MAX_SLEEP="$2"
            shift 2
            ;;
        -t|--tag)
            LOG_TAG="$2"
            shift 2
            ;;
        -f|--file-output)
            FILE_OUTPUT=true
            if [[ -n "$2" && "$2" != -* && ! -e "$2" ]]; then
                FILE_EXT="$2"
                shift 2
            else
                shift 1
            fi
            ;;
        -i|--ignore-errors)
            IGNORE_ERRORS=true
            shift
            ;;
        -k|--skip-existing)
            SKIP_EXISTING=true
            FILE_OUTPUT=true # Abilita automaticamente l'output su file per la verifica
            if [[ -n "$2" && "$2" != -* && ! -e "$2" ]]; then
                FILE_EXT="$2"
                shift 2
            else
                shift 1
            fi
            ;;
        --vtune)
            USE_VTUNE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            if [ -z "$TARGET" ]; then
                TARGET="$1"
            else
                EXTRA_ARGS+=("$1")
            fi
            shift
            ;;
    esac
done

# LOGICA --clean
if [ "$CLEAN_MODE" = true ]; then
    echo "================================================================="
    echo "[CLEAN] Searching for Intel VTune directories in: $OUTPUT_LOG_DIR"
    echo "================================================================="
    
    if [ ! -d "$OUTPUT_LOG_DIR" ]; then
        echo "[INFO] Directory '$OUTPUT_LOG_DIR' does not exist. Nothing to clean."
        exit 0
    fi

    # Trova sia cartelle visibili (vtune_*) che nascoste (.vtune_*)
    mapfile -t VTUNE_DIRS < <(find "$OUTPUT_LOG_DIR" -type d \( -name "vtune_*" -o -name ".vtune_*" \))

    if [ ${#VTUNE_DIRS[@]} -eq 0 ]; then
        echo "[INFO] No VTune report directories found."
    else
        for dir in "${VTUNE_DIRS[@]}"; do
            echo "[REMOVING] $dir"
            rm -rf "$dir"
        done
        echo "[SUCCESS] All VTune directories removed successfully."
    fi
    exit 0
fi

if [ -z "$MODE" ]; then
    echo "[ERROR] You must specify a mode: --seq, --omp, or --tbb."
    show_help
    exit 1
fi

if [ -z "$TARGET" ]; then
    echo "[ERROR] Target file or directory is missing."
    show_help
    exit 1
fi

calculate_sleep() {
    local duration=$1
    if [ "$MIN_SLEEP" -ge 0 ] || [ "$MAX_SLEEP" -ge 0 ]; then
        local computed_sleep=$duration
        
        if [ "$MIN_SLEEP" -ge 0 ] && [ "$computed_sleep" -lt "$MIN_SLEEP" ]; then
            computed_sleep=$MIN_SLEEP
        fi
        
        if [ "$MAX_SLEEP" -ge 0 ] && [ "$computed_sleep" -gt "$MAX_SLEEP" ]; then
            computed_sleep=$MAX_SLEEP
        fi
        
        echo "$computed_sleep"
    else
        echo "$SLEEP_TIME"
    fi
}

run_single_binary() {
    local input_path="$1"
    shift
    
    local dir_name="$(dirname "$input_path")"
    local base_name="$(basename "$input_path")"

    # Strip dei suffissi base
    base_name="${base_name%_seq}"
    base_name="${base_name%_omp}"
    base_name="${base_name%_tbb}"

    local exec_name="${base_name}${SUFFIX}"
    local log_name="${exec_name}"

    # Applica il Tag
    if [ -n "$LOG_TAG" ]; then
        if [[ "$LOG_TAG" != _* ]]; then
            log_name="${log_name}_${LOG_TAG}"
        else
            log_name="${log_name}${LOG_TAG}"
        fi
    fi

    # Configura la nuova struttura a cartelle per test
    local test_out_dir="$OUTPUT_LOG_DIR/$base_name"
    if [ "$FILE_OUTPUT" = true ] || [ "$USE_VTUNE" = true ]; then
        mkdir -p "$test_out_dir"
    fi

    local log_file_path="$test_out_dir/${log_name}.${FILE_EXT}"
    local old_dir="$(pwd)"

    # CONTROLLO -k / --skip-existing
    if [ "$SKIP_EXISTING" = true ] && [ -s "$log_file_path" ]; then
        echo "================================================================="
        echo "[SKIP] Test '$log_name' already has output log at:"
        echo "       $log_file_path"
        echo "       Skipping execution and cooldown."
        echo "================================================================="
        return 99
    fi

    if [ ! -d "$dir_name" ]; then
        echo "[ERROR] Directory '$dir_name' does not exist."
        return 1
    fi

    cd "$dir_name"

    if [ ! -f "./$exec_name" ]; then
        echo "[ERROR] Executable './$exec_name' not found inside $(pwd)"
        cd "$old_dir"
        return 1
    fi

    echo "================================================================="
    echo "[RUN] Executable:    $exec_name"
    echo "[RUN] Directory:     $(pwd)"
    echo "[RUN] Mode:          $MODE"
    echo "[RUN] Repeats:       $REPEATS"
    if [ "$USE_VTUNE" = true ]; then
        echo "[RUN] Profiler:      Intel VTune (Threading Analysis)"
    fi
    if [ "$MIN_SLEEP" -ge 0 ] || [ "$MAX_SLEEP" -ge 0 ]; then
        echo "[RUN] Dynamic Sleep: Clamp [${MIN_SLEEP}s, ${MAX_SLEEP}s]"
    else
        echo "[RUN] Fixed Sleep:   ${SLEEP_TIME}s"
    fi
    echo "[RUN] Ignore Errors: $IGNORE_ERRORS"
    if [ "$FILE_OUTPUT" = true ]; then
        echo "[RUN] Log File:      $log_file_path"
        > "$log_file_path"
    fi
    echo "================================================================="

    for (( i=1; i<=REPEATS; i++ )); do
        echo "-----------------------------------------------------------------"
        echo "[BENCHMARK] Iteration $i of $REPEATS for $exec_name"
        echo "-----------------------------------------------------------------"

        local exit_code=0
        local start_time=$SECONDS

        # Costruisce il comando di esecuzione
        local cmd=("./$exec_name" "$@")
        if [ "$USE_VTUNE" = true ]; then
            local vtune_dir="$test_out_dir/vtune_${log_name}_iter_${i}"
            rm -rf "$vtune_dir"
            cmd=(vtune -collect threading -result-dir "$vtune_dir" -- "${cmd[@]}")
        fi

        # Esecuzione
        set +e
        if [ "$FILE_OUTPUT" = true ]; then
            {
                echo "=== ITERATION $i / $REPEATS ==="
                "${cmd[@]}"
                exit_code=$?
                echo "[EXIT CODE]: $exit_code"
                echo ""
            } >> "$log_file_path" 2>&1
        else
            "${cmd[@]}"
            exit_code=$?
        fi

        # Calcolo durata
        local end_time=$SECONDS
        LAST_RUN_DURATION=$((end_time - start_time))

        if [ $exit_code -ne 0 ] && [ "$IGNORE_ERRORS" = false ]; then
            set -e
            echo "[ERROR] $exec_name failed with exit code $exit_code. Stopping."
            cd "$old_dir"
            return $exit_code
        fi
        set -e

        if [ $exit_code -ne 0 ]; then
            echo "[WARNING] $exec_name exited with status $exit_code (Ignored)"
        fi

        # Cooldown tra le iterazioni
        if [ $i -lt $REPEATS ]; then
            sleep_to_do=$(calculate_sleep "$LAST_RUN_DURATION")
            if [ "$sleep_to_do" -gt 0 ]; then
                echo "[COOLING] Run took ${LAST_RUN_DURATION}s. Sleeping for ${sleep_to_do}s..."
                sleep "$sleep_to_do"
            fi
        fi
    done

    cd "$old_dir"
    return 0
}

# --- LOGICA PRINCIPALE ---

if [ -f "$TARGET" ] || [ -f "${TARGET}_seq" ] || [ -f "${TARGET}_omp" ] || [ -f "${TARGET}_tbb" ]; then
    run_single_binary "$TARGET" "${EXTRA_ARGS[@]}"

elif [ -d "$TARGET" ]; then
    echo "[INFO] Starting batch folder execution on: $TARGET"
    echo "-----------------------------------------------------------------"

    mapfile -t TARGET_EXECS < <(find "$TARGET" -maxdepth 1 -type f -executable | sed -E 's/_(seq|omp|tbb)$//' | sort -u)

    if [ ${#TARGET_EXECS[@]} -eq 0 ]; then
        echo "[WARNING] No executable files found in directory '$TARGET'."
        exit 0
    fi

    for exec_path in "${TARGET_EXECS[@]}"; do
        [ -z "$exec_path" ] && continue
        
        set +e
        run_single_binary "$exec_path" "${EXTRA_ARGS[@]}"
        run_status=$?
        set -e

        if [ $run_status -ne 0 ] && [ $run_status -ne 99 ] && [ "$IGNORE_ERRORS" = false ]; then
            echo "[ERROR] Aborting batch execution due to error in $exec_path"
            exit $run_status
        fi

        if [ $run_status -ne 99 ]; then
            sleep_to_do=$(calculate_sleep "$LAST_RUN_DURATION")
            if [ "$sleep_to_do" -gt 0 ]; then
                echo "[COOLING] Pause between different targets for ${sleep_to_do}s..."
                sleep "$sleep_to_do"
            fi
        fi
    done
    echo "[SUCCESS] Batch folder benchmark completed."
    if [ "$FILE_OUTPUT" = true ]; then
        echo "[INFO] All logs and VTune reports are stored in: $OUTPUT_LOG_DIR"
    fi

else
    echo "[ERROR] '$TARGET' is neither a valid file nor a directory."
    exit 1
fi
