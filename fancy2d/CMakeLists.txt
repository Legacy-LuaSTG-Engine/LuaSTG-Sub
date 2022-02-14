# 源文件

set(FANCY2D_SOURCE
    f2dConfig.h
    f2d.h
    f2dInterface.h

    f2dEngine.h
    f2dFileSys.h
    f2dWindow.h
    f2dInputSys.h
    f2dSoundSys.h
    f2dRenderDevice.h
    f2dRenderer.h
    f2dVideoSys.h

    Common/fancy2d.manifest
    
    Common/f2dStandardCommon.hpp
    Common/f2dStandardCommon.cpp
    Common/f2dWindowsCommon.h
    Common/f2dWindowsCommon.cpp

    Engine/f2dMsgPumpImpl.h
    Engine/f2dFPSControllerImpl.h
    Engine/f2dEngineImpl.h
    Engine/f2dEngineImpl.cpp
    Engine/f2dFileSysImpl.h
    Engine/f2dFileSysImpl.cpp
    Engine/f2dWindowCommonMessage.h
    Engine/f2dWindowImpl.h
    Engine/f2dWindowImpl.cpp
    Engine/f2dMonitorHelper.h
    Engine/f2dMonitorHelper.cpp

    Input/f2dInputSysImpl.h
    Input/f2dInputSysImpl.cpp
    Input/f2dInputKeyboardImpl.h
    Input/f2dInputKeyboardImpl.cpp
    Input/f2dInputMouseImpl.h
    Input/f2dInputMouseImpl.cpp
    Input/f2dInputJoystickImpl.h
    Input/f2dInputJoystickImpl.cpp

    #RenderDev/f2dEffectImpl.h
    #RenderDev/f2dEffectImpl.cpp
    #RenderDev/f2dMeshDataImpl.h
    #RenderDev/f2dMeshDataImpl.cpp
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

    #Video/f2dVideoSysImpl.h
    #Video/f2dVideoSysImpl.cpp
    #Video/f2dVideoImpl.h
    #Video/f2dVideoImpl.cpp
    #Video/f2dVideoStream.h
    #Video/f2dVideoStream.cpp
    #Video/f2dVideoRenderer.h
    #Video/f2dVideoRenderer.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX fancy2d FILES ${FANCY2D_SOURCE})

# 静态库

add_library(fancy2dlib STATIC)

luastg_target_common_options(fancy2dlib)

target_compile_definitions(fancy2dlib PRIVATE
    ____EMPTY_0000_0000_0000_0000____
)

target_include_directories(fancy2dlib PUBLIC
    .
)
target_include_directories(fancy2dlib PRIVATE
    ../
)

target_sources(fancy2dlib PRIVATE
    ${FANCY2D_SOURCE}
    ../Common/DPIHelper.hpp
    ../Common/DPIHelper.cpp
)

target_link_libraries(fancy2dlib PUBLIC
    spdlog::spdlog
    winmm.lib
    imm32.lib
    dxguid.lib
    dinput8.lib
    DirectX::XAudio2Redist
    DirectXTex0
    dxgi.lib
    d3d11.lib
    zlib
    freetype
    vorbisfile
    fancylib
)

# 动态库

if(FALSE)
    add_library(fancy2d SHARED)

    luastg_target_common_options(fancy2d)

    target_compile_definitions(fancy2d PRIVATE
        ____EMPTY_0000_0000_0000_0000____
        F2DEXPORT
    )
    target_compile_definitions(fancy2d PUBLIC
        F2DDLL
    )

    target_include_directories(fancy2d PUBLIC
        .
    )
    target_include_directories(fancy2d PRIVATE
        ../
    )

    target_sources(fancy2d PRIVATE
        ${FANCY2D_SOURCE}
    )

    target_link_libraries(fancy2d PRIVATE
        spdlog::spdlog
        winmm.lib
        imm32.lib
        dxguid.lib
        dinput8.lib
        DirectX::XAudio2Redist
        DirectXTex0
        dxgi.lib
        d3d11.lib
        zlib
        freetype
        vorbisfile
        fancylib
        xmath
    )
endif()
