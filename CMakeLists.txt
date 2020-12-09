cmake_minimum_required(VERSION 3.15)
project(LuaSTGSub)

set(EXT_LIBRARY_TYPE Debug)

add_subdirectory(fancylib)

add_subdirectory(DirectShowClass)
add_subdirectory(fancy2d)

add_subdirectory(imgui)
add_subdirectory(imgui/test)
add_subdirectory(eyes2d)
add_subdirectory(slow)

add_subdirectory(LuaSTG)
