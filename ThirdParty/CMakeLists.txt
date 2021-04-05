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
