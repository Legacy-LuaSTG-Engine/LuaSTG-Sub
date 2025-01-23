add_subdirectory(utf8)

add_library(utility STATIC)
luastg_target_common_options(utility)
luastg_target_more_warning(utility)
target_include_directories(utility PUBLIC .)
set(utility_sources
    utility/path.hpp
    utility/path.cpp
    utility/utf.hpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${utility_sources})
target_sources(utility PRIVATE ${utility_sources})
target_link_libraries(utility PUBLIC utf8)

add_subdirectory(lua)

set_target_properties(utf8 PROPERTIES FOLDER utility)
set_target_properties(utility PROPERTIES FOLDER utility)
set_target_properties(lua_plus PROPERTIES FOLDER utility)
