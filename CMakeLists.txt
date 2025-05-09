cmake_minimum_required(VERSION 3.22)
project(LuaSTGSub)

#option(LUASTG_RESDIR "Custom configurations" "res")

option(LUASTG_SUPPORTS_WINDOWS_7 "LuaSTG Sub Windows compatibility: Windows 7" ON)
option(LUASTG_LINK_YY_THUNKS "Link to YY_Thunks for older Windows version (not recommended)" OFF)
option(LUASTG_LINK_LUASOCKET "Link to luasocket" OFF)
option(LUASTG_LINK_TRACY_CLIENT "Link to Tracy client" OFF)
option(LUASTG_LINK_STEAM_API "Link to Steam API" OFF)

if(LUASTG_SUPPORTS_WINDOWS_7)
    message(STATUS "[LuaSTG] Windows compatibility: Windows 7")
    add_compile_definitions(LUASTG_SUPPORTS_WINDOWS_7)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(cmake/TargetCommonOptions.cmake)
include(cmake/options.cmake)
include(cmake/msvc.crt.cmake)
#msvc_hybrid_runtime_library()
msvc_static_runtime_library()

include(cmake/import/all.cmake)
include(cmake/build/all.cmake)
add_subdirectory(external)
include(cmake/packages.lua.cmake)
include(LuaSTG/shaders.cmake)
add_subdirectory(imgui)

add_subdirectory(engine)
add_subdirectory(tool)
add_subdirectory(LuaSTG)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    message(STATUS "Current CMake generator is Visual Studio, set startup project to LuaSTG")
    set_directory_properties(PROPERTY VS_STARTUP_PROJECT LuaSTG)
endif ()
