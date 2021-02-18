# CMakeLists.txt
#
# zerotape
#
# Copyright (c) David Thomas, 2020-2021
#
# vim: sw=4 ts=8 et

add_library(zerotape)

set_target_properties(zerotape PROPERTIES
    VERSION 0.5.0
    DESCRIPTION "Serialisation library"
    PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/zerotape/zerotape.h
    C_STANDARD 90)

target_include_directories(zerotape PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(zerotape PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}) # generated sources need a specific include path setting

# Header (so it appears in Xcode)
target_sources(zerotape PRIVATE ${CMAKE_SOURCE_DIR}/include/zerotape/zerotape.h)
# Ordinary sources
target_sources(zerotape PRIVATE zt-ast-viz.c zt-ast.c zt-ast.h zt-gramx.h zt-lex-impl.h zt-lex-test.c zt-lex-test.h zt-lex.c zt-lex.h zt-load.c zt-driver.c zt-driver.h zt-run.c zt-run.h zt-save.c zt-walk.c zt-walk.h zt-slab-alloc.c zt-slab-alloc.h) # add regular sources
# Generated sources
target_sources(zerotape PRIVATE zt-gram.c zt-gram.h)

if(MSVC)
    target_compile_options(zerotape PRIVATE /W4 /WX)
else()
    target_compile_options(zerotape PRIVATE
        -Wall -Wextra -pedantic -Wno-unused-parameter)
    set_source_files_properties(zt-gram.c PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-variable")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(zerotape PRIVATE ZTAST_LOG ZT_DEBUG)
endif()

option(ZT_USE_HEX "Use hex for saves, or decimal otherwise" ON)
if (ZT_USE_HEX)
    target_compile_definitions(zerotape PRIVATE ZT_USE_HEX)
endif()

set(LEMON_SRC ${CMAKE_SOURCE_DIR}/libraries/lemon)
find_program(LEMON_EXE
    lemon
    PATHS ${HOSTTOOLSDIR}/libraries/lemon
    PATH_SUFFIXES Debug Release RelWithDebInfo MinSizeRel
    NO_DEFAULT_PATH
    REQUIRED)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.c ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.h
    COMMAND ${LEMON_EXE} -s -d${CMAKE_CURRENT_BINARY_DIR} -T${LEMON_SRC}/lempar.c ${CMAKE_CURRENT_SOURCE_DIR}/zt-gram.y
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/zt-gram.y)
add_custom_target(ztparser ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.c ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.h)

add_dependencies(zerotape ztparser)

if(USE_FORTIFY)
    target_compile_definitions(zerotape PRIVATE FORTIFY)
    target_link_libraries(zerotape PUBLIC Fortify)
endif()

install(TARGETS zerotape
    LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
