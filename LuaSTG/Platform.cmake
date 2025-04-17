# Windows Platform

set(PLATFORM_SOURCES
    Platform/Shared.hpp
    Platform/RuntimeLoader/DXGI.hpp
    Platform/RuntimeLoader/D3DKMT.hpp
    Platform/RuntimeLoader/D3DKMT.cpp
    Platform/RuntimeLoader/Direct3D11.hpp
    Platform/RuntimeLoader/Direct2D1.hpp
    Platform/RuntimeLoader/Direct3DCompiler.hpp
    Platform/RuntimeLoader/DirectComposition.hpp
    Platform/RuntimeLoader/DirectWrite.hpp
    Platform/RuntimeLoader/DesktopWindowManager.hpp
    Platform/RuntimeLoader/XAudio2.hpp
    Platform/RuntimeLoader/All.cpp
    Platform/CommandLineArguments.hpp
    Platform/DesktopWindowManager.hpp
    Platform/Direct3D11.hpp
    Platform/DXGI.hpp
    Platform/ModuleLoader.hpp
    Platform/All.cpp
    Platform/ApplicationSingleInstance.hpp

    Platform/CleanWindows.hpp
    Platform/KnownDirectory.hpp
    Platform/KnownDirectory.cpp
    Platform/WindowSizeMoveController.hpp
    Platform/WindowSizeMoveController.cpp
    Platform/DirectInput.hpp
    Platform/DirectInput.cpp
    Platform/WindowsVersion.hpp
    Platform/WindowsVersion.cpp
    Platform/WindowTheme.hpp
    Platform/WindowTheme.cpp
    Platform/Keyboard.hpp
    Platform/Keyboard.cpp
    Platform/HResultChecker.hpp
    Platform/HResultChecker.cpp
    #Platform/DetectCPU.hpp
    #Platform/DetectCPU.cpp
    Platform/ProcessorInfo.hpp
    Platform/ProcessorInfo.cpp
    Platform/XInput.hpp
    Platform/XInput.cpp
    Platform/AdapterPolicy.hpp
    Platform/AdapterPolicy.cpp
    Platform/MessageBox.hpp
    Platform/MessageBox.cpp
    Platform/ImmersiveTitleBarController.hpp
    Platform/ImmersiveTitleBarController.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${PLATFORM_SOURCES})

add_library(PlatformAPI STATIC)
luastg_target_common_options(PlatformAPI)
luastg_target_more_warning(PlatformAPI)
target_include_directories(PlatformAPI PUBLIC
    .
)
target_sources(PlatformAPI PRIVATE
    ${PLATFORM_SOURCES}
)
target_link_libraries(PlatformAPI PRIVATE
    Microsoft.Windows.ImplementationLibrary
    utf8
    beautiful_win32_api
)
