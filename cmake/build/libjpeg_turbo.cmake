# libjpeg-turbo

CPMAddPackage(
    NAME libjpeg_turbo
    VERSION 3.1.2
    GITHUB_REPOSITORY libjpeg-turbo/libjpeg-turbo
    GIT_TAG 3.1.2
    DOWNLOAD_ONLY YES
)

if (NOT libjpeg_turbo_ADDED)
    message(FATAL_ERROR "libjpeg-turbo is requied")
endif ()

# prepare

set(libjpeg_source_directory  ${libjpeg_turbo_SOURCE_DIR})
set(libjpeg_build_directory   ${CMAKE_BINARY_DIR}/build/libjpeg_turbo)
set(libjpeg_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(libjpeg_library_files
    ${libjpeg_install_directory}/lib/turbojpeg-static.lib
    ${libjpeg_install_directory}/lib/jpeg-static.lib
)

# external cmake build

add_custom_command(
    OUTPUT ${libjpeg_library_files}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${libjpeg_source_directory}
        -B ${libjpeg_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        -DCMAKE_BUILD_TYPE=$<CONFIG>
        # install
        -DCMAKE_INSTALL_PREFIX=${libjpeg_install_directory}
        # options
        -DENABLE_SHARED=OFF
        -DWITH_TOOLS=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${libjpeg_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${libjpeg_build_directory}
        --config $<CONFIG>
        --prefix ${libjpeg_install_directory}
    VERBATIM
)
add_custom_target(libjpeg_build ALL
    DEPENDS ${libjpeg_library_files}
)
set_target_properties(libjpeg_build PROPERTIES FOLDER external/libjpeg)

# external cmake build clean

add_custom_target(libjpeg_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${libjpeg_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -r  ${libjpeg_library_files}
)
set_target_properties(libjpeg_clean PROPERTIES FOLDER external/libjpeg)

# import targets

## libjpeg-turbo::turbojpeg-static

add_library(libjpeg_turbo_turbojpeg_static STATIC IMPORTED GLOBAL)
add_library(libjpeg-turbo::turbojpeg-static ALIAS libjpeg_turbo_turbojpeg_static)
target_include_directories(libjpeg_turbo_turbojpeg_static
INTERFACE
    ${libjpeg_install_directory}/include
)
set_target_properties(libjpeg_turbo_turbojpeg_static PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/turbojpeg-static.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/turbojpeg-static.lib
)
add_dependencies(libjpeg_turbo_turbojpeg_static libjpeg_build)

## libjpeg-turbo::jpeg-static

add_library(libjpeg_turbo_jpeg_static STATIC IMPORTED GLOBAL)
add_library(libjpeg-turbo::jpeg-static ALIAS libjpeg_turbo_jpeg_static)
target_include_directories(libjpeg_turbo_jpeg_static
INTERFACE
    ${libjpeg_install_directory}/include
)
set_target_properties(libjpeg_turbo_jpeg_static PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/jpeg-static.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/jpeg-static.lib
)
add_dependencies(libjpeg_turbo_jpeg_static libjpeg_build)
