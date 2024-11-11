# xxhash

CPMAddPackage(
    NAME xxhash
    VERSION 0.8.2
    GITHUB_REPOSITORY Cyan4973/xxHash
    DOWNLOAD_ONLY YES
)

if(xxhash_ADDED)
    add_library(xxhash STATIC)
    set_target_properties(xxhash PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    target_include_directories(xxhash PUBLIC
        ${xxhash_SOURCE_DIR}
    )
    target_sources(xxhash PRIVATE
        ${xxhash_SOURCE_DIR}/xxhash.c
        ${xxhash_SOURCE_DIR}/xxhash.h
    )
    set_target_properties(xxhash PROPERTIES FOLDER external)
endif()
