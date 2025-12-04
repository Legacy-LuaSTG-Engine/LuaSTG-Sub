# google test

CPMAddPackage(
    NAME gtest
    VERSION 1.70.0
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    URL_HASH SHA256=65FAB701D9829D38CB77C14ACDC431D2108BFDBF8979E40EB8AE567EDF10B27C
    DOWNLOAD_ONLY YES
)

if (NOT gtest_ADDED)
    message(FATAL_ERROR "gtest is requied")
endif ()

# gtest

set(gtest_source_directory  ${gtest_SOURCE_DIR})
set(gtest_build_directory   ${CMAKE_BINARY_DIR}/build/gtest/$<CONFIG>)
set(gtest_install_directory ${CMAKE_BINARY_DIR}/install/$<CONFIG>)
set(gtest_library_file      ${gtest_install_directory}/lib/gtest.lib)

# external cmake build

add_custom_command(
    OUTPUT ${gtest_library_file}
    COMMAND ${CMAKE_COMMAND} -E echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${gtest_source_directory}
        -B ${gtest_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install location
        -DCMAKE_INSTALL_PREFIX=${gtest_install_directory}
    COMMAND ${CMAKE_COMMAND}
        --build ${gtest_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${gtest_build_directory}
        --config $<CONFIG>
        --prefix ${gtest_install_directory}
    VERBATIM
)
add_custom_target(gtest_build ALL
    DEPENDS ${gtest_library_file}
)
set_target_properties(gtest_build PROPERTIES FOLDER external/gtest)

# external cmake build clean

add_custom_target(gtest_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${gtest_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${gtest_library_file}
    VERBATIM
)
set_target_properties(gtest_clean PROPERTIES FOLDER external/gtest)

# import 

## GTest::gtest

add_library(gtest STATIC IMPORTED GLOBAL)
add_library(GTest::gtest ALIAS gtest)
target_include_directories(gtest
INTERFACE
    ${gtest_install_directory}/include
)
set_target_properties(gtest PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/gtest.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/gtest.lib
)
add_dependencies(gtest gtest_build)

## GTest::gtest_main

add_library(gtest_main STATIC IMPORTED GLOBAL)
add_library(GTest::gtest_main ALIAS gtest_main)
target_include_directories(gtest_main
INTERFACE
    ${gtest_install_directory}/include
)
set_target_properties(gtest_main PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_BINARY_DIR}/install/Release/lib/gtest_main.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/install/Debug/lib/gtest_main.lib
)
target_link_libraries(gtest_main INTERFACE gtest)
add_dependencies(gtest_main gtest_build)
