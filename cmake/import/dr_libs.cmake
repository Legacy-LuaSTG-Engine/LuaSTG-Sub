# dr_libs

CPMAddPackage(
    NAME dr_libs
    VERSION 2026.1.17
    URL https://github.com/mackron/dr_libs/archive/5690d4671d7ad07ae6021756d7222eb159745f06.zip
    URL_HASH SHA256=DB9C1738C1D90EE81C0D04B35D0EA09661F1FCE00F321FD1159655819B0B8E85
    DOWNLOAD_ONLY YES
)

if(dr_libs_ADDED)
    add_library(dr_libs STATIC)
    target_include_directories(dr_libs PUBLIC
        ${dr_libs_SOURCE_DIR}
    )
    target_sources(dr_libs PRIVATE
        ${dr_libs_SOURCE_DIR}/dr_wav.h
        ${dr_libs_SOURCE_DIR}/dr_mp3.h
        ${dr_libs_SOURCE_DIR}/dr_flac.h
        ${CMAKE_CURRENT_LIST_DIR}/dr_libs/dr_libs.c
    )
    set_target_properties(dr_libs PROPERTIES FOLDER external)
endif()
