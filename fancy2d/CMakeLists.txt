cmake_minimum_required(VERSION 3.15)

add_library(fancy2d SHARED)

target_compile_options(fancy2d PUBLIC
    "/utf-8"
    "/MP"
    "/std:c++17"
)

target_compile_definitions(fancy2d PRIVATE
    UNICODE
    _UNICODE
    ZLIB_DLL
    DLL_IMPORT # freetype
)

target_include_directories(fancy2d PRIVATE
    .
    ../fancylib
    ../prebuild/dxsdk/include
    ../DirectShowClass/src
    ../prebuild/zlib/include
    ../prebuild/libogg/include
    ../prebuild/libvorbis/include
    ../prebuild/freetype/include
)

target_sources(fancy2d PRIVATE
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
    
    Engine/f2dMsgPumpImpl.h
    Engine/f2dFPSControllerImpl.h
    Engine/f2dEngineImpl.h
    Engine/f2dEngineImpl.cpp
    Engine/f2dFileSysImpl.h
    Engine/f2dFileSysImpl.cpp
    Engine/f2dWindowImpl.h
    Engine/f2dWindowImpl.cpp
    
    Input/f2dInputSysImpl.h
    Input/f2dInputSysImpl.cpp
    Input/f2dInputSysAPI.h
    Input/f2dInputSysAPI.cpp
    Input/f2dInputKeyboardImpl.h
    Input/f2dInputKeyboardImpl.cpp
    Input/f2dInputMouseImpl.h
    Input/f2dInputMouseImpl.cpp
    Input/f2dInputJoystickImpl.h
    Input/f2dInputJoystickImpl.cpp
    
    RenderDev/f2dRenderDeviceImpl.h
    RenderDev/f2dRenderDeviceImpl.cpp
    RenderDev/f2dRenderDeviceAPI.h
    RenderDev/f2dRenderDeviceAPI.cpp
    RenderDev/f2dGraphicsImpl.h
    RenderDev/f2dGraphics2DImpl.h
    RenderDev/f2dGraphics2DImpl.cpp
    RenderDev/f2dGraphics3DImpl.h
    RenderDev/f2dGraphics3DImpl.cpp
    RenderDev/f2dTextureImpl.h
    RenderDev/f2dTextureImpl.cpp
    RenderDev/f2dEffectImpl.h
    RenderDev/f2dEffectImpl.cpp
    RenderDev/f2dMeshDataImpl.h
    RenderDev/f2dMeshDataImpl.cpp
    
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
    Renderer/f2dFontFileProvider.h
    Renderer/f2dFontFileProvider.cpp
    Renderer/f2dFontTexProvider.h
    Renderer/f2dFontTexProvider.cpp
    
    Sound/f2dSoundSysImpl.h
    Sound/f2dSoundSysImpl.cpp
    Sound/f2dSoundSysAPI.h
    Sound/f2dSoundSysAPI.cpp
    Sound/f2dWaveDecoder.h
    Sound/f2dWaveDecoder.cpp
    Sound/f2dVorbisDecoder.h
    Sound/f2dVorbisDecoder.cpp
    Sound/f2dSoundBufferStatic.h
    Sound/f2dSoundBufferStatic.cpp
    Sound/f2dSoundBufferDynamic.h
    Sound/f2dSoundBufferDynamic.cpp
    Sound/f2dSoundBufferPull.h
    Sound/f2dSoundBufferPull.cpp
    Sound/f2dSoundSpriteImpl.h
    Sound/f2dSoundSpriteImpl.cpp
    
    Video/f2dVideoSysImpl.h
    Video/f2dVideoSysImpl.cpp
    Video/f2dVideoImpl.h
    Video/f2dVideoImpl.cpp
    Video/f2dVideoStream.h
    Video/f2dVideoStream.cpp
    Video/f2dVideoRenderer.h
    Video/f2dVideoRenderer.cpp
)

target_link_directories(fancy2d PRIVATE
    ../prebuild/dxsdk/lib
    ../prebuild/zlib/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libogg/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libvorbis/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/freetype/lib/${EXT_LIBRARY_TYPE}
)

target_link_libraries(fancy2d
    fancylib
    
    strmiids.lib #???
    strmbasd
    winmm.lib
    imm32.lib
    dxguid.lib
    d3dx9.lib
    
    zlib.lib
    ogg.lib
    vorbis.lib
    vorbisfile.lib
    freetype.lib
)
