#option(LUASTG_RESDIR "Custom configurations" "res")

option(LUASTG_SUPPORTS_WINDOWS_7 "LuaSTG Sub Windows compatibility: Windows 7" ON)
option(LUASTG_LINK_YY_THUNKS "Link to YY_Thunks for older Windows version (not recommended)" OFF)
option(LUASTG_LINK_LUASOCKET "Link to luasocket" OFF)
option(LUASTG_LINK_TRACY_CLIENT "Link to Tracy client" OFF)
set(LUASTG_RESDIR "${CMAKE_SOURCE_DIR}/LuaSTG/LuaSTG/Custom" CACHE PATH "LuaSTG custom build configuration")

function(luastg_cmake_option)
    set(option_args      FORCE)
    set(one_value_args   NAME TYPE HELP)
    set(multi_value_args VALUE)
    cmake_parse_arguments(PARSE_ARGV 0 arg "${option_args}" "${one_value_args}" "${multi_value_args}")
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.2.0")
        set(CACHE{${arg_NAME}} TYPE ${arg_TYPE} HELP ${arg_HELP} VALUE ${arg_VALUE})
    else ()
        set(${arg_NAME} ${arg_VALUE} CACHE ${arg_TYPE} ${arg_HELP})
    endif ()
endfunction()

# LuaSTG - Configuration

luastg_cmake_option(
    NAME LUASTG_CONFIGURATION_DEFAULT_FILE_PATH
    TYPE STRING
    HELP "LuaSTG: Configuration: Default file path"
    VALUE "config.json"
)
luastg_cmake_option(
    NAME LUASTG_CONFIGURATION_LUA_SCRIPT_ENABLE
    TYPE BOOL
    HELP "LuaSTG: Configuration: Enable lua script"
    VALUE TRUE
)
luastg_cmake_option(
    NAME LUASTG_CONFIGURATION_LUA_SCRIPT_PATHS
    TYPE STRING
    HELP "LuaSTG: Configuration: Lua script paths"
    VALUE "launch;launch.lua"
)

# LuaSTG - Logging

luastg_cmake_option(
    NAME LUASTG_LOGGING_DEFAULT_FILE_PATH
    TYPE STRING
    HELP "LuaSTG: Logging: Default file path"
    VALUE "engine.log"
)

# LuaSTG - Steam API

luastg_cmake_option(
    NAME LUASTG_STEAM_API_ENABLE
    TYPE BOOL
    HELP "LuaSTG: Steam API: Enable and link to steam_api[64].dll"
    VALUE FALSE
)
luastg_cmake_option(
    NAME LUASTG_STEAM_API_APP_ID
    TYPE STRING
    HELP "LuaSTG: Steam API: APP ID"
    VALUE "0"
)
luastg_cmake_option(
    NAME LUASTG_STEAM_API_FORCE_LAUNCH_BY_STEAM
    TYPE BOOL
    HELP "LuaSTG: Steam API: Force launch by Steam"
    VALUE FALSE
)
