
add_executable(test WIN32 test.cpp)
target_link_libraries(test PRIVATE
    DirectX::ToolKitMini
)
