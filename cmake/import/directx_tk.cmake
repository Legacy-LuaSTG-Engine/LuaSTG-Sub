# Microsoft.DirectXTK

CPMAddPackage(
    NAME Microsoft.DirectXTK
    VERSION 2024.10.1
    GITHUB_REPOSITORY microsoft/DirectXTK
    GIT_TAG oct2024
    DOWNLOAD_ONLY YES
)

if(Microsoft.DirectXTK_ADDED)
    set(DirectXTK_DIR ${Microsoft.DirectXTK_SOURCE_DIR})
    add_library(DirectXTKMini STATIC)
    luastg_target_common_options(DirectXTKMini)
    luastg_target_platform_windows_7(DirectXTKMini)
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
