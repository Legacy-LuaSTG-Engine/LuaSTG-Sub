cmake_minimum_required(VERSION 3.15)

set(LUASTG_RESDIR res)
option(LUASTG_DEVINFO "write more debug info to log." on)

add_executable(LuaSTG WIN32)

target_include_directories(LuaSTG PRIVATE
    src
    ${LUASTG_RESDIR}
    
    ../fancylib
    ../fancy2d
    
    ../eyes2d
    
    ../prebuild/luajit/include
    ../prebuild/xmath/include
    
    ../SteamAPI/public/steam
)

target_compile_options(LuaSTG PRIVATE
    "/std:c++17"
)

target_compile_definitions(LuaSTG PRIVATE
    UNICODE
    _UNICODE
    LUA_BUILD_AS_DLL
)

if(LUASTG_DEVINFO)
    target_compile_definitions(LuaSTG PRIVATE LDEVVERSION)
endif()

aux_source_directory(src LUASTG_SRC)
aux_source_directory(src/LuaWrapper LUASTG_SRC_LW)
aux_source_directory(${LUASTG_RESDIR} LUASTG_RES)

target_sources(LuaSTG PRIVATE
    ${LUASTG_SRC}
    ${LUASTG_SRC_LW}
    ${LUASTG_RES}
    
    ${LUASTG_RESDIR}/resource.rc
    ${LUASTG_RESDIR}/app.ico
    ${LUASTG_RESDIR}/splash.png
)

target_link_directories(LuaSTG PRIVATE
    ../prebuild/dxsdk/lib
    ../prebuild/luajit/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/xmath/lib/${EXT_LIBRARY_TYPE}
    ../SteamAPI/redistributable_bin
    
    ../prebuild/zlib/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libzip/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libogg/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libvorbis/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/freetype/lib/${EXT_LIBRARY_TYPE}
)

target_link_libraries(LuaSTG
    fancylib
    fancy2d
    eyes2d
    
    Gdiplus.lib
    
    lua51.lib
    xmath.lib
)
