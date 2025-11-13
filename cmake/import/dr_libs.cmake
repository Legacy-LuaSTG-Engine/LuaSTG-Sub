# dr_libs

CPMAddPackage(
    NAME dr_libs
    VERSION 2025.9.10
    URL https://github.com/mackron/dr_libs/archive/3141b54b6b0067d15c4a3ec0877f2141a2a11347.zip
    URL_HASH SHA256=4B5050ED55C598311ED34FB8E59D8D9E356F093C2603ED3599807FA4112475C2
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
