#include "LuaBinding/LuaWrapper.hpp"

namespace LuaSTGPlus::LuaWrapper::IO
{
	void StreamWrapper::Register(lua_State* L)noexcept
	{
		struct Function {
		#define GETUDATA(index) StreamWrapper::Wrapper* p = static_cast<StreamWrapper::Wrapper*>(luaL_checkudata(L, (index), LUASTG_LUA_TYPENAME_IO_STREAM));
			static int CanWrite(lua_State* L)noexcept
			{
				GETUDATA(-1);
				lua_pushboolean(L, p->handle->CanWrite());
				return 1;
			}
			static int CanResize(lua_State* L)noexcept
			{
				GETUDATA(-1);
				lua_pushboolean(L, p->handle->CanResize());
				return 1;
			}
			static int GetLength(lua_State* L)noexcept
			{
				GETUDATA(-1);
				lua_Integer len = (lua_Integer)p->handle->GetLength();
				::lua_pushinteger(L, len);
				return 1;
			}
			static int SetLength(lua_State* L)noexcept
			{
				GETUDATA(-2);
				if (p->handle->CanResize()) {
					uint64_t len = (uint64_t)luaL_checkinteger(L, -1);
					lua_pushboolean(L, FCYERR_OK == p->handle->SetLength(len));
					return 1;
				}
				else {
					return luaL_error(L, "Failed to resize stream. Resizing is not supported.");
				}
			}
			static int GetPosition(lua_State* L)noexcept
			{
				GETUDATA(-1);
				lua_Integer pos = (lua_Integer)p->handle->GetPosition();
				::lua_pushinteger(L, pos);
				return 1;
			}
			static int SetPosition(lua_State* L)noexcept
			{
				GETUDATA(-3);
				std::string base = luaL_checkstring(L, -2);
				int64_t offset = (int64_t)luaL_checkinteger(L, -1);
				FCYSEEKORIGIN org = FCYSEEKORIGIN_BEG;
				if (base == "set") {
					org = FCYSEEKORIGIN_BEG;
				}
				else if (base == "cur") {
					org = FCYSEEKORIGIN_CUR;
				}
				else if (base == "end") {
					org = FCYSEEKORIGIN_END;
				}
				else {
					org = FCYSEEKORIGIN_BEG;
				}
				lua_pushboolean(L, FCYERR_OK == p->handle->SetPosition(org, offset));
				return 1;
			}
			static int ReadBytes(lua_State* L)noexcept
			{
				GETUDATA(-2);
				size_t count = (size_t)luaL_checkinteger(L, -1);
				std::string buffer;
				buffer.resize(count);
				uint64_t cc = (uint64_t)count;
				uint64_t realread = 0u;
				bool ok = FCYERR_OK == p->handle->ReadBytes((uint8_t*)buffer.data(), cc, &realread);
				if (ok) {
					lua_pushstring(L, buffer.c_str());
				}
				else {
					lua_pushnil(L);
				}
				lua_Integer rread = (lua_Integer)realread;
				lua_pushinteger(L, rread);
				return 2;
			}
			static int WriteBytes(lua_State* L)noexcept
			{
				GETUDATA(-2);
				if (p->handle->CanWrite()) {
					std::string buffer = luaL_checkstring(L, -1);
					uint64_t len = (uint64_t)buffer.size();
					uint64_t realwirte = 0u;
					lua_pushboolean(L, FCYERR_OK == p->handle->WriteBytes((uint8_t*)buffer.data(), len, &realwirte));
					lua_Integer rwrite = (lua_Integer)realwirte;
					lua_pushinteger(L, rwrite);
					return 2;
				}
				else {
					return luaL_error(L, "Failed to write data. Writing data is not supported.");
				}
			}
			static int Lock(lua_State* L)noexcept
			{
				GETUDATA(-1);
				p->handle->Lock();
				return 0;
			}
			static int TryLock(lua_State* L)noexcept
			{
				GETUDATA(-1);
				lua_pushboolean(L, FCYERR_OK == p->handle->TryLock());
				return 1;
			}
			static int Unlock(lua_State* L)noexcept
			{
				GETUDATA(-1);
				p->handle->Unlock();
				return 0;
			}

			static int Meta_ToString(lua_State* L)noexcept
			{
				GETUDATA(-1);
				std::ignore = p;
				::lua_pushstring(L, LUASTG_LUA_TYPENAME_IO_STREAM);
				return 1;
			}
			static int Meta_GC(lua_State* L)noexcept
			{
				GETUDATA(-1);
				if (p->handle != nullptr) {
					p->handle->Release();
					p->handle = nullptr;
				}
				return 0;
			}
		#undef GETUDATA
		};

		luaL_Reg tMethods[] =
		{
			{ "CanWrite", &Function::CanWrite },
			{ "CanResize", &Function::CanResize },
			{ "GetLength", &Function::GetLength },
			{ "SetLength", &Function::SetLength },
			{ "GetPosition", &Function::GetPosition },
			{ "SetPosition", &Function::SetPosition },
			{ "ReadBytes", &Function::ReadBytes },
			{ "WriteBytes", &Function::WriteBytes },
			{ "Lock", &Function::Lock },
			{ "TryLock", &Function::TryLock },
			{ "Unlock", &Function::Unlock },
			{ NULL, NULL }
		};

		luaL_Reg tMetaTable[] =
		{
			{ "__tostring", &Function::Meta_ToString },
			{ "__gc", &Function::Meta_GC },
			{ NULL, NULL }
		};

		RegisterClassIntoTable(L, ".Stream", tMethods, LUASTG_LUA_TYPENAME_IO_STREAM, tMetaTable);
	}

	void StreamWrapper::CreateAndPush(lua_State* L, fcyStream* handle) {
		StreamWrapper::Wrapper* p = static_cast<StreamWrapper::Wrapper*>(lua_newuserdata(L, sizeof(StreamWrapper::Wrapper))); // udata
		new(p) StreamWrapper::Wrapper();
		p->handle = handle;
		p->handle->AddRef();
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_IO_STREAM); // udata mt
		lua_setmetatable(L, -2); // udata 
	}
}
