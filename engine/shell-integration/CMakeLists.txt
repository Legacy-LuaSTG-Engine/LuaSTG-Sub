set(lib_src
    core/ShellIntegration.hpp
    backend/ShellIntegration.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

set(lib_name Core.ShellIntegration)
add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_compile_features(${lib_name} PRIVATE cxx_std_20)
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PUBLIC
    utf8
    Core.Logging
)

set_target_properties(${lib_name} PROPERTIES FOLDER engine)
