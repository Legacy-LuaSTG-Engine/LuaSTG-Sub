#==================== debug family ====================#

add_library(spdlog STATIC IMPORTED GLOBAL)
target_compile_definitions(spdlog INTERFACE
    SPDLOG_COMPILED_LIB # prebuild
)
target_include_directories(spdlog INTERFACE
    spdlog/include
)
set_target_properties(spdlog PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/lib//spdlog.lib
)

#==================== deps ====================#

if(NOT platform_amd64)
    add_subdirectory(x86)
else()
    add_subdirectory(amd64)
endif()
