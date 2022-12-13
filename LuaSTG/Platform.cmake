# Windows Platform

set(PLATFORM_SOURCES
    Platform/Shared.hpp
    Platform/RuntimeLoader/DXGI.hpp
    Platform/RuntimeLoader/Direct3D11.hpp
    Platform/RuntimeLoader/Direct2D1.hpp
    Platform/RuntimeLoader/DirectComposition.hpp
    Platform/RuntimeLoader/DirectWrite.hpp
    Platform/RuntimeLoader/DesktopWindowManager.hpp
    Platform/RuntimeLoader/All.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${PLATFORM_SOURCES})

add_library(PlatformAPI STATIC)
target_compile_options(PlatformAPI PRIVATE
    "/MP"
    "/utf-8"
    "/W4"
)
set_target_properties(PlatformAPI PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_compile_definitions(PlatformAPI PRIVATE
    _UNICODE
    UNICODE
)
target_include_directories(PlatformAPI PUBLIC
    .
)
target_sources(PlatformAPI PRIVATE
    ${PLATFORM_SOURCES}
)
