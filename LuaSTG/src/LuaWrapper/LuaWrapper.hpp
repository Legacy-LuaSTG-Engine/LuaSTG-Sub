/// @file LuaWrapper.h
/// @brief lua包装层 用于导出C++函数和类
#pragma once
#include "AppFrame.h"
#include "LuaWrapper/LuaWrapperMisc.hpp"

#define LUASTG_LUA_LIBNAME "lstg"

#define LUASTG_LUA_TYPENAME_COLOR "lstg.Color"
#define LUASTG_LUA_TYPENAME_STOPWATCH "lstg.StopWatch"
#define LUASTG_LUA_TYPENAME_RANDGEN "lstg.Rand"
#define LUASTG_LUA_TYPENAME_BENTLASER "lstg.BentLaser"

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
		static void Register(lua_State* L)LNOEXCEPT;
	};
	
	//压缩包
	class ArchiveWrapper {
	private:
		struct Wrapper;
	public:
		static void Register(lua_State* L)LNOEXCEPT;
		static void CreateAndPush(lua_State* L, unsigned int uid)LNOEXCEPT;
	};

	//文件资源管理
	class FileManagerWrapper {
	public:
		static void Register(lua_State* L)LNOEXCEPT;
	};

	namespace LuaWrapper
	{
		class WindowWrapper
		{
		public:
			static void Register(lua_State* L)LNOEXCEPT;
		};
		
		class RenderWrapper
		{
		public:
			static void Register(lua_State* L)LNOEXCEPT;
		};
		
		class ColorWrapper
		{
		public:
			struct HSVColor {
				float hue;
				float saturation;
				float value;
			};
			static HSVColor RGB2HSV(const fcyColor& rgb)LNOEXCEPT;
			static fcyColor HSV2RGB(const HSVColor& hsv)LNOEXCEPT;
			static void Register(lua_State* L)LNOEXCEPT;
			static void CreateAndPush(lua_State* L, const fcyColor& color);
		};

		class StopWatchWrapper
		{
		public:
			static void Register(lua_State* L)LNOEXCEPT;
			static void CreateAndPush(lua_State* L);
		};

		class RandomizerWrapper
		{
		public:
			static void Register(lua_State* L)LNOEXCEPT;
			static void CreateAndPush(lua_State* L);
		};

		class BentLaserWrapper
		{
		private:
			struct Wrapper;
		public:
			static void Register(lua_State* L)LNOEXCEPT;
			static void CreateAndPush(lua_State* L);
		};
		
		class DInputWrapper
		{
		public:
			static void Register(lua_State* L)LNOEXCEPT;
		};
		
		namespace IO {
			class StreamWrapper
			{
			public:
				struct Wrapper {
					fcyStream* handle;
				};
			public:
				static void Register(lua_State* L)LNOEXCEPT;
				static void CreateAndPush(lua_State* L, fcyStream* handle);
			};

			class BinaryReaderWrapper
			{
			public:
				struct Wrapper {
					fcyBinaryReader* handle;
				};
			public:
				static void Register(lua_State* L)LNOEXCEPT;
				static void CreateAndPush(lua_State* L, fcyStream* handle);
			};

			class BinaryWriterWrapper
			{
			public:
				struct Wrapper {
					fcyBinaryWriter* handle;
				};
			public:
				static void Register(lua_State* L)LNOEXCEPT;
				static void CreateAndPush(lua_State* L, fcyStream* handle);
			};

			void Register(lua_State* L)LNOEXCEPT;
		}

		void Register(lua_State* L)LNOEXCEPT;
	}

	void RegistBuiltInClassWrapper(lua_State* L)LNOEXCEPT;
}
