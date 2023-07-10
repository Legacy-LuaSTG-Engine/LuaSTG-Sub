# libogg
# libvorbis 基础库

CPMAddPackage(
    NAME libogg
    VERSION 1.3.5
    GITHUB_REPOSITORY xiph/ogg
    GIT_TAG db5c7a49ce7ebda47b15b78471e78fb7f2483e22
    OPTIONS
    "BUILD_TESTING OFF"
)

if(libogg_ADDED)
    set_target_properties(ogg PROPERTIES FOLDER external)
endif()

# libvorbis
# vorbis ogg 音频文件解码

CPMAddPackage(
    NAME libvorbis
    VERSION 1.3.7
    GITHUB_REPOSITORY xiph/vorbis
    GIT_TAG 84c023699cdf023a32fa4ded32019f194afcdad0
    OPTIONS
    "BUILD_TESTING OFF"
)

if(libvorbis_ADDED)
    set_target_properties(vorbis PROPERTIES FOLDER external)
    set_target_properties(vorbisfile PROPERTIES FOLDER external)
    set_target_properties(vorbisenc PROPERTIES FOLDER external)
endif()

# libflac
# flac 音频文件解码

CPMAddPackage(
    NAME libflac
    VERSION 1.4.3
    GITHUB_REPOSITORY xiph/flac
    GIT_TAG 1.4.3
    OPTIONS
    "BUILD_CXXLIBS OFF"
    "BUILD_PROGRAMS OFF"
    "BUILD_EXAMPLES OFF"
    "BUILD_TESTING OFF"
    "BUILD_DOCS OFF"
    "INSTALL_MANPAGES OFF"
    "INSTALL_PKGCONFIG_MODULES OFF"
    "INSTALL_CMAKE_CONFIG_MODULE OFF"
)

if(libflac_ADDED)
    set_target_properties(getopt PROPERTIES FOLDER external/libflac)
    set_target_properties(grabbag PROPERTIES FOLDER external/libflac)
    set_target_properties(FLAC PROPERTIES FOLDER external/libflac)
    set_target_properties(replaygain_analysis PROPERTIES FOLDER external/libflac)
    set_target_properties(replaygain_synthesis PROPERTIES FOLDER external/libflac)
    set_target_properties(utf8 PROPERTIES FOLDER external/libflac)
    if(TARGET FLAC-asm)
        set_target_properties(FLAC-asm PROPERTIES FOLDER external/libflac)
    endif()
endif()
