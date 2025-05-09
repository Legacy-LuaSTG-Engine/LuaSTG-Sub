set(tool_name embedded-file-system-builder)

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
target_include_directories(${tool_name} PRIVATE ../../engine/embedded-file-system)
target_sources(${tool_name} PRIVATE main.cpp)

set_target_properties(embedded-file-system-builder PROPERTIES FOLDER tool)

add_custom_target(generate-embedded-file-system
    $<TARGET_FILE:${tool_name}>
        ${CMAKE_CURRENT_LIST_DIR}/../../engine/embedded-script
        --output ${CMAKE_CURRENT_LIST_DIR}/../../engine/embedded-file-system/generated/scripts.cpp
)

set_target_properties(generate-embedded-file-system PROPERTIES FOLDER tool)
