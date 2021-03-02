#==================== legecy DXSDK ====================#

# DXSDK (must remove it in the future!)

add_library(DirectXSDK INTERFACE IMPORTED GLOBAL) # load dll at runtime
target_include_directories(DirectXSDK INTERFACE
    DirectXSDK/include
)

#==================== DirectXTK family ====================#

add_library(DirectXToolkit STATIC)
target_compile_options(DirectXToolkit PRIVATE
    "/MP"
)
target_compile_definitions(DirectXToolkit PRIVATE
    UNICODE
    _UNICODE
)
target_include_directories(DirectXToolkit PUBLIC
    .
)
target_sources(DirectXToolkit PRIVATE
    ScreenGrab9.cpp
    ScreenGrab9.h
    WICTextureLoader9.cpp
    WICTextureLoader9.h
)
