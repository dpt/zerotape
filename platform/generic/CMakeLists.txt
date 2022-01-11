# CMakeLists.txt
#
# zerotape
#
# Copyright (c) David Thomas, 2020-2021
#
# vim: sw=4 ts=8 et

set(APPS_DIR ../../apps)

add_executable(zerotape-tests ${APPS_DIR}/zerotape-tests/tests.c)
target_link_libraries(zerotape-tests zerotape)
if (MSVC)
    target_compile_options(zerotape-tests PRIVATE
        /W3)
else()
    target_compile_options(zerotape-tests PRIVATE
        -Wall -Wextra -pedantic -Wno-unused-parameter)
endif()
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(zerotape-tests PRIVATE ZT_DEBUG)
endif()
if(USE_FORTIFY)
    target_link_libraries(zerotape-tests Fortify)
    target_compile_definitions(zerotape-tests PRIVATE FORTIFY)
endif()

add_executable(zerotape-demo ${APPS_DIR}/zerotape-demo/demo.c)
target_link_libraries(zerotape-demo zerotape)
if (MSVC)
    target_compile_options(zerotape-demo PRIVATE
        /W3)
else()
    target_compile_options(zerotape-demo PRIVATE
        -Wall -Wextra -pedantic -Wno-unused-parameter)
endif()
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(zerotape-demo PRIVATE ZT_DEBUG)
endif()
if(USE_FORTIFY)
    target_link_libraries(zerotape-demo Fortify)
    target_compile_definitions(zerotape-demo PRIVATE FORTIFY)
endif()
