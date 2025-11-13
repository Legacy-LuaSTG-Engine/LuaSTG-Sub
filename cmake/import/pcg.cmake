# pcg random

CPMAddPackage(
    NAME pcg_cpp
    GITHUB_REPOSITORY Legacy-LuaSTG-Engine/pcg-cpp
    GIT_TAG 69187835e14a8fa47f663ddf2647d100ec5f1eb0
    DOWNLOAD_ONLY YES
)

if(pcg_cpp_ADDED)
    add_library(pcg_cpp INTERFACE)
    target_include_directories(pcg_cpp INTERFACE
        ${pcg_cpp_SOURCE_DIR}/include
    )
endif()
