cmake_minimum_required(VERSION 3.15)
project(LuaSTGSub)

set(EXT_LIBRARY_TYPE Release)

add_subdirectory(fancylib)

add_subdirectory(DirectShowClass)
add_subdirectory(fancy2d)

add_subdirectory(eyes2d)

add_subdirectory(LuaSTG)
