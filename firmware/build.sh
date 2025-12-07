#!/bin/bash
set -euo pipefail

# Build script for robot firmware
# Runs inside Docker container with ARM toolchain

BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"

echo "=========================================="
echo "Building Robot Firmware"
echo "Build Type: $BUILD_TYPE"
echo "=========================================="

# Create and enter build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
make -j$(nproc) VERBOSE=1

echo "=========================================="
echo "Build complete!"
echo "Output: $BUILD_DIR/robot_firmware.elf"
echo "=========================================="
