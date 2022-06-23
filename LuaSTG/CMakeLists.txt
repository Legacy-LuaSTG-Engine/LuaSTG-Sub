# LuaSTG configurer

add_subdirectory(configurer)

# LuaSTG Engine

add_executable(LuaSTG WIN32)

luastg_target_common_options(LuaSTG)
luastg_target_more_warning(LuaSTG)
target_precompile_headers(LuaSTG PRIVATE
    LuaSTG/SharedHeaders.h
)
target_include_directories(LuaSTG PRIVATE
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

    LuaSTG/GameResource/ResourceDebug.cpp
    LuaSTG/GameResource/ResourceAnimation.cpp
    LuaSTG/GameResource/ResourceAnimation.hpp
    LuaSTG/GameResource/ResourceAudio.cpp
    LuaSTG/GameResource/ResourceAudio.hpp
    LuaSTG/GameResource/ResourceBase.hpp
    LuaSTG/GameResource/ResourceFont.cpp
    LuaSTG/GameResource/ResourceFont.hpp
    LuaSTG/GameResource/ResourceCommon.cpp
    LuaSTG/GameResource/ResourceFX.hpp
    LuaSTG/GameResource/ResourceMgr.cpp
    LuaSTG/GameResource/ResourceMgr.h
    LuaSTG/GameResource/ResourceModel.hpp
    LuaSTG/GameResource/ResourceParticle.cpp
    LuaSTG/GameResource/ResourceParticle.hpp
    LuaSTG/GameResource/ResourcePassword.hpp
    LuaSTG/GameResource/ResourcePool.cpp
    LuaSTG/GameResource/ResourceSprite.hpp
    LuaSTG/GameResource/ResourceTexture.hpp
    LuaSTG/GameResource/ResourceTexture.cpp

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
    LuaSTG/LuaBinding/LW_IO_BinaryReader.cpp
    LuaSTG/LuaBinding/LW_IO_BinaryWriter.cpp
    LuaSTG/LuaBinding/LW_IO_Steam.cpp
    LuaSTG/LuaBinding/LW_IO.cpp
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

    LuaSTG/SteamAPI/SteamAPI.cpp
    LuaSTG/SteamAPI/SteamAPI.hpp

    LuaSTG/Utility/CircularQueue.hpp
    LuaSTG/Utility/Dictionary.hpp
    LuaSTG/Utility/fixed_object_pool.hpp
    LuaSTG/Utility/Utility.h
    LuaSTG/Utility/ScopeObject.cpp
    
    LuaSTG/AppFrame.h
    LuaSTG/AppFrame.cpp
    LuaSTG/AppFrameFontRenderer.cpp
    LuaSTG/AppFrameInput.cpp
    LuaSTG/AppFrameLua.cpp
    LuaSTG/AppFrameRender.cpp
    LuaSTG/AppFrameRenderEx.cpp
    LuaSTG/AppFrameAdapterPolicy.cpp
    LuaSTG/LConfig.h
    LuaSTG/LMathConstant.hpp
    LuaSTG/Main.cpp

    LuaSTG/LuaSTG.manifest
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${LUASTG_ENGINE_SOURCES})
target_sources(LuaSTG PRIVATE
    ${LUASTG_ENGINE_SOURCES}
)

if(NOT DEFINED LUASTG_RESDIR)
    option(LUASTG_RESDIR "LuaSTG custom build configuration" LuaSTG/Custom)
endif()
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
    spdlog::spdlog
    utility
    platform
    fancylib
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
)

add_custom_command(TARGET LuaSTG POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # DirectX family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        ${CMAKE_SOURCE_DIR}/bin
    
    # lua family
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:luajit>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            ${CMAKE_SOURCE_DIR}/bin
    
    # luastg engine
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:LuaSTG>"            ${CMAKE_SOURCE_DIR}/bin
)
