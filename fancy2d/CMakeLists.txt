# 源文件

set(FANCY2D_SOURCE
    f2d.h
    f2dInterface.h

    f2dEngine.h
    f2dWindow.h
    f2dSoundSys.h
    f2dRenderDevice.h
    f2dRenderer.h

    #Build/fancy2d.manifest
    Build/pch.h
    Build/pch.cpp

    Engine/f2dEngineImpl.h
    Engine/f2dEngineImpl.cpp
    Engine/f2dWindowImpl.h
    Engine/f2dWindowImpl.cpp

    RenderDev/f2dRenderDevice11.h
    RenderDev/f2dRenderDevice11.cpp
    RenderDev/f2dTexture11.h
    RenderDev/f2dTexture11.cpp

    Renderer/f2dRendererImpl.h
    Renderer/f2dRendererImpl.cpp
    Renderer/f2dGeometryRendererImpl.h
    Renderer/f2dGeometryRendererImpl.cpp
    Renderer/f2dSpriteImpl.h
    Renderer/f2dSpriteImpl.cpp
    Renderer/f2dSpriteAnimationImpl.h
    Renderer/f2dSpriteAnimationImpl.cpp
    Renderer/f2dParticle.h
    Renderer/f2dParticle.cpp
    Renderer/f2dFontRendererImpl.h
    Renderer/f2dFontRendererImpl.cpp
    Renderer/f2dFontTexProvider.h
    Renderer/f2dFontTexProvider.cpp
    Renderer/f2dTrueTypeFontProvider.h
    Renderer/f2dTrueTypeFontProvider.cpp

    Sound/f2dSoundSysImpl.h
    Sound/f2dSoundSysImpl.cpp
    Sound/f2dWaveDecoder.h
    Sound/f2dWaveDecoder.cpp
    Sound/f2dVorbisDecoder.h
    Sound/f2dVorbisDecoder.cpp
    Sound/f2dAudioBufferStatic.h
    Sound/f2dAudioBufferStatic.cpp
    Sound/f2dAudioBufferDynamic.h
    Sound/f2dAudioBufferDynamic.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX fancy2d FILES ${FANCY2D_SOURCE})

# 静态库

add_library(fancy2dlib STATIC)

luastg_target_common_options(fancy2dlib)
luastg_target_more_warning(fancy2dlib)

target_compile_definitions(fancy2dlib PRIVATE
    ____EMPTY_0000_0000_0000_0000____
)

target_precompile_headers(fancy2dlib PRIVATE
    Build/pch.h
)

target_include_directories(fancy2dlib PUBLIC
    .
)
target_include_directories(fancy2dlib PRIVATE
    ../
)

target_sources(fancy2dlib PRIVATE
    ${FANCY2D_SOURCE}
)

target_link_libraries(fancy2dlib PUBLIC
    spdlog::spdlog
    winmm.lib
    imm32.lib
    dxguid.lib
    dinput8.lib
    DirectX::XAudio2Redist
    DirectXTexMini
    dxgi.lib
    d3d11.lib
    freetype
    vorbisfile
    fancylib
    xmath
    platform
    utility
    tracy
)
