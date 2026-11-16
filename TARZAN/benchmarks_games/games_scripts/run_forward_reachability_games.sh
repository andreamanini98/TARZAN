#!/bin/bash

# Script to generate the benchmark summary for forward reachability games in TARZAN.

echo "Benchmark Summary for forward reachability games is running."

./run_forward_reach_games_tests.sh 5 production_cell_hybrid
./run_forward_reach_games_tests.sh 5 shopping_stocks

echo ""
echo ""

PROJECT_ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"

rm -rf "$PROJECT_ROOT/output/forward_reachability_benchmark"
mkdir "$PROJECT_ROOT/output/forward_reachability_benchmark"

./generate_forward_reach_benchmark.sh

echo ""
echo "Benchmark Summary saved in folder $PROJECT_ROOT/output/forward_reachability_benchmark!"
echo ""
echo "Stopping the benchmark generation"

echo "Producing the LaTeX Tables to represent the Result from Forward Reachability Games"
echo ""

python3 create_latex_tables_from_benchmark.py

echo ""
echo "Benchmark Summary Tables created!"
