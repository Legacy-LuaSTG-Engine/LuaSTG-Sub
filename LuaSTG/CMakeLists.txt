# LuaSTG Engine

set(LUASTG_RESDIR res)
#set(LUASTG_RESDIR custom/THEIS)
#set(LUASTG_RESDIR custom/THTUO)

#set(LUASTG_RESDIR custom/THBDB.develop)
#set(LUASTG_RESDIR custom/THBDB.steam.demo)

add_executable(LuaSTG WIN32)

set_target_properties(LuaSTG PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(LuaSTG PUBLIC
    "/utf-8"
    "/MP"
    "/arch:SSE2"
)

target_include_directories(LuaSTG PRIVATE
    ..
    
    src
    ${LUASTG_RESDIR}
    src/LuaWrapper
    
    ../SteamAPI/public/steam
)

target_compile_definitions(LuaSTG PRIVATE
    UNICODE
    _UNICODE
)

target_compile_definitions(LuaSTG PRIVATE LDEVVERSION)

aux_source_directory(../Common COMMON_SRC)
aux_source_directory(src LUASTG_SRC)
aux_source_directory(src/LuaWrapper LUASTG_SRC_LW)
aux_source_directory(${LUASTG_RESDIR} LUASTG_RES)

target_sources(LuaSTG PRIVATE
    ${COMMON_SRC}
    
    ${LUASTG_SRC}
    ${LUASTG_SRC_LW}
    ${LUASTG_RES}
    
    src/LuaSTG.manifest
    ${LUASTG_RESDIR}/resource.rc
    ${LUASTG_RESDIR}/app.ico
)

target_link_directories(LuaSTG PRIVATE
    ../SteamAPI/redistributable_bin
)

target_link_libraries(LuaSTG
    fancylib
    fancy2d
    eyes2d
    xmath
    luajit
    imgui
    lua_imgui
)

add_custom_command(TARGET LuaSTG POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/copy_bin.bat $<CONFIG>
)
