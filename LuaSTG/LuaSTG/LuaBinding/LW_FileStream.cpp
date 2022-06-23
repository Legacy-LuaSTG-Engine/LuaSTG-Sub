#include "LuaWrapper/LuaWrapper.hpp"
#include "utility/encoding.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <wrl/wrappers/corewrappers.h>

namespace LuaSTG
{
	constexpr char const ClassID[] = "lstg.File";

	inline LARGE_INTEGER lua_to_LARGE_INTEGER(lua_State* L, int idx)
	{
		lua_Integer const arg = luaL_checkinteger(L, idx);
		return LARGE_INTEGER{ .QuadPart = arg };
	}
	inline int lua_push_LARGE_INTEGER(lua_State* L, int argc, LARGE_INTEGER N)
	{
		if constexpr (sizeof(lua_Integer) >= sizeof(LARGE_INTEGER))
		{
			lua_pushinteger(L, N.QuadPart);
			return argc + 1;
		}
		else
		{
			if (N.QuadPart > INT32_MAX || N.QuadPart < INT32_MIN)
			{
				return luaL_error(L, "numerical overflow, file too large");
			}
			else
			{
				lua_pushinteger(L, (lua_Integer)N.QuadPart);
				return argc + 1;
			}
		}
	}
	inline std::string_view lua_to_string_view(lua_State* L, int idx)
	{
		size_t len = 0;
		char const* str = luaL_checklstring(L, idx, &len);
		return std::string_view(str, len);
	}
	inline std::string_view lua_opt_string_view(lua_State* L, int idx, std::string_view const def)
	{
		if (lua_isstring(L, idx))
		{
			size_t len = 0;
			char const* str = lua_tolstring(L, idx, &len);
			return std::string_view(str, len);
		}
		else
		{
			return def;
		}
	}

	struct File
	{
		HANDLE handle;

		static int Open(lua_State* L) noexcept
		{
			std::string_view const arg1 = lua_to_string_view(L, 1);
			std::string_view const arg2 = lua_opt_string_view(L, 2, "r");
			DWORD access = FILE_GENERIC_READ;
			DWORD share = FILE_SHARE_READ;
			DWORD create = OPEN_EXISTING;
			DWORD flag = FILE_ATTRIBUTE_NORMAL;
			if (arg2 == "r" || arg2 == "rb")
			{
				access = FILE_GENERIC_READ;
				create = OPEN_EXISTING;
			}
			else if (arg2 == "w" || arg2 == "wb")
			{
				access = FILE_GENERIC_WRITE;
				create = CREATE_ALWAYS;
			}
			else if (arg2 == "a" || arg2 == "ab")
			{
				access = FILE_GENERIC_WRITE;
				create = OPEN_ALWAYS;
			}
			if (arg2 == "r+" || arg2 == "r+b")
			{
				access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
				create = OPEN_EXISTING;
			}
			else if (arg2 == "w+" || arg2 == "w+b")
			{
				access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
				create = CREATE_ALWAYS;
			}
			else if (arg2 == "a+" || arg2 == "a+b")
			{
				access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
				create = OPEN_ALWAYS;
			}
			else
			{
				return luaL_error(L, "invalid parameter #2");
			}
			int const required = MultiByteToWideChar(CP_UTF8, 0, arg1.data(), (int)arg1.size(), NULL, 0);
			if (required <= 0)
			{
				return _PushBoolResult(L, false, "MultiByteToWideChar");
			}
			std::wstring path(required, L'\0');
			int const result = MultiByteToWideChar(CP_UTF8, 0, arg1.data(), (int)arg1.size(), path.data(), required);
			if (result <= 0)
			{
				return _PushBoolResult(L, false, "MultiByteToWideChar");
			}
			Microsoft::WRL::Wrappers::FileHandle file;
			file.Attach(CreateFileW(path.c_str(), access, share, NULL, create, flag, NULL));
			if (!file.IsValid())
			{
				return _PushBoolResult(L, false, "CreateFileW");
			}
			lua_pushboolean(L, true);
			std::ignore = Create(L, file.Detach());
			return 2;
		}
		
