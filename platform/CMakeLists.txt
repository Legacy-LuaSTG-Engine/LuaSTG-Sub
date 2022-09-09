add_library(platform STATIC)
luastg_target_common_options(platform)
luastg_target_more_warning(platform)
target_include_directories(platform PUBLIC
    .
)
target_include_directories(platform PRIVATE
    platform/pch
    platform
)
target_precompile_headers(platform PRIVATE
    platform/pch/pch.h
)
set(platform_sources
    platform/pch/pch.h
    platform/pch/pch.cpp
    platform/HighDPI.hpp
    platform/HighDPI.cpp
    platform/KnownDirectory.hpp
    platform/KnownDirectory.cpp
    platform/CommandLine.hpp
    platform/CommandLine.cpp
    platform/Monitor.hpp
    platform/Monitor.cpp
    platform/WindowSizeMoveController.hpp
    platform/WindowSizeMoveController.cpp
    platform/DirectInput.hpp
    platform/DirectInput.cpp
    platform/WindowsVersion.hpp
    platform/WindowsVersion.cpp
    platform/WindowTheme.hpp
    platform/WindowTheme.cpp
    platform/Keyboard.hpp
    platform/Keyboard.cpp
    platform/HResultChecker.hpp
    platform/HResultChecker.cpp
    platform/DetectCPU.hpp
    platform/DetectCPU.cpp
    platform/XInput.hpp
    platform/XInput.cpp
    platform/AdapterPolicy.hpp
    platform/AdapterPolicy.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${platform_sources})
target_sources(platform PRIVATE ${platform_sources})
