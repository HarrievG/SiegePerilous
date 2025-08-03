#!/bin/bash
# This script automates the build process for the Siege Perilous project.
# It should be run from the root of the project directory.

# Exit immediately if a command exits with a non-zero status.
set -e

# --- 1. Setup Build Directory ---
echo "--- Setting up build directory ---"
# Remove the old build directory for a clean build, then create a new one.
rm -rf build
mkdir build

# --- 2. Configure with CMake ---
echo "--- Configuring project with CMake ---"
# Enter the build directory and run cmake.
# '..' tells CMake to look for the CMakeLists.txt file in the parent directory (the project root).
cd build
cmake ..

# --- 3. Build the Project ---
echo "--- Building the project ---"
cmake --build .

echo ""
echo "--- Build complete! ---"
echo "The executable is located at: build/SiegePerilous"
echo "To run it from the project root, use: ./build/SiegePerilous"
