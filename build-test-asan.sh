#!/bin/sh
set -xe

# Tests with ASan/LeakSan/UBSan
cmake -S. -Bbuild-test-asan \
      -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/clang.toolchain.cmake \
      -D CMAKE_BUILD_TYPE=Asan
(cd build-test-asan && make && ctest)
