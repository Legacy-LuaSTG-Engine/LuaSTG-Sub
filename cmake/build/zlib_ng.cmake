# zlib-ng

CPMAddPackage(
    NAME zlib_ng
    VERSION 2.2.5
    GITHUB_REPOSITORY zlib-ng/zlib-ng
    GIT_TAG 2.2.5
    DOWNLOAD_ONLY YES
)

if (NOT zlib_ng_ADDED)
    message(FATAL_ERROR "zlib-ng is requied")
endif ()

# prepare

set(zlib_ng_source_directory  ${zlib_ng_SOURCE_DIR})
set(zlib_ng_build_directory   ${CMAKE_BINARY_DIR}/build/zlib_ng/$<CONFIG>)
set(zlib_ng_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(zlib_ng_library_file      ${zlib_ng_install_directory}/lib/zlibstatic-ng$<$<CONFIG:Debug>:d>.lib)

# external cmake build

add_custom_command(
    OUTPUT ${zlib_ng_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${zlib_ng_source_directory}
        -B ${zlib_ng_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${zlib_ng_install_directory}
        # options
        -DZLIB_ENABLE_TESTS=OFF
        -DZLIBNG_ENABLE_TESTS=OFF
        -DWITH_GTEST=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${zlib_ng_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${zlib_ng_build_directory}
        --config $<CONFIG>
        --prefix ${zlib_ng_install_directory}
    # MAGIC: replace shared library with static library, force other libraries link to zlib-ng-static
    COMMAND ${CMAKE_COMMAND}
        -E rm -f
        ${zlib_ng_install_directory}/lib/zlib-ng$<$<CONFIG:Debug>:d>.lib
    COMMAND ${CMAKE_COMMAND}
        -E copy_if_different
        ${zlib_ng_install_directory}/lib/zlibstatic-ng$<$<CONFIG:Debug>:d>.lib
        ${zlib_ng_install_directory}/lib/zlib-ng$<$<CONFIG:Debug>:d>.lib
    VERBATIM
)
add_custom_target(zlib_ng_build ALL
    DEPENDS ${zlib_ng_library_file}
)
set_target_properties(zlib_ng_build PROPERTIES FOLDER external/zlib_ng)

# external cmake build clean

add_custom_target(zlib_ng_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${zlib_ng_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${zlib_ng_library_file}
    VERBATIM
)
set_target_properties(zlib_ng_clean PROPERTIES FOLDER external/zlib_ng)

# import

add_library(zlib_ng_static STATIC IMPORTED GLOBAL)
add_library(zlib-ng::zlibstatic ALIAS zlib_ng_static)
target_include_directories(zlib_ng_static
INTERFACE
    ${zlib_ng_install_directory}/include
)
set_target_properties(zlib_ng_static PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/zlibstatic-ng.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/zlibstatic-ngd.lib
)
add_dependencies(zlib_ng_static zlib_ng_build)
