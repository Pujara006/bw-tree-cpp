#!/bin/bash

rm -rf build

mkdir build
cd build

cmake -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Wconversion -Wshadow" ..
cmake --build .