cmake_minimum_required(VERSION 3.19)
project(LuaSTGSub)

include(cmake/CheckPlatform.cmake)
add_compile_definitions(
    _WIN32_WINNT=0x0601       # _WIN32_WINNT_WIN7
    NTDDI_VERSION=0x06010000  # NTDDI_WIN7
)

add_subdirectory(ThirdParty)
add_subdirectory(prebuild)
add_subdirectory(DirectX)

add_subdirectory(fancylib)
add_subdirectory(fancy2d)

add_subdirectory(imgui)
add_subdirectory(steam)
add_subdirectory(eyes2d)
#add_subdirectory(slow)
#add_subdirectory(tool)

add_subdirectory(LuaSTG)
