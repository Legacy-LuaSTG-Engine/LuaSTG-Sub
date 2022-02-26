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

#==================== DirectXTK family ====================#

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

#==================== XAudio2 ====================#

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
