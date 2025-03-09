# LuaSTG configurer

add_subdirectory(Shader)
include(Platform.cmake)
include(Core.cmake)
add_subdirectory(Configurer)

# LuaSTG Engine

add_executable(LuaSTG WIN32)

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
    LuaSTG/GameObject/GameObjectBentLaser.cpp
    LuaSTG/GameObject/GameObjectBentLaser.hpp
    LuaSTG/GameObject/GameObjectClass.cpp
    LuaSTG/GameObject/GameObjectClass.hpp
    LuaSTG/GameObject/GameObjectPool.cpp
    LuaSTG/GameObject/GameObjectPool.h

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

    LuaSTG/LuaBinding/lua_dwrite.hpp
    LuaSTG/LuaBinding/lua_dwrite.cpp
    LuaSTG/LuaBinding/lua_random.hpp
    LuaSTG/LuaBinding/lua_random.cpp
    LuaSTG/LuaBinding/lua_xinput.hpp
    LuaSTG/LuaBinding/lua_xinput.cpp
    LuaSTG/LuaBinding/LuaAppFrame.hpp
    LuaSTG/LuaBinding/LuaCustomLoader.cpp
    LuaSTG/LuaBinding/LuaCustomLoader.hpp
    LuaSTG/LuaBinding/LuaInternalSource.cpp
    LuaSTG/LuaBinding/LuaInternalSource.hpp
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
    LuaSTG/LuaBinding/LW_Randomizer.cpp
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
    LuaSTG/LuaBinding/Display.hpp
    LuaSTG/LuaBinding/Display.cpp
    LuaSTG/LuaBinding/Window.hpp
    LuaSTG/LuaBinding/Window.cpp
    LuaSTG/LuaBinding/SwapChain.hpp
    LuaSTG/LuaBinding/SwapChain.cpp
    LuaSTG/LuaBinding/Texture2D.hpp
    LuaSTG/LuaBinding/Texture2D.cpp
    LuaSTG/LuaBinding/Mesh.hpp
    LuaSTG/LuaBinding/Mesh.cpp
    LuaSTG/LuaBinding/MeshRenderer.hpp
    LuaSTG/LuaBinding/MeshRenderer.cpp
    LuaSTG/LuaBinding/HttpClient.hpp
    LuaSTG/LuaBinding/HttpClient.cpp
    LuaSTG/LuaBinding/Vector2.hpp
    LuaSTG/LuaBinding/Vector2.cpp
    LuaSTG/LuaBinding/Vector3.hpp
    LuaSTG/LuaBinding/Vector3.cpp
    LuaSTG/LuaBinding/Vector4.hpp
    LuaSTG/LuaBinding/Vector4.cpp
    LuaSTG/LuaBinding/generated/BlendModeX.cpp
    LuaSTG/LuaBinding/generated/BlendModeX.hpp
    LuaSTG/LuaBinding/generated/ColorMember.cpp
    LuaSTG/LuaBinding/generated/ColorMember.hpp
    LuaSTG/LuaBinding/generated/GameObjectMember.cpp
    LuaSTG/LuaBinding/generated/GameObjectMember.hpp
    
    LuaSTG/CLRBinding/CLRHost.cpp
    LuaSTG/CLRBinding/CLRHost.hpp
    LuaSTG/CLRBinding/CLRBinding.cpp
    LuaSTG/CLRBinding/CLRBinding.hpp

    LuaSTG/SteamAPI/SteamAPI.cpp
    LuaSTG/SteamAPI/SteamAPI.hpp

    LuaSTG/Utility/CircularQueue.hpp
    LuaSTG/Utility/fixed_object_pool.hpp
    LuaSTG/Utility/Utility.h
    LuaSTG/Utility/ScopeObject.cpp
    LuaSTG/Utility/xorshift.hpp
    
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
    LuaSTG/WindowsMain.cpp

    LuaSTG/LuaSTG.manifest
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${LUASTG_ENGINE_SOURCES})
target_sources(LuaSTG PRIVATE
    ${LUASTG_ENGINE_SOURCES}
)

set(LUASTG_RESDIR ${CMAKE_CURRENT_LIST_DIR}/LuaSTG/Custom CACHE PATH "LuaSTG custom build configuration")
if(EXISTS ${LUASTG_RESDIR}/Custom.cmake)
    message("LuaSTG will using custom build configuration: " ${LUASTG_RESDIR}/Custom.cmake)
    include(${LUASTG_RESDIR}/Custom.cmake)
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
    message("LuaSTG will using custom build configuration: " ${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
else()
    message(FATAL_ERROR "LuaSTG can not find custom build configuration")
endif()

target_link_libraries(LuaSTG PRIVATE
    spdlog
    utility
    utf8
    PlatformAPI
    DirectX::ToolKitMini
    DirectX::TextureMini
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
    LuaSTG.Sub.Configuration
)
if (LUASTG_LINK_YY_THUNKS)
    target_link_libraries(LuaSTG PRIVATE YY_Thunks)
    message(STATUS "[LuaSTG] Link: YY_Thunks")
endif ()
if (LUASTG_LINK_LUASOCKET)
    target_compile_definitions(LuaSTG PRIVATE LUASTG_LINK_LUASOCKET)
    target_link_libraries(LuaSTG PRIVATE luasocket)
    message(STATUS "[LuaSTG] Link: luasocket")
endif ()

set (CLR_HOST_PATH "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/8.0.13/runtimes/win-x64/native")
include_directories (${CLR_HOST_PATH})
find_library (HOSTFXR NAMES nethost PATHS ${CLR_HOST_PATH})
target_link_libraries (LuaSTG PRIVATE ${HOSTFXR})

add_custom_command(TARGET LuaSTG POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory                                                ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:LuaSTG>                       ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Microsoft.XAudio2.Redist>     ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:Microsoft.XAudio2.Redist>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Microsoft.D3DCompiler.Redist> ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:Microsoft.D3DCompiler.Redist>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CLR_HOST_PATH}/nethost.dll                ${CMAKE_BINARY_DIR}/bin/nethost.dll
    VERBATIM
)
