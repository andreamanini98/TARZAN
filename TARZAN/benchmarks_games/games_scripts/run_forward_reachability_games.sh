#!/bin/bash

# Script to generate the benchmark summary for forward reachability games in TARZAN.

echo "Benchmark Summary for forward reachability games is running."

./run_forward_reach_games_tests.sh 5 production_cell_hybrid
./run_forward_reach_games_tests.sh 5 shopping_stocks

echo ""
echo ""

./generate_forward_reach_benchmark.sh

PROJECT_ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"

echo "Benchmark Summary saved in folder $PROJECT_ROOT/output/forward_reachability_benchmark!"
echo ""
echo "Stopping the benchmark generation"
