# Clear benchmark output directories.
rm -rf ../../output/benchmark_tarzan_results/*
rm -rf ../../output/benchmark_tchecker_results/*
rm -rf ../../output/benchmark_uppaal_results/*
rm -rf ../../output/latex_results/*

# Delete benchmark summary file.
rm -f ../../output/benchmark_summary.txt

NUM_RUNS="$1"
TIMEOUT="$2"

./benchmark_uppaal.sh /Users/echo/Desktop/PoliPrograms/UPPAAL-5.0.0.app/Contents/Resources/uppaal/bin/verifyta "${NUM_RUNS}" "${TIMEOUT}"

./benchmark_tchecker.sh /Users/echo/Desktop/PoliPrograms/tchecker-0.8/bin/tck-reach "${NUM_RUNS}" "${TIMEOUT}"

./benchmark_tarzan.sh "${NUM_RUNS}" "${TIMEOUT}"

./generate_benchmark_summary.sh

python3 generate_latex_tables.py