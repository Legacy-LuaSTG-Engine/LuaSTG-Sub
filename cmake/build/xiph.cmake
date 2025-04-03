# libogg

CPMAddPackage(
    NAME libogg
    VERSION 1.3.5
    GITHUB_REPOSITORY xiph/ogg
    DOWNLOAD_ONLY YES
)

if(libogg_ADDED)
    set(libogg_source_dir  ${libogg_SOURCE_DIR})
    set(libogg_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/libogg/$<CONFIG>)
    set(libogg_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(libogg_lib_file    ${libogg_install_dir}/lib/ogg.lib)
    set(libogg_options
        # install location
        -DCMAKE_INSTALL_PREFIX=${libogg_install_dir}
        # cmake 4.0 breaking changes
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # force utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
        # libogg options
        -DINSTALL_DOCS=OFF
        -DBUILD_TESTING=OFF
    )
    add_custom_command(
        OUTPUT ${libogg_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND cmake -S ${libogg_source_dir} -B ${libogg_build_dir} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ${libogg_options}
        # magic target for MSVC
        COMMAND cmake --build   ${libogg_build_dir} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${libogg_build_dir} --config $<CONFIG> --prefix ${libogg_install_dir}
        VERBATIM
    )
    add_custom_target(libogg_build ALL
        DEPENDS ${libogg_lib_file}
    )
    add_custom_target(libogg_clean
        COMMAND cmake -E rm -rf ${libogg_build_dir}
        COMMAND cmake -E rm -f ${libogg_lib_file}
        VERBATIM
    )
    set_target_properties(libogg_build PROPERTIES FOLDER external)
    set_target_properties(libogg_clean PROPERTIES FOLDER external)

    # fuck cmake
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/ogg/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/ogg/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/ogg/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/ogg/placeholder "")

    # libogg
    add_library(libogg STATIC IMPORTED GLOBAL)
    add_library(Ogg::ogg ALIAS libogg)
    target_include_directories(libogg
    INTERFACE
        ${libogg_install_dir}/include
    )
    set_target_properties(libogg PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/ogg.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/ogg.lib
    )
    add_dependencies(libogg libogg_build)
endif()

# libvorbis

CPMAddPackage(
    NAME libvorbis
    VERSION 1.3.7
    GITHUB_REPOSITORY xiph/vorbis
    DOWNLOAD_ONLY YES
)

if(libvorbis_ADDED)
    set(libvorbis_source_dir  ${libvorbis_SOURCE_DIR})
    set(libvorbis_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/libvorbis/$<CONFIG>)
    set(libvorbis_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(libvorbis_lib_file    ${libvorbis_install_dir}/lib/vorbis.lib)
    set(libvorbis_options
        # install location
        -DCMAKE_INSTALL_PREFIX=${libvorbis_install_dir}
        # cmake 4.0 breaking changes
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # force utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
        # libvorbis options
    )
    add_custom_command(
        OUTPUT ${libvorbis_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND cmake -S ${libvorbis_source_dir} -B ${libvorbis_build_dir} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ${libvorbis_options}
        # magic target for MSVC
        COMMAND cmake --build   ${libvorbis_build_dir} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${libvorbis_build_dir} --config $<CONFIG> --prefix ${libvorbis_install_dir}
        VERBATIM
    )
    add_custom_target(libvorbis_build ALL
        DEPENDS libogg_build ${libvorbis_lib_file}
    )
    add_custom_target(libvorbis_clean
        COMMAND cmake -E rm -rf ${libvorbis_build_dir}
        COMMAND cmake -E rm -f ${libvorbis_lib_file}
        VERBATIM
    )
    set_target_properties(libvorbis_build PROPERTIES FOLDER external)
    set_target_properties(libvorbis_clean PROPERTIES FOLDER external)

    # fuck cmake
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/vorbis/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/vorbis/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/vorbis/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/vorbis/placeholder "")

    # vorbis
    add_library(libvorbis STATIC IMPORTED GLOBAL)
    add_library(Vorbis::vorbis ALIAS libvorbis)
    target_include_directories(libvorbis
    INTERFACE
        ${libvorbis_install_dir}/include
    )
    set_target_properties(libvorbis PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/vorbis.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/vorbis.lib
    )
    target_link_libraries(libvorbis
    INTERFACE
        libogg
    )
    add_dependencies(libvorbis libvorbis_build)

    # vorbisfile
    add_library(libvorbisfile STATIC IMPORTED GLOBAL)
    add_library(Vorbis::vorbisfile ALIAS libvorbisfile)
    target_include_directories(libvorbisfile
    INTERFACE
        ${libvorbis_install_dir}/include
    )
    set_target_properties(libvorbisfile PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/vorbisfile.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/vorbisfile.lib
    )
    target_link_libraries(libvorbisfile
    INTERFACE
        libvorbis
    )
    add_dependencies(libvorbisfile libvorbis_build)

    # vorbisenc
    add_library(libvorbisenc STATIC IMPORTED GLOBAL)
    add_library(Vorbis::vorbisenc ALIAS libvorbisenc)
    target_include_directories(libvorbisenc
    INTERFACE
        ${libvorbis_install_dir}/include
    )
    set_target_properties(libvorbisenc PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/vorbisenc.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/vorbisenc.lib
    )
    target_link_libraries(libvorbisenc
    INTERFACE
        libvorbis
    )
    add_dependencies(libvorbisenc libvorbis_build)
endif()

# libflac

CPMAddPackage(
    NAME libflac
    VERSION 1.5.0
    GITHUB_REPOSITORY xiph/flac
    GIT_TAG 1.5.0
    DOWNLOAD_ONLY YES
)

if(libflac_ADDED)
    set(libflac_source_dir  ${libflac_SOURCE_DIR})
    set(libflac_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/libflac/$<CONFIG>)
    set(libflac_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(libflac_lib_file    ${libflac_install_dir}/lib/FLAC.lib)
    set(libflac_options
        # install location
        -DCMAKE_INSTALL_PREFIX=${libflac_install_dir}
        # msvc runtime library
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # force utf-8
        -DCMAKE_C_FLAGS=/utf-8
        -DCMAKE_CXX_FLAGS=/utf-8
        # libflac options
        -DBUILD_CXXLIBS=OFF
        -DBUILD_PROGRAMS=OFF
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTING=OFF
        -DBUILD_DOCS=OFF
        -DINSTALL_MANPAGES=OFF
    )
    add_custom_command(
        OUTPUT ${libflac_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND cmake -S ${libflac_source_dir} -B ${libflac_build_dir} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ${libflac_options}
        # magic target for MSVC
        COMMAND cmake --build   ${libflac_build_dir} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${libflac_build_dir} --config $<CONFIG> --prefix ${libflac_install_dir}
        # fuck libflac
        COMMAND cmake -E rm -rf ${libflac_install_dir}/include/FLAC++
        VERBATIM
    )
    add_custom_target(libflac_build ALL
        DEPENDS libogg_build ${libflac_lib_file}
    )
    add_custom_target(libflac_clean
        COMMAND cmake -E rm -rf ${libflac_build_dir}
        COMMAND cmake -E rm -f ${libflac_lib_file}
        VERBATIM
    )
    set_target_properties(libflac_build PROPERTIES FOLDER external)
    set_target_properties(libflac_clean PROPERTIES FOLDER external)

    # fuck cmake
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/FLAC/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/FLAC/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/FLAC/placeholder "")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/FLAC/placeholder "")

    # libflac
    add_library(libflac STATIC IMPORTED GLOBAL)
    add_library(FLAC::FLAC ALIAS libflac)
    target_compile_definitions(libflac
    INTERFACE
        FLAC__NO_DLL
    )
    target_include_directories(libflac
    INTERFACE
        ${libflac_install_dir}/include
    )
    set_target_properties(libflac PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/FLAC.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/FLAC.lib
    )
    target_link_libraries(libflac
    INTERFACE
        libogg
    )
    add_dependencies(libflac libflac_build)
endif()
