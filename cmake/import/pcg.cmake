# pcg random

CPMAddPackage(
    NAME pcg_cpp
    GITHUB_REPOSITORY imneme/pcg-cpp
    GIT_TAG 428802d1a5634f96bcd0705fab379ff0113bcf13
    DOWNLOAD_ONLY YES
)

if(pcg_cpp_ADDED)
    add_library(pcg_cpp INTERFACE)
    target_include_directories(pcg_cpp INTERFACE
        ${pcg_cpp_SOURCE_DIR}/include
    )
endif()
