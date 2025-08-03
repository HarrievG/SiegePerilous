#!/bin/bash
set -e
cd /app
rm -rf build
mkdir build
cd build
cmake /app
make
