# steam api

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/public/steam AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin)
    # steam api 32
    add_library(steam_api SHARED IMPORTED GLOBAL)
    target_include_directories(steam_api INTERFACE
        SteamworksSDK/sdk/public/steam
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_directories(steam_api INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64
        )
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.dll
            IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.lib
        )
    else()
        target_link_directories(steam_api INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin
        )
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.dll
            IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.lib
        )
    endif()
    message("Find Steamworks SDK")
else()
    message("Steamworks SDK doesn't exist")
endif()

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
if((LINK_STEAM_API OR LUASTG_LINK_STEAM_API) AND (TARGET steam_api)) # LINK_STEAM_API for compatibility
    target_compile_definitions(lua_steam_api PUBLIC
        HAVE_STEAM_API
    )
    target_sources(lua_steam_api PRIVATE
        binding/lua_steam_SteamCallbackWrapper.inl
        binding/lua_steam_SteamAPI.inl
        binding/lua_steam_SteamInput.inl
        binding/lua_steam_SteamUserStats.inl
        binding/lua_steam_SteamUtils.inl
        binding/lua_steam.cpp
    )
    target_link_libraries(lua_steam_api PUBLIC
        steam_api
    )
    message("Link Steamworks SDK")
else()
    target_sources(lua_steam_api PRIVATE
        binding/lua_steam_empty.cpp
    )
    message("Ignore Steamworks SDK")
endif()
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
