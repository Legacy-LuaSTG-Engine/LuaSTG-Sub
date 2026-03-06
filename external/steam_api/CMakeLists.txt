# Steam API

set(STEAM_API_REQUIRED_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/public/steam/steam_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/public/steam/steam_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.dll
)

# See: cmake/luastg_options.cmake
if (LUASTG_STEAM_API_ENABLE)
    # check

    foreach (file IN LISTS STEAM_API_REQUIRED_FILES)
        if (NOT EXISTS ${file})
            message(FATAL_ERROR "LuaSTG: Steam API: Required file not found: ${file}")
        endif ()
    endforeach ()

    # import

    add_library(steam_api SHARED IMPORTED GLOBAL)
    target_include_directories(steam_api INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/public/steam"
    )
    if (LUASTG_ARCH STREQUAL "amd64")
        target_link_directories(steam_api INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64"
        )
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.dll"
            IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.lib"
        )
    elseif (LUASTG_ARCH STREQUAL "x86")
        target_link_directories(steam_api INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin"
        )
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.dll"
            IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.lib"
        )
    else ()
        message(FATAL_ERROR "Steam SDK: unsupported architecture")
    endif ()
endif ()

# binding

add_library(lua_steam_api STATIC)
luastg_target_more_warning(lua_steam_api)
set_target_properties(lua_steam_api PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(lua_steam_api PRIVATE
    "/wd4819"
)
target_include_directories(lua_steam_api PUBLIC
    binding
)
target_sources(lua_steam_api PRIVATE
    binding/lua_steam.h
)
if (LUASTG_STEAM_API_ENABLE)
    target_sources(lua_steam_api PRIVATE
        binding/lua_steam_SteamCallbackWrapper.inl
        binding/lua_steam_SteamAPI.inl
        binding/lua_steam_SteamInput.inl
        binding/lua_steam_SteamUserStats.inl
        binding/lua_steam.cpp
    )
    target_link_libraries(lua_steam_api PUBLIC
        steam_api
    )
else ()
    target_sources(lua_steam_api PRIVATE
        binding/lua_steam_empty.cpp
    )
endif ()
target_link_libraries(lua_steam_api PUBLIC
    lua51_static
)

set_target_properties(lua_steam_api PROPERTIES FOLDER lualib)

# binding dll

if(FALSE)
    add_library(steam SHARED)
    luastg_target_more_warning(steam)
    set_target_properties(steam PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_compile_options(steam PRIVATE
        "/wd4819"
    )
    target_sources(steam PRIVATE
        binding/lua_steam_dll.cpp
    )
    target_link_libraries(steam PRIVATE
        lua_steam_api
    )

    set_target_properties(steam PROPERTIES FOLDER lualib)
endif()
