# libvorbis

CPMAddPackage(
    NAME libvorbis
    VERSION 1.3.7
    GITHUB_REPOSITORY xiph/vorbis
    DOWNLOAD_ONLY YES
)

if (NOT libvorbis_ADDED)
    message(FATAL_ERROR "libvorbis is requied")
endif ()

# prepare

set(libvorbis_source_directory  ${libvorbis_SOURCE_DIR})
set(libvorbis_build_directory   ${CMAKE_BINARY_DIR}/build/libvorbis/$<CONFIG>)
set(libvorbis_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(libvorbis_library_file      ${libvorbis_install_directory}/lib/vorbis.lib)

luastg_cmake_external_build_prepare_directories(include/vorbis)

# external cmake build

add_custom_command(
    OUTPUT ${libvorbis_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${libvorbis_source_directory}
        -B ${libvorbis_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${libvorbis_install_directory}
        # cmake 4.0 breaking changes
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        # utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
    COMMAND ${CMAKE_COMMAND}
        --build ${libvorbis_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${libvorbis_build_directory}
        --config $<CONFIG>
        --prefix ${libvorbis_install_directory}
    VERBATIM
)
add_custom_target(libvorbis_build ALL
    DEPENDS ${libvorbis_library_file}
)
add_dependencies(libvorbis_build libogg_build)
set_target_properties(libvorbis_build PROPERTIES FOLDER external/libvorbis)

# external cmake build clean

add_custom_target(libvorbis_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${libvorbis_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${libvorbis_library_file}
    VERBATIM
)
set_target_properties(libvorbis_clean PROPERTIES FOLDER external/libvorbis)

# import

## vorbis

add_library(libvorbis STATIC IMPORTED GLOBAL)
add_library(Vorbis::vorbis ALIAS libvorbis)
target_include_directories(libvorbis
INTERFACE
    ${libvorbis_install_directory}/include
)
set_target_properties(libvorbis PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/vorbis.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/vorbis.lib
)
target_link_libraries(libvorbis
INTERFACE
    libogg
)
add_dependencies(libvorbis libvorbis_build)

## vorbisfile

add_library(libvorbisfile STATIC IMPORTED GLOBAL)
add_library(Vorbis::vorbisfile ALIAS libvorbisfile)
target_include_directories(libvorbisfile
INTERFACE
    ${libvorbis_install_directory}/include
)
set_target_properties(libvorbisfile PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/vorbisfile.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/vorbisfile.lib
)
target_link_libraries(libvorbisfile
INTERFACE
    libvorbis
)
add_dependencies(libvorbisfile libvorbis_build)

## vorbisenc

add_library(libvorbisenc STATIC IMPORTED GLOBAL)
add_library(Vorbis::vorbisenc ALIAS libvorbisenc)
target_include_directories(libvorbisenc
INTERFACE
    ${libvorbis_install_directory}/include
)
set_target_properties(libvorbisenc PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/vorbisenc.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/vorbisenc.lib
)
target_link_libraries(libvorbisenc
INTERFACE
    libvorbis
)
add_dependencies(libvorbisenc libvorbis_build)
