# zlib

CPMAddPackage(
    NAME zlib_ng # 必须用这个名称，用来适配一些SB库
    VERSION 2.1.3
    GITHUB_REPOSITORY zlib-ng/zlib-ng
    GIT_TAG 2.1.3
    DOWNLOAD_ONLY YES
)

# minizip
# 读取 zip 文件

CPMAddPackage(
    NAME minizip_ng
    VERSION 4.0.0
    GITHUB_REPOSITORY zlib-ng/minizip-ng
    GIT_TAG 4.0.0
    DOWNLOAD_ONLY YES
)

# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG
# FUCK ZLIB-NG AND MINIZIP-NG

if(zlib_ng_ADDED AND minizip_ng_ADDED)
    file(WRITE ${CMAKE_BINARY_DIR}/install/include/fuck_zlib_ng_and_minizip_ng.h "")
    file(WRITE ${CMAKE_BINARY_DIR}/install/include/minizip-ng/fuck_zlib_ng_and_minizip_ng.h "")

    add_custom_target(fuck_zlib_ng_and_minizip_ng
        COMMAND echo ${CMAKE_GENERATOR}
        COMMAND echo ${CMAKE_GENERATOR_PLATFORM}
        COMMAND echo $<CONFIG>
        
        COMMAND cmake -S ${zlib_ng_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}/zlib-ng/${CMAKE_GENERATOR_PLATFORM} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} -DZLIB_ENABLE_TESTS=OFF -DZLIBNG_ENABLE_TESTS=OFF -DWITH_GTEST=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        COMMAND cmake --build   ${CMAKE_BINARY_DIR}/zlib-ng/${CMAKE_GENERATOR_PLATFORM} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${CMAKE_BINARY_DIR}/zlib-ng/${CMAKE_GENERATOR_PLATFORM} --config $<CONFIG> --prefix ${CMAKE_BINARY_DIR}/install
        COMMAND cmake -E rm -f ${CMAKE_BINARY_DIR}/install/bin/zlib-ng$<$<CONFIG:Debug>:d>2.dll
        COMMAND cmake -E rm -f ${CMAKE_BINARY_DIR}/install/lib/zlib-ng$<$<CONFIG:Debug>:d>.lib
    
        COMMAND cmake -S ${minizip_ng_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}/minizip-ng/${CMAKE_GENERATOR_PLATFORM} -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} -DMZ_COMPAT=OFF -DMZ_BZIP2=OFF -DMZ_LZMA=OFF -DMZ_ZSTD=OFF -DMZ_FETCH_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug> -DZLIB_INCLUDE_DIRS=${CMAKE_BINARY_DIR}/install/include -DZLIBNG_LIBRARY=${CMAKE_BINARY_DIR}/install/lib/zlibstatic-ng$<$<CONFIG:Debug>:d>.lib
        COMMAND cmake --build   ${CMAKE_BINARY_DIR}/minizip-ng/${CMAKE_GENERATOR_PLATFORM} --config $<CONFIG> --target ALL_BUILD
        COMMAND cmake --install ${CMAKE_BINARY_DIR}/minizip-ng/${CMAKE_GENERATOR_PLATFORM} --config $<CONFIG> --prefix ${CMAKE_BINARY_DIR}/install
    )

    set_target_properties(fuck_zlib_ng_and_minizip_ng PROPERTIES FOLDER external)
    
    add_library(zlib-ng STATIC IMPORTED GLOBAL)
    target_include_directories(zlib-ng INTERFACE
        ${CMAKE_BINARY_DIR}/install/include
    )
    set_target_properties(zlib-ng PROPERTIES
        IMPORTED_LOCATION         ${CMAKE_BINARY_DIR}/install/lib/zlibstatic-ng.lib
        IMPORTED_LOCATION_DEBUG   ${CMAKE_BINARY_DIR}/install/lib/zlibstatic-ngd.lib
        IMPORTED_LOCATION_RELEASE ${CMAKE_BINARY_DIR}/install/lib/zlibstatic-ng.lib
    )
    
    add_library(minizip-ng STATIC IMPORTED GLOBAL)
    target_include_directories(minizip-ng INTERFACE
        ${CMAKE_BINARY_DIR}/install/include/minizip-ng
    )
    set_target_properties(minizip-ng PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/install/lib/libminizip-ng.lib
    )
    target_link_libraries(minizip-ng INTERFACE
        zlib-ng
        bcrypt.lib
    )
endif()
