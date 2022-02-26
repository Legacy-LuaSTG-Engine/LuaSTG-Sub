#include <string>
#include "LuaWrapper/LuaWrapper.hpp"

namespace LuaSTGPlus
{
	namespace LuaWrapper
	{
		namespace IO
		{
			void BinaryWriterWrapper::Register(lua_State* L)LNOEXCEPT {
				struct Function {
#define GETUDATA(index) BinaryWriterWrapper::Wrapper* p = static_cast<BinaryWriterWrapper::Wrapper*>(luaL_checkudata(L, (index), LUASTG_LUA_TYPENAME_IO_BINARY_WRITER));
					static int WriteChar(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						const char* buffer = luaL_checkstring(L, -1);
						bool ok = false;
						try {
							p->handle->Write(buffer[0]);
							ok = true;
						}
						catch (...) { }
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteChars(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						std::string buffer = luaL_checkstring(L, -1);
						fLen len = (fLen)buffer.size();
						bool ok = false;
						try {
							p->handle->Write(buffer.data(), len);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteByte(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						lua_Integer i = luaL_checkinteger(L, -1);
						fByte b = (fByte)i;
						bool ok = false;
						try {
							p->handle->Write(b);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteInt16(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						lua_Integer i = luaL_checkinteger(L, -1);
						fShort b = (fShort)i;
						bool ok = false;
						try {
							p->handle->Write(b);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteUInt16(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						lua_Integer i = luaL_checkinteger(L, -1);
						fuShort b = (fuShort)i;
						bool ok = false;
						try {
							p->handle->Write(b);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteInt32(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						lua_Integer i = luaL_checkinteger(L, -1);
						fInt b = (fInt)i;
						bool ok = false;
						try {
							p->handle->Write(b);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteFloat(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						lua_Number i = luaL_checknumber(L, -1);
						fFloat b = (fFloat)i;
						bool ok = false;
						try {
							p->handle->Write(b);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					static int WriteDouble(lua_State* L)LNOEXCEPT {
						GETUDATA(-2);
						lua_Number i = luaL_checknumber(L, -1);
						fDouble b = (fDouble)i;
						bool ok = false;
						try {
							p->handle->Write(b);
							ok = true;
						}
						catch (...) {}
						::lua_pushboolean(L, ok);
						return 1;
					}
					
					static int Meta_ToString(lua_State* L)LNOEXCEPT
					{
						GETUDATA(-1);
						::lua_pushstring(L, LUASTG_LUA_TYPENAME_IO_BINARY_WRITER);
						return 1;
					}
					static int Meta_GC(lua_State* L)LNOEXCEPT
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
	}
}
