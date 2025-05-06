set(tool_name internal-lua-scripts-builder)

add_executable(${tool_name})
target_compile_options(${tool_name} PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/sdl>"
        "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)
set_target_properties(${tool_name} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
target_include_directories(${tool_name} PRIVATE ../common)
target_sources(${tool_name} PRIVATE main.cpp)

set_target_properties(internal-lua-scripts-builder PROPERTIES FOLDER tool/toolchain)
