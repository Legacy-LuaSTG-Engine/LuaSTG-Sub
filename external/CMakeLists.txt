# ==================== spdlog ====================

option(SPDLOG_ENABLE_PCH "Build static or shared library using precompiled header to speed up compilation time" ON)
option(SPDLOG_BUILD_WARNINGS "Enable compiler warnings" ON)
option(SPDLOG_SANITIZE_ADDRESS "Enable address sanitizer in tests" ON)
#option(SPDLOG_WCHAR_SUPPORT "Support wchar api" ON)
option(SPDLOG_WCHAR_FILENAMES "Support wchar filenames" ON)
option(SPDLOG_DISABLE_DEFAULT_LOGGER "Disable default logger creation" ON)
add_subdirectory(spdlog)
target_compile_options(spdlog PUBLIC
    "/DSPDLOG_SHORT_LEVEL_NAMES={\"V\",\"D\",\"I\",\"W\",\"E\",\"F\",\"O\"}"
)

set_target_properties(spdlog PROPERTIES FOLDER external)

# ==================== json ====================

add_subdirectory(json)

# ==================== DirectXTK family ====================#

add_library(DirectXTexMini STATIC)
luastg_target_common_options(DirectXTexMini)
target_include_directories(DirectXTexMini PUBLIC
    DirectXTex
    DirectXTex/ScreenGrab
    DirectXTex/WICTextureLoader
)
target_sources(DirectXTexMini PRIVATE
    DirectXTex/ScreenGrab/ScreenGrab11.h
    DirectXTex/ScreenGrab/ScreenGrab11.cpp
    DirectXTex/WICTextureLoader/WICTextureLoader11.h
    DirectXTex/WICTextureLoader/WICTextureLoader11.cpp
)
target_link_libraries(DirectXTexMini PUBLIC
    dxguid.lib
)

set_target_properties(DirectXTexMini PROPERTIES FOLDER external)

# ==================== XAudio2 ====================

