set(lib_name lua_plus)
add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE lua/plus.hpp lua/plus.cpp)
target_link_libraries(${lib_name} PUBLIC lua51_static)

set_target_properties(${lib_name} PROPERTIES FOLDER utility)
