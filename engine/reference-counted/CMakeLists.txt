file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR} core/*.hpp core/*.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

set(lib_name "Core.ReferenceCounted")

add_library(${lib_name})
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PRIVATE options_compile_utf8)
target_link_libraries(${lib_name} PUBLIC Core.UUID)

set_target_properties(${lib_name} PROPERTIES FOLDER engine)

set(test_name "Core.ReferenceCounted.Test")

add_executable(${test_name})
luastg_target_common_options(${test_name})
luastg_target_more_warning(${test_name})
target_sources(${test_name} PRIVATE test/Test.cpp)
target_link_libraries(${test_name} PRIVATE options_compile_utf8 ${lib_name} GTest::gtest_main)

set_target_properties(${test_name} PROPERTIES FOLDER engine)
