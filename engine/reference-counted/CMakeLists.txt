file(GLOB_RECURSE src RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h *.c *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${src})

add_library(Core.ReferenceCounted STATIC)
luastg_target_common_options(Core.ReferenceCounted)
luastg_target_more_warning(Core.ReferenceCounted)
target_include_directories(Core.ReferenceCounted PUBLIC .)
target_sources(Core.ReferenceCounted PRIVATE ${src})
target_link_libraries(Core.ReferenceCounted PRIVATE options_compile_utf8 Core.UUID)

set_target_properties(Core.ReferenceCounted PROPERTIES FOLDER engine)
