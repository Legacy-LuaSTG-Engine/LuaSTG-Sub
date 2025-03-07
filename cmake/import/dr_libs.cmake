# dr_libs

CPMAddPackage(
    NAME dr_libs
    VERSION 2024.12.17
    # ！！！警告：不要直接使用 mackron dr_libs 的 git 仓库，否则下载耗时++++
    #GITHUB_REPOSITORY mackron/dr_libs
    #GIT_TAG 660795b2834aebb2217c9849d668b6e4bd4ef810
    URL https://github.com/mackron/dr_libs/archive/660795b2834aebb2217c9849d668b6e4bd4ef810.zip
    URL_HASH SHA256=5AA009A6E5BC35361D4255490C6976560D66B261DF8CD657EB0950A29AC34D15
    DOWNLOAD_ONLY YES
)

if(dr_libs_ADDED)
    add_library(dr_libs STATIC)
    target_include_directories(dr_libs PUBLIC
        ${dr_libs_SOURCE_DIR}
    )
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/dr_libs/dr_libs.c)
        file(WRITE ${CMAKE_BINARY_DIR}/dr_libs/dr_libs.c
            "#define DR_WAV_IMPLEMENTATION\n"
            "#define DR_MP3_IMPLEMENTATION\n"
            "#define DR_FLAC_IMPLEMENTATION\n"
            "#include \"dr_wav.h\"\n"
            "#include \"dr_mp3.h\"\n"
            "#include \"dr_flac.h\"\n"
        )
    endif ()
    target_sources(dr_libs PRIVATE
        ${dr_libs_SOURCE_DIR}/dr_wav.h
        ${dr_libs_SOURCE_DIR}/dr_mp3.h
        ${dr_libs_SOURCE_DIR}/dr_flac.h
        ${CMAKE_BINARY_DIR}/dr_libs/dr_libs.c
    )
    set_target_properties(dr_libs PROPERTIES FOLDER external)
endif()
