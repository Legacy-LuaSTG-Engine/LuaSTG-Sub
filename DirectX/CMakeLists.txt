#==================== DirectXTK family ====================#

add_library(DirectXTex0 STATIC)
luastg_target_common_options(DirectXTex0)
target_include_directories(DirectXTex0 PUBLIC
    DirectXTex
    DirectXTex/ScreenGrab
    DirectXTex/WICTextureLoader
)
target_sources(DirectXTex0 PRIVATE
    DirectXTex/ScreenGrab/ScreenGrab9.h
    DirectXTex/ScreenGrab/ScreenGrab9.cpp
    DirectXTex/ScreenGrab/ScreenGrab11.h
    DirectXTex/ScreenGrab/ScreenGrab11.cpp
    DirectXTex/WICTextureLoader/WICTextureLoader9.h
    DirectXTex/WICTextureLoader/WICTextureLoader9.cpp
    DirectXTex/WICTextureLoader/WICTextureLoader11.h
    DirectXTex/WICTextureLoader/WICTextureLoader11.cpp
)
target_link_libraries(DirectXTex0 PUBLIC
    dxguid.lib
)

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
