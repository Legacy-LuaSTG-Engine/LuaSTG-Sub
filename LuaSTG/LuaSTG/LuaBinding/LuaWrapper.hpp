/// @file LuaWrapper.h
/// @brief lua包装层 用于导出C++函数和类
#pragma once
#include "AppFrame.h"
#include "LuaBinding/LuaWrapperMisc.hpp"

#define LUASTG_LUA_LIBNAME "lstg"

#define LUASTG_LUA_TYPENAME_STOPWATCH "lstg.StopWatch"
#define LUASTG_LUA_TYPENAME_RANDGEN "lstg.Rand"
#define LUASTG_LUA_TYPENAME_BENTLASER "lstg.CurveLaser"

#define LUASTG_LUA_LIBNAME_IO "IO"
#define LUASTG_LUA_TYPENAME_IO_STREAM "lstg.IO.Stream"
#define LUASTG_LUA_TYPENAME_IO_BINARY_READER "lstg.IO.BinaryReader"
#define LUASTG_LUA_TYPENAME_IO_BINARY_WRITER "lstg.IO.BinaryWriter"

#define LUASTG_LUA_TYPENAME_RESOURCE "lstgResource"
#define LUASTG_LUA_TYPENAME_ARCHIVE "lstgArchive"

namespace LuaSTGPlus
{
	//内建函数包装
	class BuiltInFunctionWrapper
	{
	public:
		/// @brief 向lua注册包装类
		static void Register(lua_State* L) noexcept;
	};
	
	//压缩包
	class ArchiveWrapper {
	private:
		struct Wrapper;
	public:
		static void Register(lua_State* L) noexcept;
		static void CreateAndPush(lua_State* L, uint64_t uuid) noexcept;
	};

	//文件资源管理
	class FileManagerWrapper {
	public:
		static void Register(lua_State* L) noexcept;
	};

	namespace LuaWrapper
	{
		class InputWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		
		class RenderWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		class RendererWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		class GameObjectManagerWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		class ResourceMgrWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		class AudioWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		class PlatformWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		
		class ColorWrapper
		{
		public:
			static std::string_view const ClassID;
			static Core::Color4B* Cast(lua_State* L, int idx);
			static void Register(lua_State* L) noexcept;
			static void CreateAndPush(lua_State* L, Core::Color4B const& color);
		};

		class StopWatchWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
			static void CreateAndPush(lua_State* L);
		};

		class RandomizerWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
			static void CreateAndPush(lua_State* L);
		};

		class BentLaserWrapper
		{
		private:
			struct Wrapper;
		public:
			static void Register(lua_State* L) noexcept;
			static void CreateAndPush(lua_State* L);
		};
		
		class DInputWrapper
		{
		public:
			static void Register(lua_State* L) noexcept;
		};
		
		class ParticleSystemWrapper
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

		class MeshBinding
		{
		public:
			static std::string_view const ClassID;
			static Mesh* Cast(lua_State* L, int idx);
			static Mesh* Create(lua_State* L);
			static void Register(lua_State* L);
		};

		void Register(lua_State* L) noexcept;
	}

	void RegistBuiltInClassWrapper(lua_State* L) noexcept;
}
