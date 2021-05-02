# steam api

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/public/steam AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin)
    # steam api 32
    add_library(steam_api SHARED IMPORTED GLOBAL)
    target_include_directories(steam_api INTERFACE
        SteamworksSDK/sdk/public/steam
    )
    if(NOT platform_amd64)
        target_link_directories(steam_api INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin
        )
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.dll
            IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.lib
        )
    else()
        target_link_directories(steam_api INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64
        )
        set_target_properties(steam_api PROPERTIES
            IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.dll
            IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.lib
        )
    endif()
    message("Find Steamworks SDK")
else()
    message("Steamworks SDK doesn't exist")
endif()

# binding

add_library(lua_steam_api STATIC)
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
if(TARGET steam_api)
    target_compile_definitions(lua_steam_api PUBLIC
        HAVE_STEAM_API
    )
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
else()
    target_sources(lua_steam_api PRIVATE
        binding/lua_steam_empty.cpp
    )
endif()
target_link_libraries(lua_steam_api PUBLIC
    luajit
)

# binding dll

add_library(steam SHARED)
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
