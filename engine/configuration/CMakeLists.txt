file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR} "core/*.hpp" "core/*.cpp")
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

set(lib_name Core.Configuration)
add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_compile_features(${lib_name} PRIVATE cxx_std_20)
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PUBLIC nlohmann_json)

set_target_properties(Core.Configuration PROPERTIES FOLDER engine)
