
add_library(utility STATIC)
luastg_target_common_options(utility)
target_include_directories(utility PUBLIC .)
set(utility_sources
    utility/encoding.hpp
    utility/encoding.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${utility_sources})
target_sources(utility PRIVATE ${utility_sources})
