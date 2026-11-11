#!/bin/bash
set -e

# Risale alla root del repository
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

ENTRYPOINT="$ROOT_DIR/entrypoint.sh"
GAMES_DIR="executables/games_executables"

echo $ENTRYPOINT

# Optional: "rebuild <threshold> [jobs] [mode ...]" recompiles first, then runs mode (if any).
if [ "$1" = "rebuild" ]; then
    shift
    if [[ "$2" =~ ^[0-9]+$ ]]; then
        "$SCRIPT_DIR/rebuild.sh" "$1" "$2"
        shift 2
    else
        "$SCRIPT_DIR/rebuild.sh" "$1"
        shift
    fi
    [ $# -eq 0 ] && exit 0
fi

MODE=${1:-"kick-the-tires"}
# Any flags after the mode (e.g. -t thr50, -r 3, -k) are forwarded to entrypoint.sh.
EXTRA_ARGS=("${@:2}")

if [ ! -f "$ENTRYPOINT" ]; then
    echo "[ERROR] Impossibile trovare l'entrypoint originale in: $ENTRYPOINT"
    exit 1
fi

chmod +x "$ENTRYPOINT"

# Array delle varianti di parallelismo da testare per ciascun modello
VARIANTS=(
          "--seq" 
          "--omp" 
          "--tbb"
        )

if [ "$MODE" = "kick-the-tires" ]; then
    echo "=================================================="
    echo " Running Quick Verification (Kick-the-tires) "
    echo "=================================================="
    for var in "${VARIANTS[@]}"; do
      echo "--> Testing variant $var on autoclave_sterilization_system..."
      "$ENTRYPOINT" "$var" "$GAMES_DIR/autoclave_sterilization_system" -f -i "${EXTRA_ARGS[@]}"
    done

    echo "=================================================="
    echo " Generating aggregated reports... "
    echo "=================================================="
    python3 "$ROOT_DIR/scripts/analyze.py" "autoclave_sterilization_system" -f both || true


  elif [ "$MODE" = "full" ]; then
    echo "=================================================="
    echo " Running Full Reproduction for All Models "
    echo "=================================================="
    
    MODELLI=(
        "autoclave_sterilization_system"
        "synth_reactor"
        "production_cell"
        "med_app"
    )

    for model in "${MODELLI[@]}"; do
        MODEL_PATH="$GAMES_DIR/$model"
        echo "=================================================="
        echo " Processing Model: $model ($MODEL_PATH)"
        echo "=================================================="

        for var in "${VARIANTS[@]}"; do
            echo "--> Running mode $var for model $model..."
            "$ENTRYPOINT" "$var" "$MODEL_PATH" -f -i -r 10 --min-sleep 1 --max-sleep 10 "${EXTRA_ARGS[@]}"
          done
        done

        echo "=================================================="
        echo " Generating aggregated reports... "
        echo "=================================================="
        for model in "${MODELLI[@]}"; do
          python3 "$ROOT_DIR/scripts/analyze.py" "$model" -f both || true
        done

    echo "=================================================="
    echo " All benchmarks completed successfully! "
    echo "=================================================="

elif [ "$MODE" = "paper-logs" ]; then
    # paper-logs            -> list the CSV/PNG reference files
    # paper-logs <file.csv> -> pretty-print a CSV (path relative to paper_output_logs/)
    # paper-logs <file>     -> print any other text file
    PAPER_DIR="$ROOT_DIR/paper_output_logs"
    FILE="${EXTRA_ARGS[0]}"
    if [ -z "$FILE" ]; then
        (cd "$PAPER_DIR" && find . -type f \( -name "*.csv" -o -name "*.png" \) | sed 's|^\./||' | sort)
    elif [ ! -f "$PAPER_DIR/$FILE" ]; then
        echo "[ERROR] '$FILE' not found in paper_output_logs/"
        exit 1
    elif [[ "$FILE" == *.csv ]]; then
        python3 -c '
import csv, sys
rows = list(csv.reader(open(sys.argv[1])))
w = [max(len(r[i]) for r in rows if i < len(r)) for i in range(len(rows[0]))]
for r in rows: print("  ".join(c.rjust(w[i]) for i, c in enumerate(r)))
' "$PAPER_DIR/$FILE"
    else
        cat "$PAPER_DIR/$FILE"
    fi

else
    "$ENTRYPOINT" "$@"
fi
