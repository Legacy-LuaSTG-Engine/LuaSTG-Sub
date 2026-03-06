file(GLOB_RECURSE src RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h *.c *.hpp *.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${src})

add_library(Core.UUID STATIC)
luastg_target_common_options(Core.UUID)
luastg_target_more_warning(Core.UUID)
target_include_directories(Core.UUID PUBLIC .)
target_sources(Core.UUID PRIVATE ${src})

set_target_properties(Core.UUID PROPERTIES FOLDER engine)
