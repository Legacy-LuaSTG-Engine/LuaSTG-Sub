#==================== debug family ====================#

add_library(spdlog STATIC IMPORTED GLOBAL)
target_compile_definitions(spdlog INTERFACE
    SPDLOG_COMPILED_LIB # prebuild
)
target_include_directories(spdlog INTERFACE
    spdlog/include
)
set_target_properties(spdlog PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/lib//spdlog.lib
)

#==================== z family ====================#

# zlib

add_library(zlib SHARED IMPORTED GLOBAL)
target_compile_definitions(zlib INTERFACE
    ZLIB_DLL # DLL
)
target_include_directories(zlib INTERFACE
    zlib/include
)
set_target_properties(zlib PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/Release/zlib.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/Release/zlib.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/Debug/zlib.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/Release/zlib.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/Debug/zlib.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/Release/zlib.lib
)

# libzip

add_library(zip SHARED IMPORTED GLOBAL)
target_include_directories(zip INTERFACE
    libzip/include
)
target_link_libraries(zip INTERFACE
    zlib
)
set_target_properties(zip PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/libzip/lib/Release/zip.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/libzip/lib/Release/zip.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/libzip/lib/Debug/zip.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/libzip/lib/Release/zip.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/libzip/lib/Debug/zip.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/libzip/lib/Release/zip.lib
)

#==================== font family ====================#

# freetype

add_library(freetype SHARED IMPORTED GLOBAL)
target_compile_definitions(freetype INTERFACE
    DLL_IMPORT # DLL
)
target_include_directories(freetype INTERFACE
    freetype/include
)
set_target_properties(freetype PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/freetype/lib/Release/freetype.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/freetype/lib/Release/freetype.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/freetype/lib/Debug/freetype.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/freetype/lib/Release/freetype.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/freetype/lib/Debug/freetype.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/freetype/lib/Release/freetype.lib
)

#==================== lua family ====================#

# luajit

add_library(luajit SHARED IMPORTED GLOBAL)
target_compile_definitions(luajit INTERFACE
    LUA_BUILD_AS_DLL # DLL
)
target_include_directories(luajit INTERFACE
    luajit/include
)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/luajit/lib/Release/lua51.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/luajit/lib/Release/lua51.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/luajit/lib/Debug/lua51.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/luajit/lib/Release/lua51.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/luajit/lib/Debug/lua51.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/luajit/lib/Release/lua51.lib
)

#==================== ogg family ====================#

# libogg

add_library(ogg SHARED IMPORTED GLOBAL)
target_include_directories(ogg INTERFACE
    libogg/include
)
set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/libogg/lib/Release/ogg.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/libogg/lib/Release/ogg.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/libogg/lib/Debug/ogg.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/libogg/lib/Release/ogg.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/libogg/lib/Debug/ogg.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/libogg/lib/Release/ogg.lib
)

# libvorbis

add_library(vorbis SHARED IMPORTED GLOBAL)
target_include_directories(vorbis INTERFACE
    libvorbis/include
)
target_link_libraries(vorbis INTERFACE
    ogg
)
set_target_properties(vorbis PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbis.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbis.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Debug/vorbis.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbis.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Debug/vorbis.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbis.lib
)

# libvorbisfile

add_library(vorbisfile SHARED IMPORTED GLOBAL)
target_include_directories(vorbisfile INTERFACE
    libvorbis/include
)
target_link_libraries(vorbisfile INTERFACE
    vorbis
)
set_target_properties(vorbisfile PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbisfile.dll
    IMPORTED_IMPLIB             ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbisfile.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Debug/vorbisfile.dll
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbisfile.dll
    IMPORTED_IMPLIB_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Debug/vorbisfile.lib
    IMPORTED_IMPLIB_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/lib/Release/vorbisfile.lib
)

#==================== math family ====================#

add_library(xmath STATIC IMPORTED GLOBAL)
target_include_directories(xmath INTERFACE
    xmath/include
)
set_target_properties(xmath PROPERTIES
    IMPORTED_LOCATION           ${CMAKE_CURRENT_SOURCE_DIR}/xmath/lib/Release/xmath.lib
    IMPORTED_LOCATION_DEBUG     ${CMAKE_CURRENT_SOURCE_DIR}/xmath/lib/Debug/xmath.lib
    IMPORTED_LOCATION_RELEASE   ${CMAKE_CURRENT_SOURCE_DIR}/xmath/lib/Release/xmath.lib
)
