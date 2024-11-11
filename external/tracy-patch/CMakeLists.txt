add_library(TracyAPI INTERFACE)
target_include_directories(TracyAPI INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(TracyAPI INTERFACE tracy)
