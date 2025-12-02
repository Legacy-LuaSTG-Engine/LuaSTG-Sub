# libwebp

CPMAddPackage(
    NAME libwebp
    VERSION 1.6.0
    GITHUB_REPOSITORY webmproject/libwebp
    GIT_TAG v1.6.0
    DOWNLOAD_ONLY YES
)

if (NOT libwebp_ADDED)
    message(FATAL_ERROR "libwebp is required")
endif ()

# prepare

luastg_cmake_external_build_prepare_directories(include/webp)

set(libwebp_source_directory  ${libwebp_SOURCE_DIR})
set(libwebp_build_directory   ${CMAKE_BINARY_DIR}/build/libwebp)
set(libwebp_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(libwebp_library_file      ${libwebp_install_directory}/lib/libwebp.lib)

# external cmake build

add_custom_command(
    OUTPUT ${libwebp_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${libwebp_source_directory}
        -B ${libwebp_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # crt
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${libwebp_install_directory}
        # options
        -DBUILD_SHARED_LIBS=OFF
        -DWEBP_BUILD_ANIM_UTILS=OFF
        -DWEBP_BUILD_CWEBP=OFF
        -DWEBP_BUILD_DWEBP=OFF
        -DWEBP_BUILD_GIF2WEBP=OFF
        -DWEBP_BUILD_IMG2WEBP=OFF
        -DWEBP_BUILD_VWEBP=OFF
        -DWEBP_BUILD_WEBPINFO=OFF
        -DWEBP_BUILD_WEBPMUX=OFF
        -DWEBP_BUILD_EXTRAS=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${libwebp_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${libwebp_build_directory}
        --config $<CONFIG>
        --prefix ${libwebp_install_directory}
    VERBATIM
)
add_custom_target(libwebp_build ALL
    DEPENDS ${libwebp_library_file}
)
set_target_properties(libwebp_build PROPERTIES FOLDER external/libwebp)

# external cmake build clean

add_custom_target(libwebp_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${libwebp_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -r  ${libwebp_library_file}
)
set_target_properties(libwebp_clean PROPERTIES FOLDER external/libwebp)

# import targets

## WebP::sharpyuv

add_library(webp_sharpyuv STATIC IMPORTED GLOBAL)
add_library(WebP::sharpyuv ALIAS webp_sharpyuv)
target_include_directories(webp_sharpyuv
INTERFACE
    ${libwebp_install_directory}/include
    ${libwebp_install_directory}/include/webp
)
set_target_properties(webp_sharpyuv PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/libsharpyuv.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/libsharpyuv.lib
)
add_dependencies(webp_sharpyuv libwebp_build)

## WebP::webpdecoder

add_library(webp_webpdecoder STATIC IMPORTED GLOBAL)
add_library(WebP::webpdecoder ALIAS webp_webpdecoder)
target_include_directories(webp_webpdecoder
INTERFACE
    ${libwebp_install_directory}/include
)
target_link_libraries(webp_webpdecoder
INTERFACE
    shlwapi.lib
    ole32.lib
    windowscodecs.lib
)
set_target_properties(webp_webpdecoder PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/libwebpdecoder.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/libwebpdecoder.lib
)
add_dependencies(webp_webpdecoder libwebp_build)

## WebP::webp

add_library(webp_webp STATIC IMPORTED GLOBAL)
add_library(WebP::webp ALIAS webp_webp)
target_include_directories(webp_webp
INTERFACE
    ${libwebp_install_directory}/include
)
target_link_libraries(webp_webp
INTERFACE
    WebP::sharpyuv
    shlwapi.lib
    ole32.lib
    windowscodecs.lib
)
set_target_properties(webp_webp PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/libwebp.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/libwebp.lib
)
add_dependencies(webp_webp libwebp_build)

## WebP::webpdemux

add_library(webp_webpdemux STATIC IMPORTED GLOBAL)
add_library(WebP::webpdemux ALIAS webp_webpdemux)
target_include_directories(webp_webpdemux
INTERFACE
    ${libwebp_install_directory}/include
)
target_link_libraries(webp_webpdemux
INTERFACE
    WebP::webp
)
set_target_properties(webp_webpdemux PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/libwebpdemux.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/libwebpdemux.lib
)
add_dependencies(webp_webpdemux libwebp_build)

## WebP::libwebpmux

add_library(webp_libwebpmux STATIC IMPORTED GLOBAL)
add_library(WebP::libwebpmux ALIAS webp_libwebpmux)
target_include_directories(webp_libwebpmux
INTERFACE
    ${libwebp_install_directory}/include
)
target_link_libraries(webp_libwebpmux
INTERFACE
    WebP::webp
)
set_target_properties(webp_libwebpmux PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/libwebpmux.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/libwebpmux.lib
)
add_dependencies(webp_libwebpmux libwebp_build)
