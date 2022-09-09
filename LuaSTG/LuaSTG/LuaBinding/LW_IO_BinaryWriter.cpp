#include "LuaBinding/LuaWrapper.hpp"

namespace LuaSTGPlus::LuaWrapper::IO
{
	void BinaryWriterWrapper::Register(lua_State* L)noexcept {
		struct Function {
		#define GETUDATA(index) BinaryWriterWrapper::Wrapper* p = static_cast<BinaryWriterWrapper::Wrapper*>(luaL_checkudata(L, (index), LUASTG_LUA_TYPENAME_IO_BINARY_WRITER));
			static int WriteChar(lua_State* L)noexcept {
				GETUDATA(-2);
				const char* buffer = luaL_checkstring(L, -1);
				bool ok = false;
				try {
					p->handle->Write(buffer[0]);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteChars(lua_State* L)noexcept {
				GETUDATA(-2);
				std::string buffer = luaL_checkstring(L, -1);
				uint64_t len = (uint64_t)buffer.size();
				bool ok = false;
				try {
					p->handle->Write(buffer.data(), len);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteByte(lua_State* L)noexcept {
				GETUDATA(-2);
				lua_Integer i = luaL_checkinteger(L, -1);
				uint8_t b = (uint8_t)i;
				bool ok = false;
				try {
					p->handle->Write(b);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteInt16(lua_State* L)noexcept {
				GETUDATA(-2);
				lua_Integer i = luaL_checkinteger(L, -1);
				int16_t b = (int16_t)i;
				bool ok = false;
				try {
					p->handle->Write(b);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteUInt16(lua_State* L)noexcept {
				GETUDATA(-2);
				lua_Integer i = luaL_checkinteger(L, -1);
				uint16_t b = (uint16_t)i;
				bool ok = false;
				try {
					p->handle->Write(b);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteInt32(lua_State* L)noexcept {
				GETUDATA(-2);
				lua_Integer i = luaL_checkinteger(L, -1);
				int32_t b = (int32_t)i;
				bool ok = false;
				try {
					p->handle->Write(b);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteFloat(lua_State* L)noexcept {
				GETUDATA(-2);
				lua_Number i = luaL_checknumber(L, -1);
				float b = (float)i;
				bool ok = false;
				try {
					p->handle->Write(b);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}
			static int WriteDouble(lua_State* L)noexcept {
				GETUDATA(-2);
				lua_Number i = luaL_checknumber(L, -1);
				double b = (double)i;
				bool ok = false;
				try {
					p->handle->Write(b);
					ok = true;
				}
				catch (...) {}
				::lua_pushboolean(L, ok);
				return 1;
			}

			static int Meta_ToString(lua_State* L)noexcept
			{
				GETUDATA(-1);
				std::ignore = p;
				::lua_pushstring(L, LUASTG_LUA_TYPENAME_IO_BINARY_WRITER);
				return 1;
			}
			static int Meta_GC(lua_State* L)noexcept
			{
				GETUDATA(-1);
				if (p->handle != nullptr) {
					delete p->handle;
					p->handle = nullptr;
				}
				return 0;
			}
		#undef GETUDATA
		};

		luaL_Reg tMethods[] =
		{
			{ "WriteChar", &Function::WriteChar },
			{ "WriteChars", &Function::WriteChars },
			{ "WriteByte", &Function::WriteByte },
			{ "WriteInt16", &Function::WriteInt16 },
			{ "WriteUInt16", &Function::WriteUInt16 },
			{ "WriteInt32", &Function::WriteInt32 },
			{ "WriteFloat", &Function::WriteFloat },
			{ "WriteDouble", &Function::WriteDouble },
			{ NULL, NULL }
		};

		luaL_Reg tMetaTable[] =
		{
			{ "__tostring", &Function::Meta_ToString },
			{ "__gc", &Function::Meta_GC },
			{ NULL, NULL }
		};

		RegisterClassIntoTable(L, ".BinaryWriter", tMethods, LUASTG_LUA_TYPENAME_IO_BINARY_WRITER, tMetaTable);
	}

	void BinaryWriterWrapper::CreateAndPush(lua_State* L, fcyStream* handle) {
		BinaryWriterWrapper::Wrapper* p = static_cast<BinaryWriterWrapper::Wrapper*>(lua_newuserdata(L, sizeof(BinaryWriterWrapper::Wrapper))); // udata
		new(p) BinaryWriterWrapper::Wrapper();
		p->handle = new fcyBinaryWriter(handle);
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_IO_BINARY_WRITER); // udata mt
		lua_setmetatable(L, -2); // udata 
	}
}
