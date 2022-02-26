
set(LUASTG_ENGINE_SOURCES
    pch/pch.h
    pch/pch.cpp
    
    cpp/CircularQueue.hpp
    cpp/Dictionary.hpp
    cpp/fixed_object_pool.hpp
    
    src/LuaWrapper/lua_luastg_hash.cpp
    src/LuaWrapper/lua_luastg_hash.hpp
    src/LuaWrapper/LuaAppFrame.hpp
    src/LuaWrapper/LuaCustomLoader.cpp
    src/LuaWrapper/LuaCustomLoader.hpp
    src/LuaWrapper/LuaInternalSource.cpp
    src/LuaWrapper/LuaInternalSource.hpp
    src/LuaWrapper/LuaTableToOption.cpp
    src/LuaWrapper/LuaTableToOption.hpp
    src/LuaWrapper/LuaWrapper.cpp
    src/LuaWrapper/LuaWrapper.hpp
    src/LuaWrapper/LuaWrapperMisc.hpp
    src/LuaWrapper/LW_Archive.cpp
    src/LuaWrapper/LW_Audio.cpp
    src/LuaWrapper/LW_BentLaser.cpp
    src/LuaWrapper/LW_Color.cpp
    src/LuaWrapper/LW_DInput.cpp
    src/LuaWrapper/LW_FileManager.cpp
    src/LuaWrapper/LW_IO_BinaryReader.cpp
    src/LuaWrapper/LW_IO_BinaryWriter.cpp
    src/LuaWrapper/LW_IO_Steam.cpp
    src/LuaWrapper/LW_IO.cpp
    src/LuaWrapper/LW_LuaSTG.cpp
    src/LuaWrapper/LW_Randomizer.cpp
    src/LuaWrapper/LW_Render.cpp
    src/LuaWrapper/LW_Renderer.cpp
    src/LuaWrapper/LW_StopWatch.cpp
    src/LuaWrapper/LW_Window.cpp
    
    src/Resource/ResourceAnimation.cpp
    src/Resource/ResourceAnimation.hpp
    src/Resource/ResourceAudio.cpp
    src/Resource/ResourceAudio.hpp
    src/Resource/ResourceBase.hpp
    src/Resource/ResourceFont.cpp
    src/Resource/ResourceFont.hpp
    src/Resource/ResourceFX.cpp
    src/Resource/ResourceFX.hpp
    src/Resource/ResourceMgr.cpp
    src/Resource/ResourceMgr.h
    src/Resource/ResourceModel.hpp
    src/Resource/ResourceParticle.cpp
    src/Resource/ResourceParticle.hpp
    src/Resource/ResourcePassword.hpp
    src/Resource/ResourcePool.cpp
    src/Resource/ResourceSprite.hpp
    src/Resource/ResourceTexture.hpp
    
    src/SteamAPI/SteamAPI.cpp
    src/SteamAPI/SteamAPI.hpp
    
    src/AppFrame.cpp
    src/AppFrame.h
    src/AppFrameFontRenderer.cpp
    src/AppFrameInput.cpp
    src/AppFrameLua.cpp
    src/AppFrameRender.cpp
    src/AppFrameRenderEx.cpp
    src/ESC.cpp
    src/ESC.h
    src/f2dKeyCodeConvert.inl
    src/GameObject.cpp
    src/GameObject.hpp
    src/GameObjectBentLaser.cpp
    src/GameObjectBentLaser.hpp
    src/GameObjectClass.cpp
    src/GameObjectClass.hpp
    src/GameObjectPool.cpp
    src/GameObjectPool.h
    src/Global.h
    src/ImGuiExtension.cpp
    src/ImGuiExtension.h
    src/LConfig.h
    src/LLogger.cpp
    src/LLogger.hpp
    src/LMathConstant.hpp
    src/Main.cpp
    src/ScopeObject.cpp
    src/Utility.h
    src/E2DFileManager.hpp
    src/E2DFileManager.cpp
    
    src/LuaSTG.manifest
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${LUASTG_ENGINE_SOURCES})

set(LUASTG_ENGINE_SOURCES_COMMON
    ../Common/DirectInput.h
    ../Common/DirectInput.cpp
    ../Common/SystemDirectory.hpp
    ../Common/SystemDirectory.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/.. FILES ${LUASTG_ENGINE_SOURCES_COMMON})

set(LUASTG_LUA_EXTENSION_SOURCES
    ../xinput/xinput.hpp
    ../xinput/xinput.cpp
    ../xinput/lua_xinput.hpp
    ../xinput/lua_xinput.cpp
    ../security/security.hpp
    ../security/security.cpp
    ../security/lua_security.hpp
    ../security/lua_security.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/.. FILES ${LUASTG_LUA_EXTENSION_SOURCES})
