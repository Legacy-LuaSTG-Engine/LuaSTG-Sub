# libflac

CPMAddPackage(
    NAME libflac
    VERSION 1.5.0
    GITHUB_REPOSITORY xiph/flac
    GIT_TAG 1.5.0
    DOWNLOAD_ONLY YES
)

if (NOT libflac_ADDED)
    message(FATAL_ERROR "libflac is requied")
endif ()

# prepare

set(libflac_source_directory  ${libflac_SOURCE_DIR})
set(libflac_build_directory   ${CMAKE_BINARY_DIR}/build/libflac/$<CONFIG>)
set(libflac_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(libflac_library_file      ${libflac_install_directory}/lib/FLAC.lib)

luastg_cmake_external_build_prepare_directories(include/FLAC)

# external cmake build

add_custom_command(
    OUTPUT ${libflac_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${libflac_source_directory}
        -B ${libflac_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${libflac_install_directory}
        # utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
        # options
        -DBUILD_CXXLIBS=OFF
        -DBUILD_PROGRAMS=OFF
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTING=OFF
        -DBUILD_DOCS=OFF
        -DINSTALL_MANPAGES=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${libflac_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${libflac_build_directory}
        --config $<CONFIG>
        --prefix ${libflac_install_directory}
    # no FLAC++
    COMMAND ${CMAKE_COMMAND}
        -E rm -rf ${libflac_install_directory}/include/FLAC++
    VERBATIM
)
add_custom_target(libflac_build ALL
    DEPENDS ${libflac_library_file}
)
add_dependencies(libflac_build libogg_build)
set_target_properties(libflac_build PROPERTIES FOLDER external/libflac)

# external cmake build clean

add_custom_target(libflac_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${libflac_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${libflac_library_file}
    VERBATIM
)
set_target_properties(libflac_clean PROPERTIES FOLDER external/libflac)

# import

add_library(libflac STATIC IMPORTED GLOBAL)
add_library(FLAC::FLAC ALIAS libflac)
target_compile_definitions(libflac
INTERFACE
    FLAC__NO_DLL
)
target_include_directories(libflac
INTERFACE
    ${libflac_install_directory}/include
)
set_target_properties(libflac PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/FLAC.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/FLAC.lib
)
target_link_libraries(libflac
INTERFACE
    libogg
)
add_dependencies(libflac libflac_build)
