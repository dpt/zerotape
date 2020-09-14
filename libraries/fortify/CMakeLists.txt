add_library(Fortify fortify.c)
target_compile_definitions(Fortify PRIVATE FORTIFY)
target_include_directories(Fortify PUBLIC ../../include/)
