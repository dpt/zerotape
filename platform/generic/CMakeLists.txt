# CMakeLists.txt
#
# zerotape
#
# Copyright (c) David Thomas, 2020
#

set(APPS_DIR ../../apps)

set(TEST_EXE ${PROJECT_NAME}-tests)
add_executable(${TEST_EXE} ${APPS_DIR}/zerotape-tests/tests.c)
target_link_libraries(${TEST_EXE} zerotape)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(${TEST_EXE} PRIVATE ZT_DEBUG)
endif()
if(USE_FORTIFY)
    target_link_libraries(${TEST_EXE} Fortify)
    target_compile_definitions(${TEST_EXE} PRIVATE FORTIFY)
endif()

set(DEMO_EXE ${PROJECT_NAME}-demo)
add_executable(${DEMO_EXE} ${APPS_DIR}/zerotape-demo/demo.c)
target_link_libraries(${DEMO_EXE} zerotape)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(${DEMO_EXE} PRIVATE ZT_DEBUG)
endif()
if(USE_FORTIFY)
    target_link_libraries(${DEMO_EXE} Fortify)
    target_compile_definitions(${DEMO_EXE} PRIVATE FORTIFY)
endif()
