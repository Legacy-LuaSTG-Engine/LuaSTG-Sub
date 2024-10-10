# libfreetype

CPMAddPackage(
    NAME libfreetype
    VERSION 2.13.3
    GITHUB_REPOSITORY freetype/freetype
    GIT_TAG VER-2-13-3
    DOWNLOAD_ONLY YES
)

if(libfreetype_ADDED)
    set(libfreetype_source_dir  ${libfreetype_SOURCE_DIR})
    set(libfreetype_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/libfreetype/$<CONFIG>)
    set(libfreetype_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(libfreetype_lib_file    ${libfreetype_install_dir}/lib/freetype$<$<CONFIG:Debug>:d>.lib)
    set(libfreetype_options
        # install location
        -DCMAKE_INSTALL_PREFIX=${libfreetype_install_dir}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # force utf-8
        -DCMAKE_C_FLAGS=/utf-8
        #-DCMAKE_CXX_FLAGS=/utf-8  # freetype is C lib
        # libfreetype options
        -DFT_DISABLE_ZLIB=ON
        -DFT_DISABLE_BZIP2=ON
        -DFT_DISABLE_PNG=ON
        -DFT_DISABLE_HARFBUZZ=ON
        -DFT_DISABLE_BROTLI=ON
    )
    add_custom_command(
        OUTPUT ${libfreetype_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND cmake -S ${libfreetype_source_dir} -B ${libfreetype_build_dir} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ${libfreetype_options}
        # magic target for MSVC
        COMMAND cmake --build   ${libfreetype_build_dir} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${libfreetype_build_dir} --config $<CONFIG> --prefix ${libfreetype_install_dir}
        VERBATIM
    )
    add_custom_target(libfreetype_build ALL
        DEPENDS ${libfreetype_lib_file}
    )
    add_custom_target(libfreetype_clean
        COMMAND cmake -E rm -rf ${libfreetype_build_dir}
        COMMAND cmake -E rm -f ${libfreetype_lib_file}
        VERBATIM
    )
    set_target_properties(libfreetype_build PROPERTIES FOLDER external)
    set_target_properties(libfreetype_clean PROPERTIES FOLDER external)

    # fuck cmake
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/freetype2/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/freetype2/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/freetype2/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/freetype2/placeholder "")

    # libfreetype
    add_library(libfreetype STATIC IMPORTED GLOBAL)
    add_library(Freetype::Freetype ALIAS libfreetype)
    target_include_directories(libfreetype
    INTERFACE
        ${libfreetype_install_dir}/include
        ${libfreetype_install_dir}/include/freetype2
    )
    set_target_properties(libfreetype PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/freetype.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/freetyped.lib
    )
    add_dependencies(libfreetype libfreetype_build)
endif()
