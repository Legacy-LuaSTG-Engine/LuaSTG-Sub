# google test

CPMAddPackage(
    NAME gtest
    VERSION 2025.4.10
    URL https://github.com/google/googletest/archive/e90fe2485641bab0d6af4500192dc503384950d1.zip
    URL_HASH SHA256=FE5017C121F74EA8DA4A8D659A2632A844AE7BF8688B5AADF51E777409CB32E2
    DOWNLOAD_ONLY YES
)

if (NOT gtest_ADDED)
    message(FATAL_ERROR "gtest is requied")
endif ()

if (gtest_ADDED)
    # first, fuck cmake
    # cmake import target requires exist include directories, idk why

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/placeholder "")

    # then, fuck gtest

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
        COMMAND cmake -S ${gtest_source_dir} -B ${gtest_build_dir} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ${gtest_options}
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