		static int Close(lua_State* L) noexcept
		{
			File* self = SafeCast(L, 1);
			BOOL const result = CloseHandle(self->handle);
			self->handle = INVALID_HANDLE_VALUE; // always
			return _PushBoolResult(L, result, "CloseHandle");
		}
		static int Read(lua_State* L) noexcept
		{
			lua_Integer const arg2 = luaL_checkinteger(L, 2);
			if (arg2 < 0)
			{
				return luaL_error(L, "invalid parameter #2, must be greater than or equal to 0");
			}
			else if (arg2 == 0)
			{
				lua_pushboolean(L, true);
				lua_pushstring(L, "");
				return 2;
			}
			if constexpr (sizeof(lua_Integer) > sizeof(DWORD))
			{
				if (arg2 > INT32_MAX)
				{
					return luaL_error(L, "invalid parameter #2, must be less than 2147483648");
				}
			}
			File* self = SafeCast(L, 1);
			DWORD const size = (DWORD)arg2;
			DWORD read_size = 0;
			std::vector<char> buffer(size);
			BOOL const result = ReadFile(self->handle, buffer.data(), size, &read_size, NULL);
			if (result)
			{
				lua_pushboolean(L, true);
				lua_pushlstring(L, buffer.data(), read_size);
				return 2;
			}
			else
			{
				return _PushBoolResult(L, false, "ReadFile");
			}
		}
		static int Write(lua_State* L) noexcept
		{
			std::string_view const arg2 = lua_to_string_view(L, 2);
			if (arg2.size() > 0x7FFFFFFFu)
			{
				return luaL_error(L, "invalid parameter #2, string length must be less than 2147483648");
			}
			File* self = SafeCast(L, 1);
			DWORD write_size = 0;
			BOOL const result = WriteFile(self->handle, arg2.data(), (DWORD)arg2.size(), &write_size, NULL);
			if (result)
			{
				lua_pushboolean(L, true);
				lua_pushinteger(L, (lua_Integer)write_size);
				return 2;
			}
			else
			{
				return _PushBoolResult(L, false, "WriteFile");
			}
		}
		static int SetSize(lua_State* L) noexcept
		{
			File* self = SafeCast(L, 1);
			LARGE_INTEGER const file_size = lua_to_LARGE_INTEGER(L, 2);
			LARGE_INTEGER last_tell = {};
			if (!SetFilePointerEx(self->handle, LARGE_INTEGER{}, &last_tell, FILE_CURRENT))
			{
				return _PushBoolResult(L, false, "SetFilePointerEx");
			}
			if (!SetEndOfFile(self->handle))
			{
				return _PushBoolResult(L, false, "SetEndOfFile");
			}
			BOOL const result = SetFilePointerEx(self->handle, last_tell, NULL, FILE_BEGIN);
			return _PushBoolResult(L, result, "SetFilePointerEx");
		}
		static int GetSize(lua_State* L) noexcept
		{
			File* self = SafeCast(L, 1);
			LARGE_INTEGER file_size = {};
			BOOL const result = GetFileSizeEx(self->handle, &file_size);
			if (result)
			{
				lua_pushboolean(L, true);
				return lua_push_LARGE_INTEGER(L, 1, file_size);
			}
			else
			{
				return _PushBoolResult(L, false, "GetFileSizeEx");
			}
		}
		static int SetPosition(lua_State* L) noexcept
		{
			File* self = SafeCast(L, 1);
			LARGE_INTEGER const file_seek = lua_to_LARGE_INTEGER(L, 2);
			DWORD origin = FILE_BEGIN;
			if (lua_gettop(L) >= 3)
			{
				if (lua_isnumber(L, 3))
				{
					lua_Integer const arg3 = luaL_checkinteger(L, 3);
					if (arg3 < FILE_BEGIN || arg3 > FILE_END)
					{
						return luaL_error(L, "invalid parameter #3, must be 0, 1 or 2 (or string \"set\", \"cur\" or \"end\")");
					}
					origin = (DWORD)arg3;
				}
				else
				{
					char const* const options[3] = {
						"set",
						"cur",
						"end",
					};
					origin = (DWORD)luaL_checkoption(L, 3, NULL, options);
				}
			}
			BOOL const result = SetFilePointerEx(self->handle, file_seek, NULL, origin);
			return _PushBoolResult(L, result, "SetFilePointerEx");
		}
		static int GetPosition(lua_State* L) noexcept
		{
			File* self = SafeCast(L, 1);
			LARGE_INTEGER file_tell = {};
			BOOL const result = SetFilePointerEx(self->handle, LARGE_INTEGER{}, &file_tell, FILE_CURRENT);
			if (result)
			{
				lua_pushboolean(L, true);
				return lua_push_LARGE_INTEGER(L, 1, file_tell);
			}
			else
			{
				return _PushBoolResult(L, false, "SetFilePointerEx");
			}
		}
		static int Flush(lua_State* L) noexcept
		{
			File* self = SafeCast(L, 1);
			BOOL const result = FlushFileBuffers(self->handle);
			return _PushBoolResult(L, result, "FlushFileBuffers");
		}

