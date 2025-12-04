# Clear benchmark output directories.
rm -rf ../../output/benchmark_tarzan_results/*
rm -rf ../../output/benchmark_tchecker_results/*
rm -rf ../../output/benchmark_uppaal_results/*
rm -rf ../../output/latex_results/*

# Delete benchmark summary file.
rm -f ../../output/benchmark_summary.txt

NUM_RUNS="$1"
TIMEOUT="$2"

UPPAAL_PATH="$3"
# /Users/echo/Desktop/PoliPrograms/UPPAAL-5.0.0.app/Contents/Resources/uppaal/bin/verifyta

TCHECKER_PATH="$4"
# /Users/echo/Desktop/PoliPrograms/tchecker-0.8/bin/tck-reach

echo "Starting uppaal benchmarking"

./benchmark_uppaal.sh "${UPPAAL_PATH}" "${NUM_RUNS}" "${TIMEOUT}"

echo "Starting tChecker benchmarking"

./benchmark_tchecker.sh "${TCHECKER_PATH}" "${NUM_RUNS}" "${TIMEOUT}"

echo "Starting tarzan benchmarking"

./benchmark_tarzan.sh "${NUM_RUNS}" "${TIMEOUT}"

echo "Generating summary"

./generate_benchmark_summary.sh

echo "Generating latex tables"

python3 generate_latex_tables.py

echo "Benchmarking complete"