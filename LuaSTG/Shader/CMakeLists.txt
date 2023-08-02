cmake_minimum_required(VERSION 3.0)

add_executable(ShaderCompiler)
set_target_properties(ShaderCompiler PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(ShaderCompiler
PRIVATE
    "/utf-8"
    "/W4" # 更多警告
)
target_link_options(ShaderCompiler
PRIVATE
    "/DEPENDENTLOADFLAG:0x800" # Windows 10 1607+ 强制 DLL 搜索目录为系统目录
)
target_compile_definitions(ShaderCompiler
PRIVATE
    _UNICODE
    UNICODE
    HREADER_OUTPUT_DIRECTORY=\"${CMAKE_CURRENT_LIST_DIR}/generated\"
    HREADER_INPUT_DIRECTORY=\"${CMAKE_CURRENT_LIST_DIR}\"
)
target_include_directories(ShaderCompiler
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_sources(ShaderCompiler
PRIVATE
    ShaderCompiler.cpp
)
target_link_libraries(ShaderCompiler
PRIVATE
    d3dcompiler.lib
    cppwinrt
)
