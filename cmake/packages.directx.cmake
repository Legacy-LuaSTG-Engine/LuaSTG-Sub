# Microsoft.DirectXTK

CPMAddPackage(
    NAME Microsoft.DirectXTK
    VERSION 2023.4.28
    GITHUB_REPOSITORY microsoft/DirectXTK
    GIT_TAG apr2023
    DOWNLOAD_ONLY YES
)

if(Microsoft.DirectXTK_ADDED)
    set(DirectXTK_DIR ${Microsoft.DirectXTK_SOURCE_DIR})
    add_library(DirectXTKMini STATIC)
    luastg_target_common_options(DirectXTKMini)
    target_include_directories(DirectXTKMini PUBLIC
        ${DirectXTK_DIR}/Inc
    )
    target_sources(DirectXTKMini PRIVATE
        ${DirectXTK_DIR}/Inc/Keyboard.h
        ${DirectXTK_DIR}/Inc/Mouse.h
        ${DirectXTK_DIR}/Src/Keyboard.cpp
        ${DirectXTK_DIR}/Src/Mouse.cpp
        ${DirectXTK_DIR}/Src/pch.h
        ${DirectXTK_DIR}/Src/pch.cpp
        ${DirectXTK_DIR}/Src/PlatformHelpers.h
    )
    add_library(DirectX::ToolKitMini ALIAS DirectXTKMini)

    set_target_properties(DirectXTKMini PROPERTIES FOLDER external)
else()
    message(FATAL_ERROR "Microsoft.DirectXTK is requied")
endif()

# Microsoft.DirectXTex

CPMAddPackage(
    NAME Microsoft.DirectXTex
    VERSION 2023.4.28
    GITHUB_REPOSITORY microsoft/DirectXTex
    GIT_TAG apr2023
    DOWNLOAD_ONLY YES
)

if(Microsoft.DirectXTex_ADDED)
    set(DirectXTex_DIR ${Microsoft.DirectXTex_SOURCE_DIR})
    add_library(DirectXTexMini STATIC)
    luastg_target_common_options(DirectXTexMini)
    target_include_directories(DirectXTexMini PUBLIC
        ${DirectXTex_DIR}
        ${DirectXTex_DIR}/ScreenGrab
        ${DirectXTex_DIR}/WICTextureLoader
        ${DirectXTex_DIR}/DDSTextureLoader
    )
    target_sources(DirectXTexMini PRIVATE
        ${DirectXTex_DIR}/ScreenGrab/ScreenGrab11.h
        ${DirectXTex_DIR}/ScreenGrab/ScreenGrab11.cpp
        ${DirectXTex_DIR}/WICTextureLoader/WICTextureLoader11.h
        ${DirectXTex_DIR}/WICTextureLoader/WICTextureLoader11.cpp
        ${DirectXTex_DIR}/DDSTextureLoader/DDSTextureLoader11.h
        ${DirectXTex_DIR}/DDSTextureLoader/DDSTextureLoader11.cpp
    )
    target_link_libraries(DirectXTexMini PUBLIC
        dxguid.lib
    )
    
    set_target_properties(DirectXTexMini PROPERTIES FOLDER external)
endif()
