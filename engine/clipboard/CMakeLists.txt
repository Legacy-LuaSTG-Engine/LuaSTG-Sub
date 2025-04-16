set(lib_src
    core/Clipboard.hpp
    backend/Clipboard.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

set(lib_name Core.Clipboard)
add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_compile_features(${lib_name} PRIVATE cxx_std_20)
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PUBLIC Microsoft.Windows.ImplementationLibrary)

set_target_properties(Core.Clipboard PROPERTIES FOLDER engine)
