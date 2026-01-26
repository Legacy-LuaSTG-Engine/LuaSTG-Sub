file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR} win32/*.hpp win32/*.cpp windows/*.hpp windows/*.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

set(lib_name "win32")

add_library(${lib_name})
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PRIVATE options_compile_utf8)
target_link_libraries(${lib_name} PUBLIC beautiful_win32_api)

set_target_properties(${lib_name} PROPERTIES FOLDER engine)
