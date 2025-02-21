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
    Core/Type.hpp

    Core/i18n.hpp
    Core/i18n.cpp
    Core/framework.hpp
    Core/framework.cpp
    Core/Object.hpp
    Core/Object.cpp
    Core/DataObject.cpp

    Core/FileManager.hpp
    Core/FileManager.cpp

    Core/Graphics/Window.hpp
    Core/Graphics/Window_Win32.hpp
    Core/Graphics/Window_Win32.cpp
    Core/Graphics/Format.hpp
    Core/Graphics/Format_D3D11.hpp
    Core/Graphics/Device.hpp
    Core/Graphics/SwapChain.hpp
    Core/Graphics/SwapChain_D3D11.hpp
    Core/Graphics/SwapChain_D3D11.cpp
    Core/Graphics/Renderer.hpp
    Core/Graphics/Renderer_D3D11.hpp
    Core/Graphics/Renderer_D3D11.cpp
    Core/Graphics/Renderer_Shader_D3D11.cpp
    Core/Graphics/Model_D3D11.hpp
    Core/Graphics/Model_D3D11.cpp
    Core/Graphics/Model_Shader_D3D11.cpp
    Core/Graphics/Sprite.hpp
    Core/Graphics/Sprite_D3D11.hpp
    Core/Graphics/Sprite_D3D11.cpp
    Core/Graphics/Font.hpp
    Core/Graphics/Font_D3D11.hpp
    Core/Graphics/Font_D3D11.cpp
    Core/Graphics/DearImGui_Win32_D3D11.hpp
    Core/Graphics/DearImGui_Win32_D3D11.cpp

    Core/Graphics/Direct3D11/Buffer.hpp
    Core/Graphics/Direct3D11/Buffer.cpp
    Core/Graphics/Direct3D11/Texture2D.hpp
    Core/Graphics/Direct3D11/Texture2D.cpp
    Core/Graphics/Direct3D11/SamplerState.hpp
    Core/Graphics/Direct3D11/SamplerState.cpp
    Core/Graphics/Direct3D11/RenderTarget.hpp
    Core/Graphics/Direct3D11/RenderTarget.cpp
    Core/Graphics/Direct3D11/DepthStencilBuffer.hpp
    Core/Graphics/Direct3D11/DepthStencilBuffer.cpp
    Core/Graphics/Direct3D11/Device.hpp
    Core/Graphics/Direct3D11/Device.cpp
    Core/Graphics/Direct3D11/LetterBoxingRenderer.hpp
    Core/Graphics/Direct3D11/LetterBoxingRenderer.cpp

    Core/Application.hpp
    Core/ApplicationModel.hpp
    Core/ApplicationModel_Win32.hpp
    Core/ApplicationModel_Win32.cpp
    Core/EventDispatcherImpl.hpp

    Core/Audio/Decoder.hpp
    Core/Audio/Decoder_VorbisOGG.cpp
    Core/Audio/Decoder_VorbisOGG.hpp
    Core/Audio/Decoder_FLAC.hpp
    Core/Audio/Decoder_FLAC.cpp
    Core/Audio/Decoder_WAV.cpp
    Core/Audio/Decoder_WAV.hpp
    Core/Audio/Decoder_ALL.cpp
    Core/Audio/Device.hpp
    Core/Audio/Device_MM.cpp
    Core/Audio/Device_XAUDIO2.cpp
    Core/Audio/Device_XAUDIO2.hpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${Core_SRC})
target_precompile_headers(Core PRIVATE
    Core/framework.hpp
)
target_sources(Core PRIVATE
    ${Core_SRC}
)

target_link_libraries(Core PUBLIC
    # debug
    spdlog
    TracyAPI
    imgui
    # util
    utility
    utf8
    simdutf::simdutf
    PlatformAPI
    beautiful_win32_api
    GeneratedShaderHeaders
    # win32
    winmm.lib
    imm32.lib
    # dx
    dxguid.lib
    dxgi.lib
    d3d11.lib
    #Microsoft.XAudio2.Redist
    Microsoft.Windows.ImplementationLibrary
    DirectX::TextureMini
    libqoi
    # math
    xmath
    # file
    minizip_ng
    # text
    Freetype::Freetype
    # model
    tinygltf
    # audio
    dr_libs
    Ogg::ogg
    Vorbis::vorbis
    Vorbis::vorbisfile
    FLAC::FLAC
    # database
    nlohmann_json
    LuaSTG.Sub.Configuration
)
