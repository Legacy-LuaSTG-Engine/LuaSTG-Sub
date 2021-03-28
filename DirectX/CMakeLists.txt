#==================== legecy DXSDK ====================#

# DXSDK (must remove it in the future!)

add_library(DirectXSDK INTERFACE IMPORTED GLOBAL) # load dll at runtime
target_include_directories(DirectXSDK INTERFACE
    DirectXSDK/include
    )

#==================== DirectXTK family ====================#

add_library(DirectXTex0 STATIC)
target_compile_options(DirectXTex0 PRIVATE
    "/MP"
    )
target_compile_definitions(DirectXTex0 PRIVATE
    UNICODE
    _UNICODE
    )
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
    dxguid.lib)
