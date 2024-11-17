set(lib_name LuaSTG.Sub.Scriptable)

file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR} "core/*.hpp" "core/*.cpp")
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_compile_features(${lib_name} PRIVATE cxx_std_20)
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PUBLIC lua51_static)

set(lib_test_name LuaSTG.Sub.Scriptable.Test)

add_executable(${lib_test_name})
luastg_target_common_options(${lib_test_name})
luastg_target_more_warning(${lib_test_name})
target_compile_features(${lib_test_name} PRIVATE cxx_std_20)
target_sources(${lib_test_name} PRIVATE test/test.cpp)
target_link_libraries(${lib_test_name} PRIVATE ${lib_name})
