cmake_minimum_required(VERSION 3.15)

add_library(eyes2d SHARED)

target_compile_options(eyes2d PRIVATE
    "/std:c++17"
    "/MP"
    "/utf-8"
)

target_include_directories(eyes2d PRIVATE
    .
    ../fancylib
    ../prebuild/dxsdk/include
    ../prebuild/zlib/include
    ../prebuild/libzip/include
)

target_compile_definitions(eyes2d PRIVATE
    UNICODE
    _UNICODE
    ZLIB_DLL
    EYES2D_EXPORTS
)

target_sources(eyes2d PRIVATE
    E2DGlobal.hpp
    E2DException.hpp
    
    E2DLogSystem.hpp
    E2DLogSystem.cpp
    
    E2DMath.hpp
    E2DCodePage.hpp
    E2DFilePath.hpp
    
    E2DFileManager.hpp
    E2DFileManager.cpp
)

target_link_directories(eyes2d PRIVATE
    ../prebuild/zlib/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libzip/lib/${EXT_LIBRARY_TYPE}
)

target_link_libraries(eyes2d
    fancylib
    
    zlib.lib
    zip.lib
)
