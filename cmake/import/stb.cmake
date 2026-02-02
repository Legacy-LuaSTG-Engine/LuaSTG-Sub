# nothings stb

CPMAddPackage(
    NAME nothings_stb
    VERSION 2025.5.26
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c
    DOWNLOAD_ONLY YES
)

if(nothings_stb_ADDED)
    add_library(nothings_stb STATIC)
    target_compile_definitions(nothings_stb PRIVATE
    )
    target_include_directories(nothings_stb PUBLIC
        ${nothings_stb_SOURCE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/github-nothings-stb/include
    )
    target_sources(nothings_stb PRIVATE
        ${nothings_stb_SOURCE_DIR}/stb_image.h
        ${nothings_stb_SOURCE_DIR}/stb_vorbis.c
        ${CMAKE_CURRENT_LIST_DIR}/github-nothings-stb/include/stb_vorbis.h
        ${CMAKE_CURRENT_LIST_DIR}/github-nothings-stb/src/stb_image.c
    )
    set_target_properties(nothings_stb PROPERTIES FOLDER external)
endif()
