#!/bin/bash
set -e

# Risale alla root del repository
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

ENTRYPOINT="$ROOT_DIR/entrypoint.sh"
GAMES_DIR="executables/games_executables"

echo $ENTRYPOINT

MODE=${1:-"kick-the-tires"}

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
      "$ENTRYPOINT" "$var" "$GAMES_DIR/autoclave_sterilization_system" -f -i
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
            "$ENTRYPOINT" "$var" "$MODEL_PATH" -f -i -r 10 --min-sleep 1 --max-sleep 10
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

else
    "$ENTRYPOINT" "$@"
fi
