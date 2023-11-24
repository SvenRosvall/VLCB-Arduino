#!/bin/sh
set -xe

# Tests
cmake -S. -Bbuild-test \
      -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/host.toolchain.cmake \
      -D CMAKE_C_COMPILER=clang \
      -D CMAKE_CXX_COMPILER=clang++ \
      -D CMAKE_BUILD_TYPE=Debug
(cd build-test && make testAll && ctest)

# AVR binary
cmake -S. -Bbuild \
      -D ARDUINO_PORT=/dev/ttyUSB0 \
      -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/uno.toolchain.cmake \
      -D CMAKE_BUILD_TYPE=MinSizeRel
(cd build && make VLCB_1in1out)
