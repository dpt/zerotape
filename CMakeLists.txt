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

project(zerotape VERSION 0.4.1 DESCRIPTION "zerotape" LANGUAGES C)

option(USE_FORTIFY "Use Fortify" OFF)

if(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    if(APPLE)
        # Use -Oz instead of -Os
        set(CMAKE_C_FLAGS_MINSIZEREL "-Oz -DNDEBUG")
    endif()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

if(HOST_TOOLS_ONLY)
    message(STATUS "Building host tools only")
    add_subdirectory(libraries/lemon)
elseif(TARGETS_ONLY)
    message(STATUS "Building targets only")
    add_subdirectory(libraries/fortify)
    add_subdirectory(libraries/zerotape)
    add_subdirectory(platform/generic)
else()
    message(FATAL_ERROR "Must choose HOST_TOOLS_ONLY or TARGETS_ONLY")
endif()
