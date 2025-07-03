# spdlog

CPMAddPackage(
    NAME spdlog
    VERSION 1.15.3
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS
    "SPDLOG_WCHAR_SUPPORT ON"
    "SPDLOG_WCHAR_FILENAMES ON"
    "SPDLOG_WCHAR_CONSOLE ON"
    "SPDLOG_DISABLE_DEFAULT_LOGGER ON"
)

if(TARGET spdlog)
    if(MSVC)
        target_compile_options(spdlog PUBLIC
            "/DSPDLOG_SHORT_LEVEL_NAMES={\"V\",\"D\",\"I\",\"W\",\"E\",\"F\",\"O\"}"
        )
    endif()
    set_target_properties(spdlog PROPERTIES FOLDER external)
endif()
