add_subdirectory(builder)
add_subdirectory(file-system)

add_custom_target(generate-embedded-file-system
    $<TARGET_FILE:internal-lua-scripts-builder>
        ${CMAKE_CURRENT_LIST_DIR}/scripts
        --output ${CMAKE_CURRENT_LIST_DIR}/file-system/generated/scripts.cpp
)
