# freetype

CPMAddPackage(
    NAME libfreetype
    VERSION 2.14.1
    GITHUB_REPOSITORY freetype/freetype
    GIT_TAG VER-2-14-1
    DOWNLOAD_ONLY YES
)

if (NOT libfreetype_ADDED)
    message(FATAL_ERROR "freetype is requied")
endif ()

# prepare

set(freetype_source_directory  ${freetype_SOURCE_DIR})
set(freetype_build_directory   ${CMAKE_BINARY_DIR}/build/freetype/$<CONFIG>)
set(freetype_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(freetype_library_file      ${freetype_install_directory}/lib/freetype$<$<CONFIG:Debug>:d>.lib)

luastg_cmake_external_build_prepare_directories(include/freetype2)

# external cmake build

add_custom_command(
    OUTPUT ${freetype_library_file}
    COMMAND ${CMAKE_COMMAND} -E echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${freetype_source_directory}
        -B ${freetype_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install location
        -DCMAKE_INSTALL_PREFIX=${freetype_install_directory}
        # force utf-8
        -DCMAKE_C_FLAGS=/utf-8
        # options
        -DFT_DISABLE_ZLIB=ON
        -DFT_DISABLE_BZIP2=ON
        -DFT_DISABLE_PNG=ON
        -DFT_DISABLE_HARFBUZZ=ON
        -DFT_DISABLE_BROTLI=ON
    COMMAND ${CMAKE_COMMAND}
        --build ${freetype_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${freetype_build_directory}
        --config $<CONFIG>
        --prefix ${freetype_install_directory}
    VERBATIM
)
add_custom_target(freetype_build ALL
    DEPENDS ${freetype_library_file}
)
set_target_properties(freetype_build PROPERTIES FOLDER external/freetype)

# external cmake build clean

add_custom_target(freetype_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${freetype_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${freetype_library_file}
    VERBATIM
)
set_target_properties(freetype_clean PROPERTIES FOLDER external/freetype)

# import

add_library(freetype STATIC IMPORTED GLOBAL)
add_library(Freetype::Freetype ALIAS freetype)
target_include_directories(freetype
INTERFACE
    ${freetype_install_directory}/include
    ${freetype_install_directory}/include/freetype2
)
set_target_properties(freetype PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/freetype.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/freetyped.lib
)
add_dependencies(freetype freetype_build)
