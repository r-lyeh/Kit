#!/usr/bin/env bash
# build.sh:  One-shot configure + build for Linux / macOS
set -e

BUILD_DIR="${1:-build}"
BUILD_TYPE="${2:-Release}"

echo "==> Configuring (build_type=$BUILD_TYPE, dir=$BUILD_DIR)"
cmake -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DFETCH_SDL3=ON

echo "==> Building"
cmake --build "$BUILD_DIR" --parallel "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "✓  Done.  Run:  ./$BUILD_DIR/sdl3_gltf_starter"
