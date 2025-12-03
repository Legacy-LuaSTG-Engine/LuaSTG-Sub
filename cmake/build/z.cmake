# zlib

CPMAddPackage(
    NAME zlib_ng # 必须用这个名称，用来适配一些SB库
    VERSION 2.2.5
    GITHUB_REPOSITORY zlib-ng/zlib-ng
    GIT_TAG 2.2.5
    DOWNLOAD_ONLY YES
)

# minizip
# 读取 zip 文件

CPMAddPackage(
    NAME minizip_ng
    VERSION 4.0.10
    GITHUB_REPOSITORY zlib-ng/minizip-ng
    GIT_TAG 4.0.10
    DOWNLOAD_ONLY YES
)

if(zlib_ng_ADDED AND minizip_ng_ADDED)
    # first, fuck cmake
    luastg_cmake_external_build_prepare_directories(include/minizip-ng)

    # then, fuck zlib ng

    set(zlib_ng_source_dir  ${zlib_ng_SOURCE_DIR})
    set(zlib_ng_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/zlib-ng/$<CONFIG>)
    set(zlib_ng_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(zlib_ng_lib_file    ${zlib_ng_install_dir}/lib/zlibstatic-ng$<$<CONFIG:Debug>:d>.lib)
    set(zlib_ng_options
        -DCMAKE_INSTALL_PREFIX=${zlib_ng_install_dir}
        -DZLIB_ENABLE_TESTS=OFF
        -DZLIBNG_ENABLE_TESTS=OFF
        -DWITH_GTEST=OFF
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
    )
    add_custom_command(
        OUTPUT ${zlib_ng_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND ${CMAKE_COMMAND}
            -S ${zlib_ng_source_dir}
            -B ${zlib_ng_build_dir}
            -G ${CMAKE_GENERATOR}
            $<$<BOOL:${CMAKE_GENERATOR_PLATFORM}>:-A> ${CMAKE_GENERATOR_PLATFORM}
            $<$<BOOL:${CMAKE_GENERATOR_TOOLSET}>:-T> ${CMAKE_GENERATOR_TOOLSET}
            ${zlib_ng_options}
        COMMAND ${CMAKE_COMMAND}
            --build ${zlib_ng_build_dir}
            --config $<CONFIG>
        COMMAND ${CMAKE_COMMAND}
            --install ${zlib_ng_build_dir}
            --config $<CONFIG>
            --prefix ${zlib_ng_install_dir}
        COMMAND ${CMAKE_COMMAND}
            -E rm -f
            ${zlib_ng_install_dir}/bin/zlib-ng$<$<CONFIG:Debug>:d>2.dll # remove dynamic libraries
        COMMAND ${CMAKE_COMMAND}
            -E touch
            ${zlib_ng_install_dir}/bin/zlib-ng$<$<CONFIG:Debug>:d>2.dll # fake dynamic library
        COMMAND ${CMAKE_COMMAND}
            -E rm -f
            ${zlib_ng_install_dir}/lib/zlib-ng$<$<CONFIG:Debug>:d>.lib  # remove dynamic libraries
        COMMAND ${CMAKE_COMMAND}
            -E copy_if_different
            ${zlib_ng_install_dir}/lib/zlibstatic-ng$<$<CONFIG:Debug>:d>.lib
            ${zlib_ng_install_dir}/lib/zlib-ng$<$<CONFIG:Debug>:d>.lib
        VERBATIM
    )
    add_custom_target(zlib_ng_build ALL
        DEPENDS ${zlib_ng_lib_file}
    )
    set_target_properties(zlib_ng_build PROPERTIES FOLDER external)

    add_custom_target(zlib_ng_clean
        COMMAND cmake -E rm -rf ${zlib_ng_build_dir}
        COMMAND cmake -E rm -f  ${zlib_ng_lib_file}
    )
    set_target_properties(zlib_ng_clean PROPERTIES FOLDER external)

    # next, fuck minizip ng

    set(minizip_ng_source_dir  ${minizip_ng_SOURCE_DIR})
    set(minizip_ng_build_dir   ${CMAKE_CURRENT_BINARY_DIR}/minizip-ng/$<CONFIG>)
    set(minizip_ng_install_dir ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(minizip_ng_lib_file    ${zlib_ng_install_dir}/lib/minizip-ng.lib)
    set(minizip_ng_options
        -DCMAKE_INSTALL_PREFIX=${minizip_ng_install_dir}
        -DMZ_COMPAT=OFF
        -DMZ_BZIP2=OFF
        -DMZ_LZMA=OFF
        -DMZ_ZSTD=OFF
        -DMZ_FETCH_LIBS=OFF
        -DZLIB_INCLUDE_DIRS=${zlib_ng_install_dir}/include # stupid mechanism 
        -DZLIBNG_LIBRARY=${zlib_ng_lib_file}               # stupid mechanism
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW # CMAKE_MSVC_RUNTIME_LIBRARY
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # force utf-8
        "-DCMAKE_C_FLAGS=/utf-8 /D_WIN32_WINNT=0x0601"
        #-DCMAKE_CXX_FLAGS=/utf-8 # minizip-ng is C lib
    )
    add_custom_command(
        OUTPUT ${minizip_ng_lib_file}
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        COMMAND ${CMAKE_COMMAND}
            -S ${minizip_ng_source_dir}
            -B ${minizip_ng_build_dir}
            -G ${CMAKE_GENERATOR}
            $<$<BOOL:${CMAKE_GENERATOR_PLATFORM}>:-A> ${CMAKE_GENERATOR_PLATFORM}
            $<$<BOOL:${CMAKE_GENERATOR_TOOLSET}>:-T> ${CMAKE_GENERATOR_TOOLSET}
            ${minizip_ng_options}
        COMMAND cmake --build   ${minizip_ng_build_dir} --config $<CONFIG> --target ALL_BUILD # magic target for MSVC
        COMMAND cmake --install ${minizip_ng_build_dir} --config $<CONFIG> --prefix ${minizip_ng_install_dir}
        VERBATIM
    )
    add_custom_target(minizip_ng_build ALL
        DEPENDS zlib_ng_build ${minizip_ng_lib_file}
    )
    set_target_properties(minizip_ng_build PROPERTIES FOLDER external)

    add_custom_target(minizip_ng_clean
        COMMAND cmake -E rm -rf ${minizip_ng_build_dir}
    )
    set_target_properties(minizip_ng_clean PROPERTIES FOLDER external)

    # finally, import targets

    add_library(zlib_ng STATIC IMPORTED GLOBAL)
    add_library(zlib-ng::zlibstatic ALIAS zlib_ng)
    target_include_directories(zlib_ng
    INTERFACE
        ${zlib_ng_install_dir}/include
    )
    set_target_properties(zlib_ng PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/zlibstatic-ng.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/zlibstatic-ngd.lib
    )
    add_dependencies(zlib_ng zlib_ng_build)

    add_library(minizip_ng STATIC IMPORTED GLOBAL)
    target_include_directories(minizip_ng
    INTERFACE
        ${minizip_ng_install_dir}/include
        ${minizip_ng_install_dir}/include/minizip-ng
    )
    set_target_properties(minizip_ng PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/minizip-ng.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/minizip-ng.lib
    )
    target_link_libraries(minizip_ng INTERFACE
        zlib_ng
        bcrypt.lib # for win32
    )
    add_dependencies(minizip_ng minizip_ng_build)
endif()
