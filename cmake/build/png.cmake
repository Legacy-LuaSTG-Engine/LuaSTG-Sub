# libpng

CPMAddPackage(
    NAME libpng
    VERSION 1.6.55
    GITHUB_REPOSITORY Legacy-LuaSTG-Engine/libpng-zlib-ng
    GIT_TAG v1.6.55-zlib-ng
    DOWNLOAD_ONLY YES
)

if (NOT libpng_ADDED)
    message(FATAL_ERROR "libpng is required")
endif ()

# prepare

set(libpng_source_directory  ${libpng_SOURCE_DIR})
set(libpng_build_directory   ${CMAKE_BINARY_DIR}/build/libpng)
set(libpng_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(libpng_library_file      ${libpng_install_directory}/lib/libpng16_static.lib)

luastg_cmake_external_build_prepare_directories(include/libpng16)

# external cmake build

add_custom_command(
    OUTPUT ${libpng_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${libpng_source_directory}
        -B ${libpng_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # crt
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${libpng_install_directory}
        # options
        -DBUILD_SHARED_LIBS=OFF
        -DPNG_DEBUG_POSTFIX=
        -DPNG_SHARED=OFF
        -DPNG_TESTS=OFF
        -DPNG_TOOLS=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${libpng_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${libpng_build_directory}
        --config $<CONFIG>
        --prefix ${libpng_install_directory}
    VERBATIM
)
add_custom_target(libpng_build ALL
    DEPENDS ${libpng_library_file}
)
add_dependencies(libpng_build zlib_ng_build)
set_target_properties(libpng_build PROPERTIES FOLDER external/libpng)

# external cmake build clean

add_custom_target(libpng_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${libpng_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -r  ${libpng_library_file}
)
set_target_properties(libpng_clean PROPERTIES FOLDER external/libpng)

# import

add_library(png_static STATIC IMPORTED GLOBAL)
add_library(PNG::png_static ALIAS png_static)
target_include_directories(png_static
INTERFACE
    ${libpng_install_directory}/include
    ${libpng_install_directory}/include/libpng16
)
target_link_libraries(png_static
INTERFACE
    zlib-ng::zlibstatic
)
set_target_properties(png_static PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/libpng16_static.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/libpng16_static.lib
)
add_dependencies(png_static libpng_build)
