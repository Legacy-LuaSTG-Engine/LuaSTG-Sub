# LuaSTG configurer

add_subdirectory(Shader)
include(Platform.cmake)
include(Core.cmake)
add_subdirectory(Configurer)

# LuaSTG Engine

add_executable(LuaSTG WIN32)
set_target_properties(LuaSTG PROPERTIES OUTPUT_NAME "LuaSTGSub")

luastg_target_common_options(LuaSTG)
luastg_target_more_warning(LuaSTG)
target_precompile_headers(LuaSTG PRIVATE
    LuaSTG/SharedHeaders.h
)
target_include_directories(LuaSTG PRIVATE
    LuaSTG/Utility/fancylib
    LuaSTG
    LuaSTG/LuaBinding
)
target_include_directories(Setting PRIVATE
    LuaSTG
)
target_compile_definitions(LuaSTG PRIVATE
    LDEVVERSION
)

set(LUASTG_ENGINE_SOURCES
    LuaSTG/SharedHeaders.h
    LuaSTG/SharedHeaders.cpp
    
    LuaSTG/Debugger/ImGuiExtension.cpp
    LuaSTG/Debugger/ImGuiExtension.h
    LuaSTG/Debugger/Logger.cpp
    LuaSTG/Debugger/Logger.hpp

    LuaSTG/GameObject/GameObject.cpp
    LuaSTG/GameObject/GameObject.hpp
    LuaSTG/GameObject/GameObjectIntersectDetect.cpp
    LuaSTG/GameObject/GameObjectBentLaser.cpp
    LuaSTG/GameObject/GameObjectBentLaser.hpp
    LuaSTG/GameObject/GameObjectPool.cpp
    LuaSTG/GameObject/GameObjectPool.h

    LuaSTG/GameResource/LegacyBlendStateHelper.hpp
    LuaSTG/GameResource/ResourceBase.hpp
    LuaSTG/GameResource/ResourceTexture.hpp
    LuaSTG/GameResource/ResourceSprite.hpp
    LuaSTG/GameResource/ResourceAnimation.hpp
    LuaSTG/GameResource/ResourceMusic.hpp
    LuaSTG/GameResource/ResourceSoundEffect.hpp
    LuaSTG/GameResource/ResourceParticle.hpp
    LuaSTG/GameResource/ResourceFont.hpp
    LuaSTG/GameResource/ResourcePostEffectShader.hpp
    LuaSTG/GameResource/ResourceModel.hpp
    LuaSTG/GameResource/ResourceModel.cpp

    LuaSTG/GameResource/ResourceDebug.cpp
    LuaSTG/GameResource/ResourceManager.cpp
    LuaSTG/GameResource/ResourceManager.h
    LuaSTG/GameResource/ResourcePassword.hpp
    LuaSTG/GameResource/ResourcePool.cpp

    LuaSTG/GameResource/Implement/ResourceBaseImpl.hpp
    LuaSTG/GameResource/Implement/ResourceBaseImpl.cpp
    LuaSTG/GameResource/Implement/ResourceTextureImpl.hpp
    LuaSTG/GameResource/Implement/ResourceTextureImpl.cpp
    LuaSTG/GameResource/Implement/ResourceSpriteImpl.hpp
    LuaSTG/GameResource/Implement/ResourceSpriteImpl.cpp
    LuaSTG/GameResource/Implement/ResourceAnimationImpl.hpp
    LuaSTG/GameResource/Implement/ResourceAnimationImpl.cpp
    LuaSTG/GameResource/Implement/ResourceMusicImpl.hpp
    LuaSTG/GameResource/Implement/ResourceMusicImpl.cpp
    LuaSTG/GameResource/Implement/ResourceSoundEffectImpl.hpp
    LuaSTG/GameResource/Implement/ResourceSoundEffectImpl.cpp
    LuaSTG/GameResource/Implement/ResourceParticleImpl.hpp
    LuaSTG/GameResource/Implement/ResourceParticleImpl.cpp
    LuaSTG/GameResource/Implement/ResourceFontImpl.hpp
    LuaSTG/GameResource/Implement/ResourceFontImpl.cpp
    LuaSTG/GameResource/Implement/ResourcePostEffectShaderImpl.hpp
    LuaSTG/GameResource/Implement/ResourcePostEffectShaderImpl.cpp
    LuaSTG/GameResource/Implement/ResourceModelImpl.hpp
    LuaSTG/GameResource/Implement/ResourceModelImpl.cpp

    LuaSTG/LuaBinding/LuaAppFrame.hpp
    LuaSTG/LuaBinding/LuaCustomLoader.cpp
    LuaSTG/LuaBinding/LuaCustomLoader.hpp
    LuaSTG/LuaBinding/LuaWrapper.cpp
    LuaSTG/LuaBinding/LuaWrapper.hpp
    LuaSTG/LuaBinding/LuaWrapperMisc.hpp
    LuaSTG/LuaBinding/LW_Archive.cpp
    LuaSTG/LuaBinding/LW_Audio.cpp
    LuaSTG/LuaBinding/LW_BentLaser.cpp
    LuaSTG/LuaBinding/LW_Color.cpp
    LuaSTG/LuaBinding/LW_DInput.cpp
    LuaSTG/LuaBinding/LW_FileManager.cpp
    LuaSTG/LuaBinding/LW_Input.cpp
    LuaSTG/LuaBinding/LW_LuaSTG.cpp
    LuaSTG/LuaBinding/LW_ParticleSystem.cpp
    LuaSTG/LuaBinding/LW_Render.cpp
    LuaSTG/LuaBinding/LW_Renderer.cpp
    LuaSTG/LuaBinding/LW_StopWatch.cpp
    LuaSTG/LuaBinding/LW_ResourceMgr.cpp
    LuaSTG/LuaBinding/LW_Platform.cpp
    LuaSTG/LuaBinding/LW_GameObjectManager.cpp
    LuaSTG/LuaBinding/PostEffectShader.hpp
    LuaSTG/LuaBinding/PostEffectShader.cpp
    LuaSTG/LuaBinding/Resource.hpp
    LuaSTG/LuaBinding/Resource.cpp

    LuaSTG/LuaBinding/external/lua_dwrite.hpp
    LuaSTG/LuaBinding/external/lua_dwrite.cpp
    LuaSTG/LuaBinding/external/lua_random.hpp
    LuaSTG/LuaBinding/external/lua_random.cpp
    LuaSTG/LuaBinding/external/lua_xinput.hpp
    LuaSTG/LuaBinding/external/lua_xinput.cpp
    LuaSTG/LuaBinding/external/HttpClient.hpp
    LuaSTG/LuaBinding/external/HttpClient.cpp

    LuaSTG/LuaBinding/modern/Display.hpp
    LuaSTG/LuaBinding/modern/Display.cpp
    LuaSTG/LuaBinding/modern/Window.hpp
    LuaSTG/LuaBinding/modern/Window.cpp
    LuaSTG/LuaBinding/modern/SwapChain.hpp
    LuaSTG/LuaBinding/modern/SwapChain.cpp
    LuaSTG/LuaBinding/modern/Texture2D.hpp
    LuaSTG/LuaBinding/modern/Texture2D.cpp
    LuaSTG/LuaBinding/modern/RenderTarget.hpp
    LuaSTG/LuaBinding/modern/RenderTarget.cpp
    LuaSTG/LuaBinding/modern/DepthStencilBuffer.hpp
    LuaSTG/LuaBinding/modern/DepthStencilBuffer.cpp
    LuaSTG/LuaBinding/modern/Mesh.hpp
    LuaSTG/LuaBinding/modern/Mesh.cpp
    LuaSTG/LuaBinding/modern/MeshRenderer.hpp
    LuaSTG/LuaBinding/modern/MeshRenderer.cpp
    LuaSTG/LuaBinding/modern/Vector2.hpp
    LuaSTG/LuaBinding/modern/Vector2.cpp
    LuaSTG/LuaBinding/modern/Vector3.hpp
    LuaSTG/LuaBinding/modern/Vector3.cpp
    LuaSTG/LuaBinding/modern/Vector4.hpp
    LuaSTG/LuaBinding/modern/Vector4.cpp
    LuaSTG/LuaBinding/modern/Sprite.hpp
    LuaSTG/LuaBinding/modern/Sprite.cpp
    LuaSTG/LuaBinding/modern/SpriteRenderer.hpp
    LuaSTG/LuaBinding/modern/SpriteRenderer.cpp
    LuaSTG/LuaBinding/modern/Clipboard.hpp
    LuaSTG/LuaBinding/modern/Clipboard.cpp
    LuaSTG/LuaBinding/modern/FileSystemWatcher.hpp
    LuaSTG/LuaBinding/modern/FileSystemWatcher.cpp
    LuaSTG/LuaBinding/modern/GameObject.hpp
    LuaSTG/LuaBinding/modern/GameObject.cpp
    LuaSTG/LuaBinding/modern/Well512.hpp
    LuaSTG/LuaBinding/modern/Well512.cpp
    LuaSTG/LuaBinding/modern/ShellIntegration.hpp
    LuaSTG/LuaBinding/modern/ShellIntegration.cpp

    LuaSTG/LuaBinding/generated/BlendModeX.cpp
    LuaSTG/LuaBinding/generated/BlendModeX.hpp
    LuaSTG/LuaBinding/generated/ColorMember.cpp
    LuaSTG/LuaBinding/generated/ColorMember.hpp
    LuaSTG/LuaBinding/generated/GameObjectMember.cpp
    LuaSTG/LuaBinding/generated/GameObjectMember.hpp

    LuaSTG/SteamAPI/SteamAPI.cpp
    LuaSTG/SteamAPI/SteamAPI.hpp

    LuaSTG/Utility/Utility.h
    LuaSTG/Utility/ScopeObject.cpp
    LuaSTG/Utility/xorshift.hpp
    LuaSTG/Utility/well512.hpp
    LuaSTG/Utility/well512.cpp
    
    LuaSTG/AppFrame.h
    LuaSTG/AppFrame.cpp
    LuaSTG/AppFrameFontRenderer.cpp
    LuaSTG/AppFrameDisplayMode.cpp
    LuaSTG/AppFrameInput.cpp
    LuaSTG/AppFrameLua.cpp
    LuaSTG/AppFrameRender.cpp
    LuaSTG/AppFrameRenderEx.cpp
    LuaSTG/LConfig.h
    LuaSTG/LMathConstant.hpp
    LuaSTG/RuntimeCheck.hpp
    LuaSTG/RuntimeCheck.cpp
    LuaSTG/Main.h
    LuaSTG/Main.cpp
    LuaSTG/ApplicationRestart.hpp
    LuaSTG/ApplicationRestart.cpp
    LuaSTG/WindowsMain.cpp

    LuaSTG/LuaSTG.manifest
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${LUASTG_ENGINE_SOURCES})
target_sources(LuaSTG PRIVATE
    ${LUASTG_ENGINE_SOURCES}
)

