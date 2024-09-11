# utf-8 source files and binary 

add_library(options_compile_utf8 INTERFACE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(options_compile_utf8 INTERFACE "/utf-8")
endif ()
