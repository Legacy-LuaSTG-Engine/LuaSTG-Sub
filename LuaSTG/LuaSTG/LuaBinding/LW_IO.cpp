#include "LuaBinding/LuaWrapper.hpp"
#include "Core/FileManager.hpp"

namespace LuaSTGPlus::LuaWrapper::IO
{
	void Register(lua_State* L)noexcept
	{
		struct Wrapper
		{
		#define GETSTREAM_UDATA(i) StreamWrapper::Wrapper* p = static_cast<StreamWrapper::Wrapper*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_IO_STREAM));
			static int FileStream(lua_State* L)noexcept
			{
				fcyStream* stream = nullptr;
				try {
					//std::string path = luaL_checkstring(L, 1);
					//bool write = (bool)lua_toboolean(L, 2);
					//std::wstring wpath = utility::encoding::to_utf8(path);
					//fcyFileStream* _stream = new fcyFileStream(wpath.c_str(), write);
					//stream = _stream;

					std::vector<uint8_t> src;
					if (!GFileManager().loadEx(luaL_checkstring(L, 1), src))
					{
						lua_pushnil(L);
						return 1;
					}
					stream = new fcyMemStream(std::move(src));
				}
				catch (...) {
					lua_pushnil(L);
					return 1;
				}
				StreamWrapper::CreateAndPush(L, stream);
				// ref = 2
				stream->Release(); // 此时归属交给了udata（已AddRef），这边可以释放掉了
				// ref = 1
				return 1;
			}
			static int MemoryStream(lua_State* L)noexcept
			{
				fcyStream* stream = nullptr;
				try {
					lua_Integer len = luaL_checkinteger(L, 1);
					bool write = 0 != luaL_checkinteger(L, 2);
					bool resize = 0 != luaL_checkinteger(L, 3);
					uint64_t _len = (uint64_t)len;
					fcyMemStream* _stream = new fcyMemStream(nullptr, _len, write, resize);
					stream = _stream;
				}
				catch (...) {
					lua_pushnil(L);
					return 1;
				}
				StreamWrapper::CreateAndPush(L, stream);
				return 1;
			}
			static int BinaryReader(lua_State* L)noexcept
			{
				GETSTREAM_UDATA(1);
				BinaryReaderWrapper::CreateAndPush(L, p->handle);
				return 1;
			}
			static int BinaryWriter(lua_State* L)noexcept
			{
				GETSTREAM_UDATA(1);
				BinaryWriterWrapper::CreateAndPush(L, p->handle);
				return 1;
			}
		#undef GETSTREAM_UDATA
		};

		luaL_Reg Function[] =
		{
			{ "FileStream", &Wrapper::FileStream },
			{ "MemoryStream", &Wrapper::MemoryStream },
			{ "BinaryReader", &Wrapper::BinaryReader },
			{ "BinaryWriter", &Wrapper::BinaryWriter },
			{ NULL, NULL }
		};

		lua_pushstring(L, LUASTG_LUA_LIBNAME_IO);	// ... t s
		lua_newtable(L);							// ... t s t
		StreamWrapper::Register(L);
		BinaryReaderWrapper::Register(L);
		BinaryWriterWrapper::Register(L);
		luaL_register(L, NULL, Function);
		lua_settable(L, -3);						// ... t
	}
}
