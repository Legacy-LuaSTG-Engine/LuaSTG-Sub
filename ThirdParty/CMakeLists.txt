#==================== debug family ====================#

#add_library(spdlog STATIC IMPORTED GLOBAL)
#target_compile_definitions(spdlog INTERFACE
#    SPDLOG_COMPILED_LIB # prebuild
#)
#target_include_directories(spdlog INTERFACE
#    spdlog/include
#)
#set_target_properties(spdlog PROPERTIES
#    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/lib//spdlog.lib
#)

# tweakme.h
#     define SPDLOG_SHORT_LEVEL_NAMES { "V", "D", "I", "W", "E", "F", "O" }

option(SPDLOG_ENABLE_PCH "Build static or shared library using precompiled header to speed up compilation time" ON)
option(SPDLOG_BUILD_WARNINGS "Enable compiler warnings" ON)
option(SPDLOG_SANITIZE_ADDRESS "Enable address sanitizer in tests" ON)
#option(SPDLOG_WCHAR_SUPPORT "Support wchar api" ON)
option(SPDLOG_WCHAR_FILENAMES "Support wchar filenames" ON)
option(SPDLOG_DISABLE_DEFAULT_LOGGER "Disable default logger creation" ON)
add_subdirectory(spdlog)

set_target_properties(spdlog PROPERTIES FOLDER external)

#==================== lua cjson ====================#

add_library(lua_cjson STATIC)
target_compile_definitions(lua_cjson PRIVATE
    _CRT_SECURE_NO_WARNINGS
    DISABLE_INVALID_NUMBERS
    #snprintf=_snprintf
    #strncasecmp=_strnicmp
    strncasecmp=strnicmp
)
target_include_directories(lua_cjson PUBLIC
    luacjson
)
target_sources(lua_cjson PRIVATE
    luacjson/lua_cjson.h
    luacjson/lua_cjson.c
    luacjson/strbuf.h
    luacjson/strbuf.c
    luacjson/fpconv.h
    luacjson/fpconv.c
)
target_link_libraries(lua_cjson PUBLIC
    luajit
)

set_target_properties(lua_cjson PROPERTIES FOLDER external)

#==================== lua fs ====================#

add_library(lua_fs STATIC)
target_compile_definitions(lua_fs PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
target_include_directories(lua_fs PUBLIC
    luafilesystem/src
)
target_sources(lua_fs PRIVATE
    luafilesystem/src/lua_fs.h
    luafilesystem/src/lfs.h
    luafilesystem/src/lfs.c
)
target_link_libraries(lua_fs PUBLIC
    luajit
)

set_target_properties(lua_fs PROPERTIES FOLDER external)

#==================== lua extension ====================#

add_subdirectory(pugixml)

set_target_properties(pugixml-static PROPERTIES FOLDER external)

add_library(lua_xlsx_csv STATIC)
set_target_properties(lua_xlsx_csv PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(lua_xlsx_csv PUBLIC
    luaext
)
target_sources(lua_xlsx_csv PRIVATE
    luaext/lua_xlsx_csv.h
    luaext/lua_xlsx.cpp
    luaext/lua_csv.cpp
)
target_link_libraries(lua_xlsx_csv PUBLIC
    luajit
    zip
    pugixml
)

set_target_properties(lua_xlsx_csv PROPERTIES FOLDER external)
