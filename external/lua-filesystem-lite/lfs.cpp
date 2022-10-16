extern "C"
{
#include "lfs.h"
#include "lauxlib.h"
}
#include <string>
#include <filesystem>

#define _lfs_utf8

#ifdef _lfs_utf8
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
inline std::string to_utf8(std::wstring_view const& src)
{
	int const size = WideCharToMultiByte(CP_UTF8, 0, src.data(), (int)src.length(), NULL, 0, NULL, NULL);
	if (size > 0)
	{
		std::string buf(size, '\0');
		int const result = WideCharToMultiByte(CP_UTF8, 0, src.data(), (int)src.length(), buf.data(), size, NULL, NULL);
		if (result == size)
		{
			return buf;
		}
	}
	return "";
}
inline std::wstring to_wide(std::string_view const& src)
{
	int const size = MultiByteToWideChar(CP_UTF8, 0, src.data(), (int)src.length(), NULL, 0);
	if (size > 0)
	{
		std::wstring buf(size, L'\0');
		int const result = MultiByteToWideChar(CP_UTF8, 0, src.data(), (int)src.length(), buf.data(), size);
		if (result == size)
		{
			return buf;
		}
	}
	return L"";
}
inline std::filesystem::path to_path(std::string_view const& str) { return std::filesystem::path(to_wide(str)); }
inline std::string from_path(std::filesystem::path const& p) { return to_utf8(p.generic_wstring()); }
#else
inline std::filesystem::path to_path(std::string_view const& str) { return std::filesystem::path(str); }
inline std::string from_path(std::filesystem::path const& p) { return p.generic_string(); }
#endif

struct lfs_api
{
	static void push_attributes_table(lua_State* L, int o_f_d, int sz) noexcept
	{
		lua_createtable(L, 0, 14);
		set_attributes_table(L, lua_gettop(L), o_f_d, sz);
	}
	static void set_attributes_table(lua_State* L, int idx, int o_f_d, int sz) noexcept
	{
		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "dev"); // on Unix systems, this represents the device that the inode resides on. On Windows systems, represents the drive number of the disk containing the file

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "ino"); // on Unix systems, this represents the inode number. On Windows systems this has no meaning

		char const* o_f_d_sw[3] = {
			"other",
			"file",
			"directory",
		};
		lua_pushstring(L, o_f_d_sw[o_f_d]);
		lua_setfield(L, idx, "mode"); // string representing the associated protection mode (the values could be file, directory, link, socket, named pipe, char device, block device or other)

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "nlink"); // number of hard links to the file

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "uid"); // user-id of owner (Unix only, always 0 on Windows)

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "gid"); // group-id of owner (Unix only, always 0 on Windows)

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "rdev");// on Unix systems, represents the device type, for special file inodes. On Windows systems represents the same as dev

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "access"); // time of last access

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "modification"); // time of last data modification

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "change"); // time of last file status change

		lua_pushinteger(L, sz);
		lua_setfield(L, idx, "size"); // file size, in bytes

		lua_pushstring(L, "-r--r--r--");
		lua_setfield(L, idx, "permissions"); // file permissions string

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "blocks"); // block allocated for file; (Unix only)

		lua_pushinteger(L, 0);
		lua_setfield(L, idx, "blksize"); // optimal file system I/O blocksize; (Unix only)
	}

