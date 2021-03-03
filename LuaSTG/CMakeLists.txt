# LuaSTG Engine

include(Custom.cmake OPTIONAL)
if(NOT DEFINED LUASTG_RESDIR)
    message("LuaSTG will using default config, built-in resources and source codes")
    set(LUASTG_RESDIR res)
else()
    message("LuaSTG will using custom config, built-in resources and source codes: " ${LUASTG_RESDIR})
endif()

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
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # z family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:zlib>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zlib>/$<TARGET_FILE_NAME:zlib>"                "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zlib>/$<TARGET_FILE_NAME:zlib>"                ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:zip>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zip>/$<TARGET_FILE_NAME:zip>"                  "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zip>/$<TARGET_FILE_NAME:zip>"                  ${CMAKE_SOURCE_DIR}/bin
    
    # xiph family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:ogg>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:ogg>/$<TARGET_FILE_NAME:ogg>"                  "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:ogg>/$<TARGET_FILE_NAME:ogg>"                  ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:vorbis>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbis>/$<TARGET_FILE_NAME:vorbis>"            "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbis>/$<TARGET_FILE_NAME:vorbis>"            ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:vorbisfile>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbisfile>/$<TARGET_FILE_NAME:vorbisfile>"    "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbisfile>/$<TARGET_FILE_NAME:vorbisfile>"    ${CMAKE_SOURCE_DIR}/bin
    
    # font family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:freetype>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:freetype>/$<TARGET_FILE_NAME:freetype>"        "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:freetype>/$<TARGET_FILE_NAME:freetype>"        ${CMAKE_SOURCE_DIR}/bin
    
    # fancy family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:fancy2d>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:fancy2d>/$<TARGET_FILE_NAME:fancy2d>"          "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:fancy2d>/$<TARGET_FILE_NAME:fancy2d>"          ${CMAKE_SOURCE_DIR}/bin
    
    # lua family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:luajit>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            ${CMAKE_SOURCE_DIR}/bin
    
    # lua extensions
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:lcjson>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lcjson>/$<TARGET_FILE_NAME:lcjson>"            "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lcjson>/$<TARGET_FILE_NAME:lcjson>"            ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:lfs>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lfs>/$<TARGET_FILE_NAME:lfs>"                  "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lfs>/$<TARGET_FILE_NAME:lfs>"                  ${CMAKE_SOURCE_DIR}/bin
    
    # luastg engine
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:LuaSTG>"            ${CMAKE_SOURCE_DIR}/bin
)
