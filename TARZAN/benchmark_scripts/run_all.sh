#!/bin/bash

EXECUTABLES_PATH="../../executables/benchmark_executables"
BENCHMARKS_PATH="../benchmarks"
TOTAL_RUNS=5
OUTPUT_PATH="../../output/benchmark_results"
TIMEOUT=100


./sh_single_ta.sh "${EXECUTABLES_PATH}/flower" "${BENCHMARKS_PATH}/flower/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}" "flower" "${TIMEOUT}"