#define _lfs_error_not_supported return luaL_error(L, "not support")

	static int lfs_attributes(lua_State* L) noexcept
	{
		size_t arg1_size{ 0 };
		char const* arg1 = luaL_checklstring(L, 1, &arg1_size);
		std::string_view const filepath(arg1, arg1_size);
		std::wstring widepath(to_path(filepath));
		if (lua_isstring(L, 2))
		{
			size_t arg2_size{ 0 };
			char const* arg2 = luaL_checklstring(L, 2, &arg2_size);
			std::string_view const request_name(arg2, arg2_size);
			if (request_name == "mode")
			{
				std::error_code ec;
				if (std::filesystem::is_regular_file(widepath, ec))
				{
					std::string_view const mode_file("file");
					lua_pushlstring(L, mode_file.data(), mode_file.length());
					return 1;
				}
				else if (std::filesystem::is_directory(widepath, ec))
				{
					std::string_view const mode_directory("directory");
					lua_pushlstring(L, mode_directory.data(), mode_directory.length());
					return 1;
				}
				else if (std::filesystem::exists(widepath))
				{
					std::string_view const mode_other("other");
					lua_pushlstring(L, mode_other.data(), mode_other.length());
					return 1;
				}
				else
				{
					lua_pushnil(L);
					return 1;
				}
			}
			if (request_name == "size")
			{
				std::error_code ec;
				uintmax_t const size = std::filesystem::file_size(widepath, ec);
				if (size != static_cast<std::uintmax_t>(-1))
				{
					lua_pushinteger(L, (lua_Integer)size);
					return 1;
				}
				else
				{
					lua_pushinteger(L, 0);
					return 1;
				}
			}
			else
			{
				_lfs_error_not_supported;
			}
		}
		else
		{
			int o_f_d = 0;
			int sz = 0;
			std::error_code ec;
			if (std::filesystem::is_regular_file(widepath, ec))
			{
				o_f_d = 1;
			}
			else if (std::filesystem::is_directory(widepath, ec))
			{
				o_f_d = 2;
			}
			uintmax_t const size = std::filesystem::file_size(widepath, ec);
			if (size != static_cast<std::uintmax_t>(-1))
			{
				sz = (lua_Integer)size;
			}
			if (lua_istable(L, 2))
			{
				set_attributes_table(L, 2, o_f_d, sz);
				lua_pushvalue(L, 2);
			}
			else
			{
				push_attributes_table(L, o_f_d, sz);
			}
			return 1;
		}
	}
	static int lfs_chdir(lua_State* L) noexcept
	{
		size_t arg1_size{ 0 };
		char const* arg1 = luaL_checklstring(L, 1, &arg1_size);
		std::string_view const path(arg1, arg1_size);
		std::error_code ec;
		std::filesystem::current_path(to_path(path), ec);
		if (!ec)
		{
			lua_pushboolean(L, true);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			std::string const msg = ec.message();
			lua_pushlstring(L, msg.c_str(), msg.length());
			lua_pushinteger(L, ec.value());
			return 3;
		}
	}

	static int __lfs_lock_dir(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}

	static int lfs_currentdir(lua_State* L) noexcept
	{
		std::error_code ec;
		std::filesystem::path const pwd = std::filesystem::current_path(ec);
		if (!ec)
		{
			std::string const path = from_path(pwd);
			lua_pushlstring(L, path.c_str(), path.length());
			return 1;
		}
		else
		{
			lua_pushnil(L);
			std::string const msg = ec.message();
			lua_pushlstring(L, msg.c_str(), msg.length());
			lua_pushinteger(L, ec.value());
			return 3;
		}
	}
	static int lfs_dir(lua_State* L) noexcept
	{
		size_t arg1_size{ 0 };
		char const* arg1 = luaL_checklstring(L, 1, &arg1_size);
		std::string_view const path(arg1, arg1_size);
		std::error_code ec;
		if (std::filesystem::is_directory(to_path(path), ec))
		{
			lua_pushcfunction(L, &lfs_dir_obj::next);
			lfs_dir_obj::create(L, path);
			return 2;
		}
		else
		{
			return luaL_error(L, "'%s' is not a directory", arg1);
		}
	}

	static int __lfs_lock(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}
	static int __lfs_link(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}

	static int lfs_mkdir(lua_State* L) noexcept
	{
		size_t arg1_size{ 0 };
		char const* arg1 = luaL_checklstring(L, 1, &arg1_size);
		std::string_view const path(arg1, arg1_size);
		std::error_code ec;
		if (std::filesystem::create_directory(to_path(path), ec))
		{
			lua_pushboolean(L, true);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			std::string const msg = ec.message();
			lua_pushlstring(L, msg.c_str(), msg.length());
			lua_pushinteger(L, ec.value());
			return 3;
		}
	}
	static int lfs_rmdir(lua_State* L) noexcept
	{
		size_t arg1_size{ 0 };
		char const* arg1 = luaL_checklstring(L, 1, &arg1_size);
		std::string_view const path(arg1, arg1_size);
		std::error_code ec;
		if (std::filesystem::remove(to_path(path), ec))
		{
			lua_pushboolean(L, true);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			std::string const msg = ec.message();
			lua_pushlstring(L, msg.c_str(), msg.length());
			lua_pushinteger(L, ec.value());
			return 3;
		}
	}

	static int __lfs_setmode(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}
	static int __lfs_symlinkattributes(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}
	static int __lfs_touch(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}
	static int __lfs_unlock(lua_State* L) noexcept
	{
		_lfs_error_not_supported;
	}

	struct lfs_dir_obj
	{
	#define _lfs_dir_class_id "lfs.dir"

		std::filesystem::directory_iterator iter;
		std::filesystem::directory_iterator end;
		std::error_code ec;

		explicit lfs_dir_obj(std::filesystem::path const& path) : iter(path, ec), end() {}
		~lfs_dir_obj() {}

		static int __tostring(lua_State* L) noexcept
		{
			std::ignore = (lfs_dir_obj*)luaL_checkudata(L, 1, _lfs_dir_class_id);
			lua_pushstring(L, _lfs_dir_class_id);
			return 0;
		}
		static int __gc(lua_State* L) noexcept
		{
			lfs_dir_obj* self = (lfs_dir_obj*)luaL_checkudata(L, 1, _lfs_dir_class_id);
			self->~lfs_dir_obj();
			return 0;
		}

		static int next(lua_State* L) noexcept
		{
			lfs_dir_obj* self = (lfs_dir_obj*)luaL_checkudata(L, 1, _lfs_dir_class_id);
			if (self->ec || self->iter == self->end) // MAGIC CODE
			{
				lua_pushnil(L);
			}
			else
			{
				std::string const path = from_path(self->iter->path().filename());
				self->iter.increment(self->ec);
				lua_pushlstring(L, path.c_str(), path.length());
			}
			return 1;
		}
		static int close(lua_State* L) noexcept
		{
			std::ignore = (lfs_dir_obj*)luaL_checkudata(L, 1, _lfs_dir_class_id);
			return 0;
		}

		static void luareg(lua_State* L) noexcept
		{
			luaL_Reg const mt[] = {
				{ "__tostring", &__tostring },
				{ "__gc", &__gc },
				{ NULL, NULL },
			};
			luaL_Reg const lib[] = {
				{ "next", &next },
				{ "close", &close },
				{ NULL, NULL },
			};
			luaL_newmetatable(L, _lfs_dir_class_id);
			lua_createtable(L, 0, 2);
			luaL_register(L, NULL, lib);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);
		}
		static lfs_dir_obj* create(lua_State* L, std::filesystem::path const& p) noexcept
		{
			lfs_dir_obj* self = (lfs_dir_obj*)lua_newuserdata(L, sizeof(lfs_dir_obj));
			new(self) lfs_dir_obj(p);
			luaL_getmetatable(L, _lfs_dir_class_id);
			lua_setmetatable(L, -2);
			return self;
		}
		static lfs_dir_obj* cast(lua_State* L, int idx)
		{
			return (lfs_dir_obj*)luaL_checkudata(L, idx, _lfs_dir_class_id);
		}

	#undef _lfs_dir_class_id
	};

#undef _lfs_error_not_supported

	static void luareg(lua_State* L) noexcept
	{
		lfs_dir_obj::luareg(L);
		luaL_Reg const lib[] = {
			{ "attributes", &lfs_attributes },
			{ "chdir", &lfs_chdir },
			{ "lock_dir", &__lfs_lock_dir },
			{ "currentdir", &lfs_currentdir },
			{ "dir", &lfs_dir },
			{ "lock", &__lfs_lock },
			{ "link", &__lfs_link },
			{ "mkdir", &lfs_mkdir },
			{ "rmdir", &lfs_rmdir },
			{ "setmode", &__lfs_setmode },
			{ "symlinkattributes", &__lfs_symlinkattributes },
			{ "touch", &__lfs_touch },
			{ "unlock", &__lfs_unlock },
			{ NULL, NULL },
		};
		luaL_register(L, "lfs", lib);
	}
};

int luaopen_lfs(lua_State * L)
{
	lfs_api::luareg(L);
    return 1;
}
