#!/bin/bash

EXECUTABLES_PATH="../../executables/benchmark_executables"
BENCHMARKS_PATH="../benchmarks"
TOTAL_RUNS=5
OUTPUT_PATH="../../output/benchmark_results"
TIMEOUT=100


# and_or_original

num_dirs=$(find "${BENCHMARKS_PATH}/and_or_original/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)

for ((key=0; key<num_dirs; key++)); do
  ./sh_network_ta.sh "${EXECUTABLES_PATH}/and_or_original" "${BENCHMARKS_PATH}/and_or_original/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}" "and_or_original" "${TIMEOUT}" "${key}"
done


# ---


#flower

# ./sh_single_ta.sh "${EXECUTABLES_PATH}/flower" "${BENCHMARKS_PATH}/flower/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}" "flower" "${TIMEOUT}"


