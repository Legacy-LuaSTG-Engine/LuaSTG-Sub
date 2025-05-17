/// @file LuaWrapper.h
/// @brief lua包装层 用于导出C++函数和类
#pragma once
#include "AppFrame.h"
#include "core/FileSystem.hpp"
#include "LuaBinding/LuaWrapperMisc.hpp"

#define LUASTG_LUA_LIBNAME "lstg"

#define LUASTG_LUA_TYPENAME_STOPWATCH "lstg.StopWatch"
#define LUASTG_LUA_TYPENAME_BENTLASER "lstg.CurveLaser"

#define LUASTG_LUA_LIBNAME_IO "IO"
#define LUASTG_LUA_TYPENAME_IO_STREAM "lstg.IO.Stream"
#define LUASTG_LUA_TYPENAME_IO_BINARY_READER "lstg.IO.BinaryReader"
#define LUASTG_LUA_TYPENAME_IO_BINARY_WRITER "lstg.IO.BinaryWriter"

#define LUASTG_LUA_TYPENAME_RESOURCE "lstgResource"
#define LUASTG_LUA_TYPENAME_ARCHIVE "lstgArchive"

namespace luastg::binding
{
	class BuiltInFunction
	{
	public:
		/// @brief 向lua注册包装类
		static void Register(lua_State* L) noexcept;
	};
	
	class Archive {
	private:
		struct Wrapper;
	public:
		static void Register(lua_State* L) noexcept;
		static void CreateAndPush(lua_State* L, core::IFileSystemArchive* archive) noexcept;
	};
	
	class FileManager {
	public:
		static void Register(lua_State* L) noexcept;
	};

	class Input
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class Render
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class Renderer
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class GameObjectManager
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class ResourceManager
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class Audio
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class Platform
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class Color
	{
	public:
		static std::string_view const ClassID;
		static core::Color4B* Cast(lua_State* L, int idx);
		static void Register(lua_State* L) noexcept;
		static void CreateAndPush(lua_State* L, core::Color4B const& color);
	};

	class StopWatch
	{
	public:
		static void Register(lua_State* L) noexcept;
		static void CreateAndPush(lua_State* L);
	};

	class BentLaser
	{
	private:
		struct Wrapper;
	public:
		static void Register(lua_State* L) noexcept;
		static void CreateAndPush(lua_State* L);
	};
	
	class DirectInput
	{
	public:
		static void Register(lua_State* L) noexcept;
	};
	
	class ParticleSystem
	{
	public:
		struct UserData
		{
			IResourceParticle* res;
			IParticlePool* ptr;
			void ReleaseAll();
		};
		static std::string_view const ClassID;
		static UserData* Cast(lua_State* L, int idx);
		static UserData* Create(lua_State* L);
		static void Register(lua_State* L);
	};

	void RegistBuiltInClassWrapper(lua_State* L) noexcept;
}
