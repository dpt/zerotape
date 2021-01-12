# CMakeLists.txt
#
# zerotape
#
# Copyright (c) David Thomas, 2020-2021
#

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(zerotape VERSION 0.4.1 DESCRIPTION "Serialisation library" LANGUAGES C)

# If a host tools build was requested, then build them alone
if(HOST_TOOLS_ONLY)
    message(STATUS "zerotape: Building host tools only")
    add_subdirectory(libraries/lemon)
    return()
endif()

option(USE_FORTIFY "Use Fortify" OFF)

# Where -Oz is supported, use it instead of -Os (smaller)
if(APPLE AND CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(CMAKE_C_FLAGS_MINSIZEREL "-Oz -DNDEBUG")
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "zerotape: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Build the native 'host' tools (the lemon parser generator) in this dir
set(HOSTTOOLSDIR "${CMAKE_BINARY_DIR}/host")

# Create a directory in which to build the host tools
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${HOSTTOOLSDIR}")

# Generate the host tools build
#
# We use -DCMAKE_C_COMPILER to force the use of the system compiler, otherwise
# the child build may inherit a cross-compiler via the CC environment variable.
# This is bound to break on Windows.
execute_process(COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    -DCMAKE_BUILD_TYPE='Release'
    -DCMAKE_C_COMPILER='cc'
    -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -DHOST_TOOLS_ONLY=1
    ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY "${HOSTTOOLSDIR}")

# Build the host tools
execute_process(COMMAND ${CMAKE_COMMAND} --build . --target lemon WORKING_DIRECTORY "${HOSTTOOLSDIR}")

# Build zerotape itself
message(STATUS "zerotape: Building targets only")
add_subdirectory(libraries/fortify)
add_subdirectory(libraries/zerotape)
add_subdirectory(platform/generic)