		static int __gc(lua_State* L) noexcept
		{
			return Close(L);
		}
		static int __tostring(lua_State* L) noexcept
		{
			File* self = Cast(L, 1);
			if (self->handle != INVALID_HANDLE_VALUE)
			{
				lua_pushfstring(L, "lstg.File(%p)", self->handle);
			}
			else
			{
				lua_pushstring(L, "lstg.File(null)");
			}
			return 1;
		}

		static int _PushBoolResult(lua_State* L, bool result, char const* api)
		{
			if (result)
			{
				lua_pushboolean(L, true);
				return 1;
			}
			else
			{
				DWORD const error = GetLastError();
				lua_pushboolean(L, false);
				lua_pushfstring(L, "%s failed (GetLastError = %d)", api, static_cast<long>(error));
				lua_pushinteger(L, static_cast<long>(error));
				return 3;
			}
		}

		static File* Cast(lua_State* L, int idx) noexcept
		{
			return (File*)luaL_checkudata(L, idx, ClassID);
		}
		static File* SafeCast(lua_State* L, int idx) noexcept
		{
			File* self = Cast(L, 1);
			if (self->handle == INVALID_HANDLE_VALUE)
			{
				luaL_error(L, "attempt to use a invalid (null) file");
			}
			return self;
		}
		static File* Create(lua_State* L, HANDLE handle) noexcept
		{
			File* self = (File*)lua_newuserdata(L, sizeof(File));	// ??? udata
			self->handle = handle;
			luaL_getmetatable(L, ClassID);							// ??? udata mt
			lua_setmetatable(L, -2);								// ??? udata
			return self;
		}
		static void Register(lua_State* L)
		{
			luaL_Reg const lib[] = {
				{ "Open", &Open },
				{ "Close", &Close },
				{ "read", &Read },
				{ "write", &Write },
				{ "SetSize", &SetSize },
				{ "GetSize", &GetSize },
				{ "SetPosition", &SetPosition },
				{ "GetPosition", &GetPosition },
				{ "Flush", &Flush },
				{ NULL, NULL },
			};
			luaL_Reg const mt[] = {
				{ "__index", NULL },
				{ "__gc", &__gc },
				{ "__tostring", &__tostring },
				{ NULL, NULL },
			};
			luaL_Reg const empty[] = {
				{ NULL, NULL },
			};
			luaL_register(L, "lstg.File", lib);			// ??? lib
			luaL_newmetatable(L, ClassID);				// ??? lib mt
			luaL_register(L, NULL, mt);					// ??? lib mt
			lua_pushvalue(L, -2);						// ??? lib mt lib
			lua_setfield(L, -2, "__index");				// ??? lib mt
			lua_pop(L, 1);								// ??? lib
			luaL_register(L, "lstg", empty);			// ??? lib lstg
			lua_pushvalue(L, -2);						// ??? lib lstg lib
			lua_setfield(L, -2, "File");				// ??? lib lstg
			lua_pop(L, 2);								// ???
		}

		
	};
}
