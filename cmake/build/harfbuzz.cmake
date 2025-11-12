# libharfbuzz

CPMAddPackage(
    NAME libharfbuzz
    VERSION 11.0.0
    GITHUB_REPOSITORY harfbuzz/harfbuzz
    GIT_TAG 11.0.0
    DOWNLOAD_ONLY YES
)

if(libharfbuzz_ADDED)
    set(libharfbuzz_source_dir  ${libharfbuzz_SOURCE_DIR})
    set(libharfbuzz_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/libharfbuzz/$<CONFIG>)
    set(libharfbuzz_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(libharfbuzz_lib_file    ${libharfbuzz_install_dir}/lib/harfbuzz.lib)
    set(libharfbuzz_options
        # install location
        -DCMAKE_INSTALL_PREFIX=${libharfbuzz_install_dir}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # force utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
        # libharfbuzz options
    )
    add_custom_command(
        OUTPUT ${libharfbuzz_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND ${CMAKE_COMMAND}
            -S ${libharfbuzz_source_dir}
            -B ${libharfbuzz_build_dir}
            -G ${CMAKE_GENERATOR}
            $<$<BOOL:${CMAKE_GENERATOR_PLATFORM}>:-A> ${CMAKE_GENERATOR_PLATFORM}
            $<$<BOOL:${CMAKE_GENERATOR_TOOLSET}>:-T> ${CMAKE_GENERATOR_TOOLSET}
            ${libharfbuzz_options}
        # magic target for MSVC
        COMMAND cmake --build   ${libharfbuzz_build_dir} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${libharfbuzz_build_dir} --config $<CONFIG> --prefix ${libharfbuzz_install_dir}
        VERBATIM
    )
    add_custom_target(libharfbuzz_build ALL
        DEPENDS ${libharfbuzz_lib_file}
    )
    add_custom_target(libharfbuzz_clean
        COMMAND cmake -E rm -rf ${libharfbuzz_build_dir}
        COMMAND cmake -E rm -f ${libharfbuzz_lib_file}
        VERBATIM
    )
    set_target_properties(libharfbuzz_build PROPERTIES FOLDER external)
    set_target_properties(libharfbuzz_clean PROPERTIES FOLDER external)

    # fuck cmake
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/harfbuzz/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/harfbuzz/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/harfbuzz/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/harfbuzz/placeholder "")

    # libharfbuzz
    add_library(libharfbuzz STATIC IMPORTED GLOBAL)
    add_library(harfbuzz::harfbuzz ALIAS libharfbuzz)
    target_include_directories(libharfbuzz
    INTERFACE
        ${libharfbuzz_install_dir}/include
    )
    set_target_properties(libharfbuzz PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/harfbuzz.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/harfbuzz.lib
    )
    add_dependencies(libharfbuzz libharfbuzz_build)
endif()