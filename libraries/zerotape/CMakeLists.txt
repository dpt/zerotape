add_library(zerotape)
target_sources(zerotape PRIVATE zt-ast-impl.h zt-ast-viz.c zt-ast-walk.c zt-ast.c zt-ast.h zt-gramx.h zt-lex-impl.h zt-lex-test.c zt-lex-test.h zt-lex.c zt-lex.h zt-load.c zt-parser.c zt-parser.h zt-run.c zt-run.h zt-save.c zt-walk.c zt-walk.h) # add regular sources
target_include_directories(zerotape PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(zerotape PRIVATE zt-gram.c zt-gram.h) # add generated sources
target_include_directories(zerotape PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}) # generated sources need a specific include path setting

if(MSVC)
	target_compile_options(zerotape PRIVATE /W4 /WX)
else()
	target_compile_options(zerotape PRIVATE -std=c89 -Wall -Wextra -pedantic)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_definitions(zerotape PRIVATE ZTAST_LOG ZT_DEBUG)
endif()

# todo: ZT_USE_HEX

set(LEMON_SRC ${CMAKE_SOURCE_DIR}/libraries/lemon)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.c ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.h
	COMMAND lemon -s -d${CMAKE_CURRENT_BINARY_DIR} -T${LEMON_SRC}/lempar.c ${CMAKE_CURRENT_SOURCE_DIR}/zt-gram.y
	DEPENDS lemon
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/zt-gram.y 
	)

add_custom_target(ztparser ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.c ${CMAKE_CURRENT_BINARY_DIR}/zt-gram.h)

add_dependencies(zerotape ztparser)

if(USE_FORTIFY)
	target_compile_definitions(zerotape PRIVATE FORTIFY)
	target_link_libraries(zerotape PUBLIC Fortify)
endif()
