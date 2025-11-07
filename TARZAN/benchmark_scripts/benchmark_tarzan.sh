#!/bin/bash

EXECUTABLES_PATH="../../executables/benchmark_executables"
BENCHMARKS_PATH="../benchmarks"
OUTPUT_PATH="../../output/benchmark_tarzan_results"

TOTAL_RUNS="$1"

TIMEOUT="$2"


## and_or_original
#
#num_dirs=$(find "${BENCHMARKS_PATH}/and_or_original/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
#for ((key=0; key<num_dirs; key++)); do
#  ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/and_or_original" "${BENCHMARKS_PATH}/and_or_original/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/and_or_original" "and_or_original" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## boolean
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/boolean/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/boolean" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/boolean" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## csma
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/csma/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/csma" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/csma" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## exSITH
#
#./helpers/sh_single_ta.sh "${EXECUTABLES_PATH}/exSITH" "${BENCHMARKS_PATH}/exSITH/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/exSITH" "exSITH" "${TIMEOUT}"
#
## ---
#
#
## fischer
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/fischer/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/fischer" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/fischer" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## flower
#
#./helpers/sh_single_ta.sh "${EXECUTABLES_PATH}/flower" "${BENCHMARKS_PATH}/flower/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/flower" "flower" "${TIMEOUT}"
#
#
## ---
#
#
## gates
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/gates/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/gates" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/gates" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## latch
#
#num_dirs=$(find "${BENCHMARKS_PATH}/latch/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
#for ((key=0; key<num_dirs; key++)); do
#  ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/latch" "${BENCHMARKS_PATH}/latch/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/latch" "latch" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## lynch
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/lynch/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/lynch" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/lynch" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## maler
#
#num_dirs=$(find "${BENCHMARKS_PATH}/maler/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
#for ((key=0; key<num_dirs; key++)); do
#  ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/maler" "${BENCHMARKS_PATH}/maler/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/maler" "maler" "${TIMEOUT}" "${key}"
#done
#
#
# ---


# medical_workflow

subdirs=()
while IFS= read -r dir; do
    subdirs+=("$dir")
done < <(find "${BENCHMARKS_PATH}/medical_workflow/liana" -mindepth 1 -maxdepth 1 -type d | sort)

num_dirs=${#subdirs[@]}

for ((key=0; key<num_dirs; key++)); do
    current_dir="${subdirs[$key]}"
    folder_name=$(basename "$current_dir")

    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/medical_workflow" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/medical_workflow" "${folder_name}" "${TIMEOUT}" "${key}"
done


# ---


## pagerank
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/pagerank/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/pagerank" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/pagerank" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
## ---
#
#
## rcp
#
#num_dirs=$(find "${BENCHMARKS_PATH}/rcp/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
#for ((key=0; key<num_dirs; key++)); do
#  ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/rcp" "${BENCHMARKS_PATH}/rcp/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/rcp" "rcp" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## ring
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/ring/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/ring" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/ring" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## simple
#
#./helpers/sh_single_ta.sh "${EXECUTABLES_PATH}/simple" "${BENCHMARKS_PATH}/simple/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/simple" "simple" "${TIMEOUT}"
#
#
## ---
#
#
## soldiers
#
#num_dirs=$(find "${BENCHMARKS_PATH}/soldiers/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
#for ((key=0; key<num_dirs; key++)); do
#  ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/soldiers" "${BENCHMARKS_PATH}/soldiers/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/soldiers" "soldiers" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## SRlatch
#
#num_dirs=$(find "${BENCHMARKS_PATH}/SRlatch/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
#for ((key=0; key<num_dirs; key++)); do
#  ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/SRlatch" "${BENCHMARKS_PATH}/SRlatch/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/SRlatch" "SRlatch" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## trainAHV93
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/trainAHV93/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/trainAHV93" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/trainAHV93" "${folder_name}" "${TIMEOUT}" "${key}"
#done
#
#
## ---
#
#
## vikings
#
#subdirs=()
#while IFS= read -r dir; do
#    subdirs+=("$dir")
#done < <(find "${BENCHMARKS_PATH}/vikings/liana" -mindepth 1 -maxdepth 1 -type d | sort)
#
#num_dirs=${#subdirs[@]}
#
#for ((key=0; key<num_dirs; key++)); do
#    current_dir="${subdirs[$key]}"
#    folder_name=$(basename "$current_dir")
#
#    ./helpers/sh_network_ta.sh "${EXECUTABLES_PATH}/vikings" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/vikings" "${folder_name}" "${TIMEOUT}" "${key}"
#done