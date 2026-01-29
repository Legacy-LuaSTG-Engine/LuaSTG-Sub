# Graphics

set(lib_name "Core.Graphics")

file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR} core/*.hpp core/*.cpp d3d11/*.hpp d3d11/*.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

add_library(${lib_name})
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PRIVATE options_compile_utf8)
target_link_libraries(${lib_name} PUBLIC
    Core.ReferenceCounted
    Core.Configuration
    Core.FileSystem
    Core.Logging
    Core.Math
    Core.Image
    win32
)

set_target_properties(${lib_name} PROPERTIES FOLDER engine)

# Core

add_library(Core STATIC)

luastg_target_common_options(Core)
luastg_target_more_warning(Core)
target_compile_definitions(Core PRIVATE
    LUASTG_CORE_USING_IMGUI
)
target_include_directories(Core PUBLIC
    .
)

set(Core_SRC
    d3d11/pch.h
    d3d11/pch.cpp

    core/Graphics/Format.hpp
    core/Graphics/Device.hpp
    core/Graphics/SwapChain.hpp
    core/Graphics/SwapChain_D3D11.hpp
    core/Graphics/SwapChain_D3D11.cpp
    core/Graphics/Renderer.hpp
    core/Graphics/Renderer_D3D11.hpp
    core/Graphics/Renderer_D3D11.cpp
    core/Graphics/Renderer_Shader_D3D11.cpp
    core/Graphics/Model_D3D11.hpp
    core/Graphics/Model_D3D11.cpp
    core/Graphics/Model_Shader_D3D11.cpp
    core/Graphics/Sprite.hpp
    core/Graphics/Font.hpp
    core/Graphics/Mesh.hpp

    core/Graphics/Common/Sprite.hpp
    core/Graphics/Common/Sprite.cpp
    core/Graphics/Common/SpriteRenderer.hpp
    core/Graphics/Common/SpriteRenderer.cpp
    core/Graphics/Common/FreeTypeGlyphManager.hpp
    core/Graphics/Common/FreeTypeGlyphManager.cpp
    core/Graphics/Common/TextRenderer.hpp
    core/Graphics/Common/TextRenderer.cpp
    core/Graphics/Common/TextRenderer2.hpp
    core/Graphics/Common/TextRenderer2.cpp

    core/Graphics/Direct3D11/Constants.hpp
    core/Graphics/Direct3D11/Buffer.hpp
    core/Graphics/Direct3D11/Buffer.cpp
    core/Graphics/Direct3D11/Texture2D.hpp
    core/Graphics/Direct3D11/Texture2D.cpp
    core/Graphics/Direct3D11/SamplerState.hpp
    core/Graphics/Direct3D11/SamplerState.cpp
    core/Graphics/Direct3D11/RenderTarget.hpp
    core/Graphics/Direct3D11/RenderTarget.cpp
    core/Graphics/Direct3D11/DepthStencilBuffer.hpp
    core/Graphics/Direct3D11/DepthStencilBuffer.cpp
    core/Graphics/Direct3D11/Device.hpp
    core/Graphics/Direct3D11/Device.cpp
    core/Graphics/Direct3D11/Mesh.hpp
    core/Graphics/Direct3D11/Mesh.cpp
    core/Graphics/Direct3D11/MeshRenderer.hpp
    core/Graphics/Direct3D11/MeshRenderer.cpp
    core/Graphics/Direct3D11/LetterBoxingRenderer.hpp
    core/Graphics/Direct3D11/LetterBoxingRenderer.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${Core_SRC})
target_precompile_headers(Core PRIVATE
    d3d11/pch.h
)
target_sources(Core PRIVATE
    ${Core_SRC}
)

target_link_libraries(Core PUBLIC
    # debug
    TracyAPI
    # util
    utility
    utf8
    simdutf::simdutf
    # win32
    winmm.lib
    imm32.lib
    # dx
    dxguid.lib
    dxgi.lib
    d3d11.lib
    Microsoft.Windows.ImplementationLibrary
    Microsoft::DirectXTexMini
    GeneratedShaderHeaders
    libqoi
    # math
    xmath
    # text
    Freetype::Freetype
    # model
    tinygltf
    # engine
    Core.Math
    Core.String
    Core.Configuration
    Core.ReferenceCounted
    Core.FileSystem
    Core.Image
    Core.Font
    Core.WindowSystem
    win32
    beautiful_win32_api
)
