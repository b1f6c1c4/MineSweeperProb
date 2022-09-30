#!/bin/sh

set -eux

export CC=clang
export CXX=clang++
export LD=ld.lld
export CFLAGS='-fuse-ld=lld'
cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-native \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-march=native' -G Ninja
cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-avx512 \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-march=skylake-avx512 -static' -G Ninja
cmake -S MineSweeperSolver -B MineSweeperSolver/cmake-build-amd \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-march=znver2 -static' -G Ninja
cmake --build MineSweeperSolver/cmake-build-native
cmake --build MineSweeperSolver/cmake-build-avx512
cmake --build MineSweeperSolver/cmake-build-amd
