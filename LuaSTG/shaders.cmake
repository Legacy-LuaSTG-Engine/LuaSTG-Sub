add_library(GeneratedShaderHeaders INTERFACE)
target_include_directories(GeneratedShaderHeaders
INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Shader/generated
)
