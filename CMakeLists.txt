cmake_minimum_required(VERSION 3.22)
project(LuaSTGSub)

#option(LINK_STEAM_API "Enable Steam API" OFF)
#option(LUASTG_RESDIR "Custom configurations" "res")

option(LUASTG_SUPPORTS_WINDOWS_7 "LuaSTG Sub Windows compatibility: Windows 7" ON)
option(LUASTG_LINK_YY_THUNKS "Link to YY_Thunks for older Windows version" OFF)

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

include(cmake/loader.cpm.cmake)

CPMInitialize()

if(LUASTG_LINK_YY_THUNKS)
    include(cmake/import-yy.cmake)
endif()
include(cmake/packages.cpm.cmake)
include(cmake/packages.nuget.cmake)
include(cmake/packages.directx.cmake)
include(cmake/packages.z.cmake)
include(cmake/import-xiph.cmake)
include(cmake/import-typography.cmake)
add_subdirectory(external)
include(cmake/packages.lua.cmake)
include(LuaSTG/shaders.cmake)
add_subdirectory(imgui)

add_subdirectory(utility)
add_subdirectory(LuaSTG)
