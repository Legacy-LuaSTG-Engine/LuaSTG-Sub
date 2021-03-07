# steam api

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/public/steam AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin)
    # steam api 32
    add_library(steam_api SHARED IMPORTED GLOBAL)
    target_include_directories(steam_api INTERFACE
        SteamworksSDK/sdk/public/steam
    )
    set_target_properties(steam_api PROPERTIES
        IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.dll
        IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/steam_api.lib
    )
    # steam api 64
    add_library(steam_api64 SHARED IMPORTED GLOBAL)
    target_include_directories(steam_api64 INTERFACE
        SteamworksSDK/sdk/public/steam
    )
    set_target_properties(steam_api64 PROPERTIES
        IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.dll
        IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/SteamworksSDK/sdk/redistributable_bin/win64/steam_api64.lib
    )
    message("Find Steamworks SDK")
else()
    message("Steamworks SDK doesn't exist")
endif()
