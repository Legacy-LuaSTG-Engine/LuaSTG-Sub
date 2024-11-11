# nothings stb

CPMAddPackage(
    NAME nothings_stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG 5c205738c191bcb0abc65c4febfa9bd25ff35234
    DOWNLOAD_ONLY YES
)

if(nothings_stb_ADDED)
    add_library(nothings_stb STATIC)
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/nothings_stb/include)
        file(WRITE ${CMAKE_BINARY_DIR}/nothings_stb/include/placeholder "")
    endif ()
    target_include_directories(nothings_stb PUBLIC
        ${nothings_stb_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/nothings_stb/include
    )
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/nothings_stb/include/stb_vorbis.h)
        file(WRITE ${CMAKE_BINARY_DIR}/nothings_stb/include/stb_vorbis.h
            "#define STB_VORBIS_HEADER_ONLY\n"
            "#include \"stb_vorbis.c\"\n"
            "#undef STB_VORBIS_HEADER_ONLY\n"
        )
    endif ()
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/nothings_stb/nothings_stb.c)
        file(WRITE ${CMAKE_BINARY_DIR}/nothings_stb/nothings_stb.c
            "#define STB_IMAGE_IMPLEMENTATION\n"
            "#include \"stb_image.h\"\n"
            "#include \"stb_vorbis.c\"\n"
        )
    endif ()
    target_sources(nothings_stb PRIVATE
        ${CMAKE_BINARY_DIR}/nothings_stb/include/stb_vorbis.h
        ${nothings_stb_SOURCE_DIR}/stb_image.h
        ${CMAKE_BINARY_DIR}/nothings_stb/nothings_stb.c
    )
    set_target_properties(nothings_stb PROPERTIES FOLDER external)
endif()
