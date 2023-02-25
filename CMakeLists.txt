cmake_minimum_required(VERSION 3.22)
project(LuaSTGSub)

#option(LINK_STEAM_API "Enable Steam API" OFF)
#option(LUASTG_RESDIR "Custom configurations" "res")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(cmake/CheckPlatform.cmake)
include(cmake/TargetCommonOptions.cmake)
add_compile_definitions(
    _WIN32_WINNT=0x0601       # _WIN32_WINNT_WIN7
    NTDDI_VERSION=0x06010000  # NTDDI_WIN7
)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Debug>:DLL>")

include(cmake/loader.cpm.cmake)

CPMInitialize()

include(cmake/packages.cpm.cmake)
include(cmake/packages.nuget.cmake)
include(cmake/packages.xiph.cmake)
add_subdirectory(external)
add_subdirectory(imgui)

add_subdirectory(utility)
add_subdirectory(LuaSTG)
