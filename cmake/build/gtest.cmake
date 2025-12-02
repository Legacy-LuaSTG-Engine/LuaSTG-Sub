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

if (gtest_ADDED)
    # gtest

    set(gtest_source_dir  ${gtest_SOURCE_DIR})
    set(gtest_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/gtest/$<CONFIG>)
    set(gtest_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(gtest_lib_file    ${gtest_install_dir}/lib/gtest.lib)
    set(gtest_options
        -DCMAKE_INSTALL_PREFIX=${gtest_install_dir}
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
    )
    add_custom_command(
        OUTPUT ${gtest_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND ${CMAKE_COMMAND}
            -S ${gtest_source_dir}
            -B ${gtest_build_dir}
            -G ${CMAKE_GENERATOR}
            $<$<BOOL:${CMAKE_GENERATOR_PLATFORM}>:-A> ${CMAKE_GENERATOR_PLATFORM}
            $<$<BOOL:${CMAKE_GENERATOR_TOOLSET}>:-T> ${CMAKE_GENERATOR_TOOLSET}
            ${gtest_options}
        COMMAND cmake --build   ${gtest_build_dir} --config $<CONFIG> --target ALL_BUILD # magic target for MSVC
        COMMAND cmake --install ${gtest_build_dir} --config $<CONFIG> --prefix ${gtest_install_dir}
        VERBATIM
    )
    add_custom_target(gtest_build ALL
        DEPENDS ${gtest_lib_file}
    )
    set_target_properties(gtest_build PROPERTIES FOLDER external)

    # finally, import targets

    add_library(gtest STATIC IMPORTED GLOBAL)
    add_library(GTest::gtest ALIAS gtest)
    target_include_directories(gtest
    INTERFACE
        ${gtest_install_dir}/include
    )
    set_target_properties(gtest PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/gtest.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/gtest.lib
    )
    add_dependencies(gtest gtest_build)

    add_library(gtest_main STATIC IMPORTED GLOBAL)
    add_library(GTest::gtest_main ALIAS gtest_main)
    target_include_directories(gtest_main
    INTERFACE
        ${gtest_install_dir}/include
    )
    set_target_properties(gtest_main PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/gtest_main.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/gtest_main.lib
    )
    target_link_libraries(gtest_main INTERFACE gtest)
    add_dependencies(gtest_main gtest_build)
endif ()