target_link_libraries(LuaSTG PRIVATE
    spdlog
    utility
    utf8
    win32
    PlatformAPI
    DirectX::ToolKitMini
    Microsoft::DirectXTexMini
    Core
    xmath
    lua51_static
    lua_plus
    imgui
    implot
    lua_cjson
    lua_filesystem
    #lua_xlsx_csv
    lua_imgui
    imgui_impl_win32ex
    imgui_impl_dx11
    lua_steam_api
    TracyAPI
    pcg_cpp
    pugixml
    xxhash
    Core.Collection
    Core.Configuration
    Core.Clipboard
    Core.ShellIntegration
    Core.Audio
    LuaSTG.InternalLuaScriptsFileSystem
)

include(${CMAKE_CURRENT_LIST_DIR}/import_luastg_options.cmake)

add_custom_command(TARGET LuaSTG POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Microsoft.XAudio2.Redist>     $<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:Microsoft.XAudio2.Redist>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Microsoft.D3DCompiler.Redist> $<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:Microsoft.D3DCompiler.Redist>
    COMMAND ${CMAKE_COMMAND} -E make_directory                                                ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:LuaSTG>                       ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Microsoft.XAudio2.Redist>     ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:Microsoft.XAudio2.Redist>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Microsoft.D3DCompiler.Redist> ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:Microsoft.D3DCompiler.Redist>
    VERBATIM
)
