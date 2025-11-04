# Clear benchmark output directories.
rm -rf ../../output/benchmark_tarzan_results/*
rm -rf ../../output/benchmark_tchecker_results/*
rm -rf ../../output/benchmark_uppaal_results/*
rm -rf ../../output/latex_results/*

# Delete benchmark summary file.
rm -f ../../output/benchmark_summary.txt

NUM_RUNS="$1"
TIMEOUT="$2"

echo "Starting uppaal benchmarking"

./benchmark_uppaal.sh /Users/echo/Desktop/PoliPrograms/UPPAAL-5.0.0.app/Contents/Resources/uppaal/bin/verifyta "${NUM_RUNS}" "${TIMEOUT}"

echo "Starting tChecker benchmarking"

./benchmark_tchecker.sh /Users/echo/Desktop/PoliPrograms/tchecker-0.8/bin/tck-reach "${NUM_RUNS}" "${TIMEOUT}"

echo "Starting tarzan benchmarking"

./benchmark_tarzan.sh "${NUM_RUNS}" "${TIMEOUT}"

echo "Generating summary"

./generate_benchmark_summary.sh

echo "Generating latex tables"

python3 generate_latex_tables.py

echo "Benchmarking complete"