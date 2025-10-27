#!/bin/bash

EXECUTABLES_PATH="../../executables/benchmark_executables"
BENCHMARKS_PATH="../benchmarks"
TOTAL_RUNS=5
OUTPUT_PATH="../../output/benchmark_results"
TIMEOUT=100


# and_or_original

num_dirs=$(find "${BENCHMARKS_PATH}/and_or_original/liana" -mindepth 1 -maxdepth 1 -type d | wc -l)
for ((key=0; key<num_dirs; key++)); do
  ./sh_network_ta.sh "${EXECUTABLES_PATH}/and_or_original" "${BENCHMARKS_PATH}/and_or_original/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/and_or_original" "and_or_original" "${TIMEOUT}" "${key}"
done


# ---


# boolean

subdirs=()
while IFS= read -r dir; do
    subdirs+=("$dir")
done < <(find "${BENCHMARKS_PATH}/boolean/liana" -mindepth 1 -maxdepth 1 -type d | sort)

num_dirs=${#subdirs[@]}

for ((key=0; key<num_dirs; key++)); do
    current_dir="${subdirs[$key]}"
    folder_name=$(basename "$current_dir")

    ./sh_network_ta.sh "${EXECUTABLES_PATH}/boolean" "${current_dir}" "${TOTAL_RUNS}" "${OUTPUT_PATH}/boolean" "${folder_name}" "${TIMEOUT}" "${key}"
done


# ---


#flower

./sh_single_ta.sh "${EXECUTABLES_PATH}/flower" "${BENCHMARKS_PATH}/flower/liana" "${TOTAL_RUNS}" "${OUTPUT_PATH}/flower" "flower" "${TIMEOUT}"

# TODO: per come è strutturato il file sh_network_ta.sh, se una directory  contiene più directory le fa passare tutte (se data come benchmark_path),
#       altrimenti considera solo i file all'interno di quella specifica cartella (linee 104-114) [questo è solo un reminder].
#       and_or_original per processare ogni subdirectory automaticamente (magari ti può servire se scarti la chiave)
#       boolean devi passare le directory a mano (utile per test di scalabilità con chiave)


