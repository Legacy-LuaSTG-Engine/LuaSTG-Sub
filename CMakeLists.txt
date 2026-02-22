cmake_minimum_required(VERSION 3.31)
project(LuaSTGSub)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/architecture.cmake)
include(cmake/luastg-options.cmake)
include(cmake/luastg-utils.cmake)
include(cmake/msvc-c-runtime.cmake)

msvc_static_runtime_library()

include(cmake/import/all.cmake)
include(cmake/build/all.cmake)
add_subdirectory(external)
include(cmake/packages.lua.cmake)
add_subdirectory(imgui)

add_subdirectory(engine)
target_link_libraries(lua_imgui PUBLIC lua_plus) # TODO: 丑不拉几的
add_subdirectory(tool)
add_subdirectory(LuaSTG)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    message(STATUS "Current CMake generator is Visual Studio, set startup project to LuaSTG")
    set_directory_properties(PROPERTY VS_STARTUP_PROJECT LuaSTG)
endif ()
