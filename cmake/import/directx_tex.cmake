# Microsoft.DirectXTex

CPMAddPackage(
    NAME Microsoft.DirectXTex
    VERSION 2024.10.1
    GITHUB_REPOSITORY microsoft/DirectXTex
    GIT_TAG oct2024
    DOWNLOAD_ONLY YES
)

if(Microsoft.DirectXTex_ADDED)
    set(DirectXTex_DIR ${Microsoft.DirectXTex_SOURCE_DIR})
    add_library(DirectXTexMini STATIC)
    luastg_target_common_options(DirectXTexMini)
    luastg_target_platform_windows_7(DirectXTexMini)
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

    add_library(DirectX::TextureMini ALIAS DirectXTexMini)
    set_target_properties(DirectXTexMini PROPERTIES FOLDER external)
else()
    message(FATAL_ERROR "Microsoft.DirectXTex is requied")
endif()
