# libogg

CPMAddPackage(
    NAME libogg
    VERSION 1.3.6
    GITHUB_REPOSITORY xiph/ogg
    DOWNLOAD_ONLY YES
)

if (NOT libogg_ADDED)
    message(FATAL_ERROR "libogg is requied")
endif ()

# prepare

set(libogg_source_directory  ${libogg_SOURCE_DIR})
set(libogg_build_directory   ${CMAKE_BINARY_DIR}/build/libogg/$<CONFIG>)
set(libogg_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(libogg_library_file      ${libogg_install_directory}/lib/ogg.lib)

luastg_cmake_external_build_prepare_directories(include/ogg)

# external cmake build

add_custom_command(
    OUTPUT ${libogg_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${libogg_source_directory}
        -B ${libogg_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${libogg_install_directory}
        # cmake 4.0 breaking changes
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        # utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
        # options
        -DINSTALL_DOCS=OFF
        -DBUILD_TESTING=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${libogg_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${libogg_build_directory}
        --config $<CONFIG>
        --prefix ${libogg_install_directory}
    VERBATIM
)
add_custom_target(libogg_build ALL
    DEPENDS ${libogg_library_file}
)
set_target_properties(libogg_build PROPERTIES FOLDER external/libogg)

# external cmake build clean

add_custom_target(libogg_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${libogg_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${libogg_library_file}
    VERBATIM
)
set_target_properties(libogg_clean PROPERTIES FOLDER external/libogg)

# import

add_library(libogg STATIC IMPORTED GLOBAL)
add_library(Ogg::ogg ALIAS libogg)
target_include_directories(libogg
INTERFACE
    ${libogg_install_directory}/include
)
set_target_properties(libogg PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/ogg.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/ogg.lib
)
add_dependencies(libogg libogg_build)
