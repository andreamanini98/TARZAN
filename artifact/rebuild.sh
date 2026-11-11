#!/bin/bash
# Rebuild TARZAN executables with a custom PARALLEL_THRESHOLD.
# Usage: rebuild.sh <parallel_threshold> [jobs]   (jobs defaults to nproc)
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

THRESHOLD=$1
JOBS=${2:-$(nproc)}

if ! [[ "$THRESHOLD" =~ ^[0-9]+$ ]] || ! [[ "$JOBS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Usage: $0 <parallel_threshold> [jobs]"
    echo "  parallel_threshold  non-negative integer"
    echo "  jobs                positive integer (default: nproc)"
    exit 1
fi

echo "=================================================="
echo " Rebuilding with PARALLEL_THRESHOLD=$THRESHOLD (jobs: $JOBS)"
echo "=================================================="

mkdir -p "$ROOT_DIR/build"
cd "$ROOT_DIR/build"
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=ON \
      -DENABLE_APPIMAGE=ON \
      -DENABLE_OPTIMIZATION=ON \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DPARALLEL_THRESHOLD="$THRESHOLD" \
      ..
make -j"$JOBS"
