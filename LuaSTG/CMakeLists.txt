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
    LuaSTG/LuaBinding/lua_utility.hpp
    LuaSTG/LuaBinding/lua_luastg_hash.cpp
    LuaSTG/LuaBinding/lua_luastg_hash.hpp
    LuaSTG/LuaBinding/LuaAppFrame.hpp
    LuaSTG/LuaBinding/LuaCustomLoader.cpp
    LuaSTG/LuaBinding/LuaCustomLoader.hpp
    LuaSTG/LuaBinding/LuaInternalSource.cpp
    LuaSTG/LuaBinding/LuaInternalSource.hpp
    LuaSTG/LuaBinding/LuaTableToOption.cpp
    LuaSTG/LuaBinding/LuaTableToOption.hpp
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
    LuaSTG/LuaBinding/LW_Window.cpp
    LuaSTG/LuaBinding/LW_ResourceMgr.cpp
    LuaSTG/LuaBinding/LW_Platform.cpp
    LuaSTG/LuaBinding/LW_GameObjectManager.cpp
    LuaSTG/LuaBinding/LB_Mesh.cpp
    LuaSTG/LuaBinding/PostEffectShader.hpp
    LuaSTG/LuaBinding/PostEffectShader.cpp
    LuaSTG/LuaBinding/Resource.hpp
    LuaSTG/LuaBinding/Resource.cpp

    LuaSTG/SteamAPI/SteamAPI.cpp
    LuaSTG/SteamAPI/SteamAPI.hpp

    LuaSTG/Utility/CircularQueue.hpp
    LuaSTG/Utility/Dictionary.hpp
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
    LuaSTG/Main.cpp

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
    PlatformAPI
    DirectXTexMini
    DirectX::ToolKitMini
    Core
    xmath
    lua51static
    imgui
    implot
    lua_cjson
    lua_filesystem
    #lua_xlsx_csv
    lua_imgui
    imgui_impl_win32ex
    imgui_impl_dx11
    lua_steam_api
    tracy
    pcg_cpp
    pugixml
)

set(LUASTG_BUILD_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
add_custom_command(TARGET LuaSTG POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LUASTG_BUILD_OUTPUT_DIR}

    COMMAND ${CMAKE_COMMAND} -E rm -f ${LUASTG_BUILD_OUTPUT_DIR}/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:LuaSTG>         ${LUASTG_BUILD_OUTPUT_DIR}

    #COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:LuaSTG> $<TARGET_FILE_DIR:LuaSTG>
    #COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:LuaSTG> ${LUASTG_BUILD_OUTPUT_DIR}
    COMMAND_EXPAND_LISTS
)
