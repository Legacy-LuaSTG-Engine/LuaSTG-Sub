set(lib_name LuaSTG.Sub.Clipboard)
set(clipboard_src
    core/Clipboard.hpp
    backend/Clipboard.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${clipboard_src})

add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_compile_features(${lib_name} PRIVATE cxx_std_20)
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${clipboard_src})
target_link_libraries(${lib_name} PUBLIC Microsoft.Windows.ImplementationLibrary)
