#==================== deps ====================#

if(NOT platform_amd64)
    add_subdirectory(x86)
else()
    add_subdirectory(amd64)
endif()
