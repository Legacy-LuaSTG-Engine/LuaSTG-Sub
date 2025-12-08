# Graphics

set(lib_name "Core.Graphics")

file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR} core/*.hpp core/*.cpp d3d11/*.hpp d3d11/*.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

add_library(${lib_name})
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PRIVATE options_compile_utf8)
target_link_libraries(${lib_name} PUBLIC
    Core.ReferenceCounted
    Core.Configuration
    Core.FileSystem
    Core.Logging
    Core.Math
    win32
)

set_target_properties(${lib_name} PROPERTIES FOLDER engine)
