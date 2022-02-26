# ==================== spdlog ====================

option(SPDLOG_ENABLE_PCH "Build static or shared library using precompiled header to speed up compilation time" ON)
option(SPDLOG_BUILD_WARNINGS "Enable compiler warnings" ON)
option(SPDLOG_SANITIZE_ADDRESS "Enable address sanitizer in tests" ON)
#option(SPDLOG_WCHAR_SUPPORT "Support wchar api" ON)
option(SPDLOG_WCHAR_FILENAMES "Support wchar filenames" ON)
option(SPDLOG_DISABLE_DEFAULT_LOGGER "Disable default logger creation" ON)
add_subdirectory(spdlog)
target_compile_options(spdlog PUBLIC
    "/DSPDLOG_SHORT_LEVEL_NAMES={\"V\",\"D\",\"I\",\"W\",\"E\",\"F\",\"O\"}"
)

set_target_properties(spdlog PROPERTIES FOLDER external)
