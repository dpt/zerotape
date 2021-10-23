#!/bin/bash
#
# Build zerotape for RISC OS via GCCSDK
#

set -e
set -o pipefail

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -
cmake --version || { apt-get update && \
                     apt-get install -y software-properties-common && \
                     apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' && \
                     apt-get install -y cmake ; }

ninja --version || { apt-get update && \
                     apt-get install -y ninja-build ; }

source /home/riscos/gccsdk-params

# zerotape
mkdir -p build-ro && cd build-ro
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/riscos.cmake .. || bash -i
cmake --build . || bash -i
