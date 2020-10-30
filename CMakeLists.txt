# CMakeLists.txt
#
# zerotape
#
# Copyright (c) David Thomas, 2020
#

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(zerotape VERSION 1.0.0 DESCRIPTION "zerotape" LANGUAGES C)

option(USE_FORTIFY "Use Fortify" OFF)

if(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    if(APPLE)
        # Use -Oz instead of -Os
        set(CMAKE_C_FLAGS_MINSIZEREL "-Oz -DNDEBUG")
    endif()
endif()

add_subdirectory(libraries/fortify)
add_subdirectory(libraries/lemon)
add_subdirectory(libraries/zerotape)
add_subdirectory(platform/generic)
