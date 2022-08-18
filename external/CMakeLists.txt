include(AutoDownload.cmake)

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

# ==================== d3dcompiler ====================#

add_library(D3DCompilerRedist MODULE IMPORTED GLOBAL)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(D3DCompilerRedist PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/D3DRedist/amd64/d3dcompiler_47.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_target_properties(D3DCompilerRedist PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/D3DRedist/x86/d3dcompiler_47.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()
add_library(DirectX::D3DCompilerRedist ALIAS D3DCompilerRedist)

# ==================== DirectXTK ====================#

add_library(DirectXTKMini STATIC)
luastg_target_common_options(DirectXTKMini)
target_include_directories(DirectXTKMini PUBLIC
    DirectXTK/Inc
)
target_sources(DirectXTKMini PRIVATE
    DirectXTK/Inc/Keyboard.h
    DirectXTK/Inc/Mouse.h
    DirectXTK/Src/Keyboard.cpp
    DirectXTK/Src/Mouse.cpp
    DirectXTK/Src/pch.h
    DirectXTK/Src/pch.cpp
    DirectXTK/Src/PlatformHelpers.h
)
add_library(DirectX::ToolKitMini ALIAS DirectXTKMini)

set_target_properties(DirectXTKMini PROPERTIES FOLDER external)

# ==================== DirectXTex ====================#

add_library(DirectXTexMini STATIC)
luastg_target_common_options(DirectXTexMini)
target_include_directories(DirectXTexMini PUBLIC
    DirectXTex
    DirectXTex/ScreenGrab
    DirectXTex/WICTextureLoader
    DirectXTex/DDSTextureLoader
)
target_sources(DirectXTexMini PRIVATE
    DirectXTex/ScreenGrab/ScreenGrab11.h
    DirectXTex/ScreenGrab/ScreenGrab11.cpp
    DirectXTex/WICTextureLoader/WICTextureLoader11.h
    DirectXTex/WICTextureLoader/WICTextureLoader11.cpp
    DirectXTex/DDSTextureLoader/DDSTextureLoader11.h
    DirectXTex/DDSTextureLoader/DDSTextureLoader11.cpp
)
target_link_libraries(DirectXTexMini PUBLIC
    dxguid.lib
)

set_target_properties(DirectXTexMini PROPERTIES FOLDER external)

# ==================== XAudio2 ====================

add_library(XAudio2Redist SHARED IMPORTED GLOBAL)
target_include_directories(XAudio2Redist INTERFACE
    XAudio2Redist/build/native/include
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(XAudio2Redist PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/XAudio2Redist/build/native/release/lib/x64/xaudio2_9redist.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/XAudio2Redist/build/native/release/bin/x64/xaudio2_9redist.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_target_properties(XAudio2Redist PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/XAudio2Redist/build/native/release/lib/x86/xaudio2_9redist.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/XAudio2Redist/build/native/release/bin/x86/xaudio2_9redist.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()
add_library(DirectX::XAudio2Redist ALIAS XAudio2Redist)

# ==================== dr libs ====================

add_library(dr_libs STATIC)
luastg_target_common_options(dr_libs)
target_compile_definitions(dr_libs PUBLIC
    ____DR_WAV_NO_STDIO
)
target_include_directories(dr_libs PUBLIC
    dr_libs
)
target_sources(dr_libs PRIVATE
    dr_libs/dr_wav.h
    dr_libs-patch/dr_wav.cpp
)

set_target_properties(dr_libs PROPERTIES FOLDER external)

# ==================== luajit ====================

add_subdirectory(luajit)
set_target_properties(minilua PROPERTIES FOLDER luajit)
set_target_properties(buildvm PROPERTIES FOLDER luajit)
set_target_properties(lua51static PROPERTIES FOLDER luajit)
set_target_properties(lua51 PROPERTIES FOLDER luajit)
set_target_properties(luajit PROPERTIES FOLDER luajit)

# ==================== lua filesystem ====================

add_library(lua_filesystem STATIC)
luastg_target_common_options(lua_filesystem)
luastg_target_more_warning(lua_filesystem)
target_include_directories(lua_filesystem PUBLIC
    lua-filesystem-lite
)
target_sources(lua_filesystem PRIVATE
    lua-filesystem-lite/lfs.h
    lua-filesystem-lite/lfs.cpp
)
target_link_libraries(lua_filesystem PUBLIC
    lua51static
)

set_target_properties(lua_filesystem PROPERTIES FOLDER lualib)

# ==================== lua cjson ====================

add_library(lua_cjson STATIC)
luastg_target_common_options(lua_cjson)
target_compile_options(lua_cjson PRIVATE
    "/wd4090" # 不同的const限定符（指针传递）
)
target_compile_definitions(lua_cjson PRIVATE
    _CRT_SECURE_NO_WARNINGS
    DISABLE_INVALID_NUMBERS
    ENABLE_CJSON_GLOBAL
)
target_include_directories(lua_cjson PUBLIC
    lua-cjson-patch
)
target_include_directories(lua_cjson PRIVATE
    lua-cjson
)
target_sources(lua_cjson PRIVATE
    lua-cjson-patch/lua_cjson.h
    lua-cjson/lua_cjson.c
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
target_compile_options(freetype PRIVATE
    "/utf-8" # Unicode warning
)
set_target_properties(freetype PROPERTIES FOLDER external)

# ==================== xiph ====================

option(BUILD_TESTING "No Testing!" OFF)
add_subdirectory(libogg)
set_target_properties(ogg PROPERTIES FOLDER external)
add_subdirectory(libvorbis)
set_target_properties(vorbis PROPERTIES FOLDER external)
set_target_properties(vorbisfile PROPERTIES FOLDER external)
set_target_properties(vorbisenc PROPERTIES FOLDER external)

option(BUILD_CXXLIBS "Build libFLAC++" OFF)
option(BUILD_PROGRAMS "Build and install programs" OFF)
option(BUILD_EXAMPLES "Build and install examples" OFF)
option(BUILD_TESTING "Build tests" OFF)
option(BUILD_DOCS "Build and install doxygen documents" OFF)
option(INSTALL_MANPAGES "Install MAN pages" OFF)
option(INSTALL_PKGCONFIG_MODULES "Install PkgConfig modules" OFF)
option(INSTALL_CMAKE_CONFIG_MODULE "Install CMake package-config module" OFF)
option(WITH_AVX "Enable AVX, AVX2 optimizations (with runtime detection, resulting binary does not require AVX2)" OFF)
add_subdirectory(libflac)
set_target_properties(getopt PROPERTIES FOLDER external/flac)
set_target_properties(grabbag PROPERTIES FOLDER external/flac)
set_target_properties(FLAC PROPERTIES FOLDER external/flac)
set_target_properties(replaygain_analysis PROPERTIES FOLDER external/flac)
set_target_properties(replaygain_synthesis PROPERTIES FOLDER external/flac)
set_target_properties(utf8 PROPERTIES FOLDER external/flac)

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

# ==================== tinygltf ====================

option(TINYGLTF_BUILD_LOADER_EXAMPLE "Build loader_example(load glTF and dump infos)" OFF)
option(TINYGLTF_INSTALL "Install tinygltf files during install step. Usually set to OFF if you include tinygltf through add_subdirectory()" OFF)
add_subdirectory(tinygltf)
set_target_properties(tinygltf PROPERTIES FOLDER external)

# ==================== tracy ====================

option(ENABLE_TRACY_CLIENT "" OFF)

add_library(tracy STATIC)
luastg_target_common_options(tracy)
if(ENABLE_TRACY_CLIENT)
    target_compile_definitions(tracy PUBLIC
        TRACY_ENABLE
    )
endif()
target_include_directories(tracy PUBLIC
    tracy
    tracy-patch
)
target_sources(tracy PRIVATE
    tracy/Tracy.hpp
    tracy/TracyD3D11.hpp
    tracy/TracyClient.cpp
    tracy-patch/TracyD3D11Ctx.hpp
    tracy-patch/TracyD3D11Ctx.cpp
)

set_target_properties(tracy PROPERTIES FOLDER external)

# ==================== pcg random ====================

add_library(pcgcpp STATIC)
target_include_directories(pcgcpp PUBLIC
    pcg-cpp/include
)
target_sources(pcgcpp PRIVATE
    pcg-cpp/include/pcg_extras.hpp
    pcg-cpp/include/pcg_random.hpp
    pcg-cpp/include/pcg_uint128.hpp
    pcg-cpp-patch/pcg_cpp.cpp
)

set_target_properties(pcgcpp PROPERTIES FOLDER external)
