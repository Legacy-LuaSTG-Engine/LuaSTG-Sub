cmake_minimum_required(VERSION 3.19)
project(LuaSTGSub)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(cmake/CheckPlatform.cmake)
include(cmake/TargetCommonOptions.cmake)
add_compile_definitions(
    _WIN32_WINNT=0x0601       # _WIN32_WINNT_WIN7
    NTDDI_VERSION=0x06010000  # NTDDI_WIN7
)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Debug>:DLL>")

add_subdirectory(prebuild)
add_subdirectory(ThirdParty)
add_subdirectory(DirectX)

add_subdirectory(utility)
add_subdirectory(fancylib)
add_subdirectory(fancy2d)

add_subdirectory(imgui)
add_subdirectory(steam)
add_subdirectory(xmath)

add_subdirectory(LuaSTG)
