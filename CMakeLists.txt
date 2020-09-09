cmake_minimum_required(VERSION 3.18)

project(zerotape VERSION 0.0.1 DESCRIPTION "zerotape" LANGUAGES C)
option(USE_FORTIFY "Use Fortify" OFF)

add_executable(lemon libraries/lemon/lemon.c)
add_subdirectory(libraries/fortify)
add_subdirectory(libraries/zerotape)
add_subdirectory(platform/generic)
