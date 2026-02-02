# minizip-ng

CPMAddPackage(
    NAME minizip_ng
    VERSION 4.1.0
    GITHUB_REPOSITORY zlib-ng/minizip-ng
    GIT_TAG 4.1.0
    DOWNLOAD_ONLY YES
)

if (NOT minizip_ng_ADDED)
    message(FATAL_ERROR "minizip-ng is requied")
endif ()

# prepare

set(zlib_ng_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(zlib_ng_library_file      ${zlib_ng_install_directory}/lib/zlibstatic-ng$<$<CONFIG:Debug>:d>.lib)

set(minizip_ng_source_directory  ${minizip_ng_SOURCE_DIR})
set(minizip_ng_build_directory   ${CMAKE_BINARY_DIR}/build/minizip_ng/$<CONFIG>)
set(minizip_ng_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(minizip_ng_library_file      ${minizip_ng_install_directory}/lib/minizip-ng.lib)

luastg_cmake_external_build_prepare_directories(include/minizip-ng)

# external cmake build

add_custom_command(
    OUTPUT ${minizip_ng_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${minizip_ng_source_directory}
        -B ${minizip_ng_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${minizip_ng_install_directory}
        # force utf-8, win7
        "-DCMAKE_C_FLAGS=/utf-8 /D_WIN32_WINNT=0x0601"
        # options
        -DMZ_COMPAT=OFF
        -DMZ_BZIP2=OFF
        -DMZ_LZMA=OFF
        -DMZ_ZSTD=OFF
        -DMZ_FETCH_LIBS=OFF
        -DMZ_ZLIB_FLAVOR=zlib-ng
        -DZLIB-NG_INCLUDE_DIRS=${zlib_ng_install_directory}/include  # stupid mechanism 
        -DZLIB-NG_LIBRARY=${zlib_ng_library_file}                    # stupid mechanism
    COMMAND ${CMAKE_COMMAND}
        --build ${minizip_ng_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${minizip_ng_build_directory}
        --config $<CONFIG>
        --prefix ${minizip_ng_install_directory}
    VERBATIM
)
add_custom_target(minizip_ng_build ALL
    DEPENDS ${minizip_ng_library_file}
)
add_dependencies(minizip_ng_build zlib_ng_build)
set_target_properties(minizip_ng_build PROPERTIES FOLDER external/minizip_ng)

# external cmake build clean

add_custom_target(minizip_ng_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${minizip_ng_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${minizip_ng_library_file}
    VERBATIM
)
set_target_properties(minizip_ng_clean PROPERTIES FOLDER external/minizip_ng)

# import

add_library(minizip_ng STATIC IMPORTED GLOBAL)
target_include_directories(minizip_ng
INTERFACE
    ${minizip_ng_install_directory}/include
    ${minizip_ng_install_directory}/include/minizip-ng
)
set_target_properties(minizip_ng PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/minizip-ng.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/minizip-ng.lib
)
target_link_libraries(minizip_ng INTERFACE
    zlib-ng::zlibstatic
    bcrypt.lib # for win32
)
add_dependencies(minizip_ng minizip_ng_build)
