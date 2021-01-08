#!/bin/bash -e
#
# Build zerotape natively
#

if ! command -v cmake &>/dev/null; then
	echo "CMake could not be found"
	exit
fi

if command -v ninja &>/dev/null; then
	GENERATOR="-G Ninja"
	PARALLEL=
else
	GENERATOR=
	PARALLEL="--parallel 5" # wild guess
fi

mkdir -p build/host
cd build/host
cmake $GENERATOR -DHOST_TOOLS_ONLY=YES ../..
cmake --build . $PARALLEL
cd ..
cmake $GENERATOR -DTARGETS_ONLY=YES ..
cmake --build . $PARALLEL
