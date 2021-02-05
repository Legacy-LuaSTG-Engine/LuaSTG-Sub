cmake_minimum_required(VERSION 3.15)
project(LuaSTGSub)

add_compile_definitions(
    _WIN32_WINNT=1537       # _WIN32_WINNT_WIN7 (0x0601)
    NTDDI_VERSION=100728832 # NTDDI_WIN7 (0x06010000)
)

add_subdirectory(prebuild)
add_subdirectory(DirectX)

add_subdirectory(fancylib)
add_subdirectory(fancy2d)

add_subdirectory(imgui)
add_subdirectory(eyes2d)
#add_subdirectory(slow)

add_subdirectory(LuaSTG)
