#!/bin/sh

set -eux

cd "$(dirname "$(realpath "$0")")"
export CC=clang
export CXX=clang++
export LD=ld.lld
export CFLAGS='-fuse-ld=lld'
cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-native \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-march=native' -G Ninja
cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-avx512 \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-march=skylake-avx512 -static' -G Ninja
cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-amd \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-march=znver2' -G Ninja
/usr/lib/emscripten/emcmake cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-wasm \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=' -s ENVIRONMENT=web -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME=MineSweeperSolver' -G Ninja
cmake --build MineSweeperSolver/cmake-build-native
cmake --build MineSweeperSolver/cmake-build-avx512
cmake --build MineSweeperSolver/cmake-build-amd
cmake --build MineSweeperSolver/cmake-build-wasm