add_library(XAudio2Redist SHARED IMPORTED GLOBAL)
target_include_directories(XAudio2Redist INTERFACE
    XAudio2/build/native/include
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(XAudio2Redist PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/XAudio2/build/native/release/lib/x64/xaudio2_9redist.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/XAudio2/build/native/release/bin/x64/xaudio2_9redist.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_target_properties(XAudio2Redist PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/XAudio2/build/native/release/lib/x86/xaudio2_9redist.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/XAudio2/build/native/release/bin/x86/xaudio2_9redist.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()
add_library(DirectX::XAudio2Redist ALIAS XAudio2Redist)

# ==================== luajit ====================

add_subdirectory(luajit)
set_target_properties(minilua PROPERTIES FOLDER luajit)
set_target_properties(buildvm PROPERTIES FOLDER luajit)
set_target_properties(lua51static PROPERTIES FOLDER luajit)
set_target_properties(lua51 PROPERTIES FOLDER luajit)
set_target_properties(luajit PROPERTIES FOLDER luajit)

# ==================== lua filesystem ====================

add_library(lua_filesystem STATIC)
luastg_target_common_options2(lua_filesystem)
target_compile_definitions(lua_filesystem PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
target_include_directories(lua_filesystem PUBLIC
    lua-filesystem-patch
)
target_sources(lua_filesystem PRIVATE
    lua-filesystem-patch/lfs.h
    lua-filesystem-patch/lfs.c
)
target_link_libraries(lua_filesystem PUBLIC
    lua51static
)

set_target_properties(lua_filesystem PROPERTIES FOLDER lualib)

# ==================== lua cjson ====================

add_library(lua_cjson STATIC)
luastg_target_common_options(lua_cjson)
target_compile_definitions(lua_cjson PRIVATE
    _CRT_SECURE_NO_WARNINGS
    DISABLE_INVALID_NUMBERS
    ENABLE_CJSON_GLOBAL
    #snprintf=_snprintf
    #strncasecmp=_strnicmp
    strncasecmp=strnicmp
)
target_include_directories(lua_cjson PUBLIC
    lua-cjson-patch
    lua-cjson
)
target_sources(lua_cjson PRIVATE
    lua-cjson-patch/lua_cjson.h
    lua-cjson-patch/lua_cjson.c
    lua-cjson/strbuf.h
    lua-cjson/strbuf.c
    lua-cjson/fpconv.h
    lua-cjson/fpconv.c
)
target_link_libraries(lua_cjson PUBLIC
    lua51static
)

set_target_properties(lua_cjson PROPERTIES FOLDER lualib)

# ==================== pugixml ====================

#add_subdirectory(pugixml)
#
#set_target_properties(pugixml-static PROPERTIES FOLDER external)

# ==================== lua csv ====================

#add_library(lua_xlsx_csv STATIC)
#luastg_target_common_options(lua_xlsx_csv)
#target_include_directories(lua_xlsx_csv PUBLIC
#    lua-csv
#)
#target_sources(lua_xlsx_csv PRIVATE
#    lua-csv/lua_xlsx_csv.h
#    lua-csv/lua_xlsx.cpp
#    lua-csv/lua_csv.cpp
#)
#target_link_libraries(lua_xlsx_csv PUBLIC
#    luajit
#    zip
#    pugixml
#)
#
#set_target_properties(lua_xlsx_csv PROPERTIES FOLDER external)

# ==================== xmath ====================

add_library(xmath STATIC)
luastg_target_common_options(xmath)
target_include_directories(xmath PUBLIC
    xmath-patch
    xmath
    .
)
target_sources(xmath PRIVATE
    xmath-patch/math/Vec2.h
    xmath-patch/math/Vec2.cpp
    xmath/meow_fft.c
    xmath/meow_fft.h
    xmath/XCollision.cpp
    xmath/XCollision.h
    xmath/XComplex.cpp
    xmath/XComplex.h
    xmath/XConstant.h
    xmath/XDistance.cpp
    xmath/XDistance.h
    xmath/XEquation.cpp
    xmath/XEquation.h
    xmath/XFFT.cpp
    xmath/XFFT.h
    xmath/XIntersect.cpp
    xmath/XIntersect.h
    xmath/XMath.h
    xmath/XRandom.cpp
    xmath/XRandom.h
    xmath/XSpline.cpp
    xmath/XSpline.h
    xmath/XTween.cpp
    xmath/XTween.h
)

set_target_properties(xmath PROPERTIES FOLDER external)

# ==================== freetype ====================

add_subdirectory(freetype)
set_target_properties(freetype PROPERTIES FOLDER external)

# ==================== xiph ====================

set(BUILD_TESTING OFF)
add_subdirectory(libogg)
set_target_properties(ogg PROPERTIES FOLDER external)
add_subdirectory(libvorbis)
set_target_properties(vorbis PROPERTIES FOLDER external)
set_target_properties(vorbisfile PROPERTIES FOLDER external)
set_target_properties(vorbisenc PROPERTIES FOLDER external)

# ==================== zlib-ng ====================

option(ZLIB_ENABLE_TESTS "Build test binaries" OFF)
option(WITH_AVX2 "Build with AVX2" OFF)
option(WITH_AVX512 "Build with AVX512" OFF)
option(WITH_AVX512VNNI "Build with AVX512 VNNI extensions" OFF)
option(WITH_SSSE3 "Build with SSSE3" OFF)
option(WITH_SSE41 "Build with SSE41" OFF)
option(WITH_SSE42 "Build with SSE42" OFF)
option(WITH_PCLMULQDQ "Build with PCLMULQDQ" OFF)
option(WITH_VPCLMULQDQ "Build with VPCLMULQDQ" OFF)
add_subdirectory(zlib)
if(TARGET zlib-ng)
    set_target_properties(zlib-ng PROPERTIES FOLDER external)
else()
    set_target_properties(zlib PROPERTIES FOLDER external)
endif()

# ==================== minizip-ng ====================

option(MZ_COMPAT "Enables compatibility layer" OFF)
option(MZ_FETCH_LIBS "Enables fetching third-party libraries if not found" OFF)
set(SKIP_INSTALL_ALL ON)
add_subdirectory(minizip)
set_target_properties(minizip-ng PROPERTIES FOLDER external)
