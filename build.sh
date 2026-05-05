#!/bin/bash

# Go to project root (just in case)
cd "$(dirname "$0")"

# Clean old build
rm -rf build

# Create build directory
mkdir build
cd build

# Run CMake
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt)

# Build
make

# Run program
./DFBMS