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
    Core/DataObject.cpp

    Core/FileManager.hpp
    Core/FileManager.cpp
    Core/InitializeConfigure.hpp
    Core/InitializeConfigure.cpp

    Core/Graphics/Window.hpp
    Core/Graphics/Window_Win32.hpp
    Core/Graphics/Window_Win32.cpp
    Core/Graphics/Format.hpp
    Core/Graphics/Format_D3D11.hpp
    Core/Graphics/Device.hpp
    Core/Graphics/Device_D3D11.hpp
    Core/Graphics/Device_D3D11.cpp
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
    Core/ApplicationModel.hpp
    Core/ApplicationModel_Win32.hpp
    Core/ApplicationModel_Win32.cpp

    Core/Audio/Decoder.hpp
    Core/Audio/Decoder_VorbisOGG.cpp
    Core/Audio/Decoder_VorbisOGG.hpp
    Core/Audio/Decoder_FLAC.hpp
    Core/Audio/Decoder_FLAC.cpp
    Core/Audio/Decoder_WAV.cpp
    Core/Audio/Decoder_WAV.hpp
    Core/Audio/Decoder_ALL.cpp
    Core/Audio/Device.hpp
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
    spdlog::spdlog
    tracy
    imgui
    # util
    utility
    platform
    # win32
    winmm.lib
    imm32.lib
    # dx
    dxguid.lib
    dxgi.lib
    d3d11.lib
    d3dcompiler.lib
    DirectX::XAudio2Redist
    DirectXTexMini
    libqoi
    # math
    xmath
    # file
    MINIZIP::minizip
    # text
    freetype
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
)
