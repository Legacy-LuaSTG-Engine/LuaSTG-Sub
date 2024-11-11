# tinyobjloader

CPMAddPackage(
    NAME tinyobjloader
    #VERSION 2.0.0
    GITHUB_REPOSITORY tinyobjloader/tinyobjloader
    GIT_TAG v2.0.0rc13
    DOWNLOAD_ONLY YES
)

if(tinyobjloader_ADDED)
    # tinyobjloader 提供的 CMake 支持有点脏，我们自己来
    add_library(tinyobjloader STATIC)
    target_include_directories(tinyobjloader PUBLIC
        ${tinyobjloader_SOURCE_DIR}
    )
    target_sources(tinyobjloader PRIVATE
        ${tinyobjloader_SOURCE_DIR}/tiny_obj_loader.h
        ${tinyobjloader_SOURCE_DIR}/tiny_obj_loader.cc
    )
    set_target_properties(tinyobjloader PROPERTIES FOLDER external)
endif()
