#include "LuaBinding/LuaWrapper.hpp"
#include "Core/FileManager.hpp"
#include "utility/path.hpp"
#include "AppFrame.h"

namespace luastg::binding {

	struct Archive::Wrapper
	{
		uint64_t uuid;
	};

	#define getself Archive::Wrapper* self = static_cast<Archive::Wrapper*>(luaL_checkudata(L, 1, LUASTG_LUA_TYPENAME_ARCHIVE));
	#define getthis (GFileManager().getFileArchiveByUUID(self->uuid))

	void Archive::Register(lua_State* L)noexcept
	{
		struct Function
		{
			static int IsValid(lua_State* L)
			{
				// self
				getself;
				::lua_pushboolean(L, !getthis.empty());
				return 1;
			}
			static int EnumFiles(lua_State* L)
			{
				// ??? self searchpath
				getself;
				auto& zip = getthis;
				if (!zip.empty()) {
					std::string frompathattr = luaL_checkstring(L, -1);
					utility::path::to_slash(frompathattr);//转换为'/'分隔符
					if ((frompathattr.size() == 1) && (frompathattr.back() == '/')) {
						frompathattr.pop_back();//根目录不需要分隔符
					}
					else if ((frompathattr.size() > 0) && (frompathattr.back() != '/')) {
						frompathattr.push_back('/');//补充一个分隔符
					}
					std::string_view frompath = frompathattr; //目标路径
					lua_newtable(L);// ??? self searchpath t 
					int i = 1;
					for (size_t index = 0; index < zip.getCount(); index++) {
						std::string topath(zip.getName(index)); //要比较的路径
						if (frompath.size() >= topath.size()) {
							continue; // 短的直接pass
						}
						else {
							std::string_view path(&topath[0], frompath.size()); //前导部分
							if (path == frompath) {
								std::string_view path2(&topath[frompath.size()], topath.size() - frompath.size());//剩余部分
								int count = 0;
								for (auto& c : path2) {
									if (c == '/') {
										count++;
									}
								}
								bool flag = false;
								if (count <= 0) {
									flag = true;//没有别的分割符，是查找路径下的文件
								}
								else if ((count == 1) && (path2.back() == '/')) {
									flag = true;//有一个分割符，是查找路径下一级的文件夹
								}
								if (flag) {
									lua_pushinteger(L, i);// ??? self searchpath t i 
									lua_newtable(L);// ??? self searchpath t i tt 
									lua_pushinteger(L, 1);// ??? self searchpath t i tt 1 
									lua_pushstring(L, topath.c_str());// ??? self searchpath t i tt 1 s 
									lua_settable(L, -3);// ??? self searchpath t i tt 
									lua_pushinteger(L, 2);// ??? self searchpath t i tt 2 
									lua_pushboolean(L, (topath.back() == '/'));// ??? self searchpath t i tt 2 bool //以分隔符结尾的都是文件夹
									lua_settable(L, -3);// ??? self searchpath t i tt 
									lua_settable(L, -3);// ??? self searchpath t 

									i++;
								}
							}
						}
					}
				}
				else {
					lua_newtable(L); // ??? self searchpath t 
				}
				return 1;
			}
			static int ListFiles(lua_State* L)
			{
				// self
				getself;
				auto& zip = getthis;
				if (!zip.empty()) {
					lua_newtable(L);// ??? self t 
					int i = 1;
					for (size_t index = 0; index < zip.getCount(); index++) {
						std::string topath(zip.getName(index));

						lua_pushinteger(L, i);// ??? self searchpath t i 
						lua_newtable(L);// ??? self searchpath t i tt 
						lua_pushinteger(L, 1);// ??? self searchpath t i tt 1 
						lua_pushstring(L, topath.c_str());// ??? self searchpath t i tt 1 s 
						lua_settable(L, -3);// ??? self searchpath t i tt 
						lua_pushinteger(L, 2);// ??? self searchpath t i tt 2 
						lua_pushboolean(L, (topath.back() == '/'));// ??? self searchpath t i tt 2 bool //以分隔符结尾的都是文件夹
						lua_settable(L, -3);// ??? self searchpath t i tt 
						lua_settable(L, -3);// ??? self searchpath t 

						i++;
					}
				}
				else {
					lua_newtable(L); // ??? self t 
				}
				return 1;
			}
			static int FileExist(lua_State* L)
			{
				// self path
				getself;
				auto& zip = getthis;
				if (!zip.empty())
				{
					std::string frompath = luaL_checkstring(L, -1);
					utility::path::to_slash(frompath);
					lua_pushboolean(L, zip.contain(frompath));
				}
				else
				{
					lua_pushboolean(L, false);
				}
				return 1;
			}
			static int GetName(lua_State* L) {
				getself;
				auto& zip = getthis;
				if (!zip.empty())
				{
					lua_pushstring(L, zip.getFileArchiveName().data());
				}
				else
				{
					lua_pushnil(L);
				}
				return 1;
			}
			static int GetPriority(lua_State* L)
			{
				lua_pushinteger(L, 0);
				return 1;
			}
			static int SetPriority(lua_State*)
			{
				return 0;
			}

			static int Meta_ToString(lua_State* L)noexcept
			{
				getself;
				auto& zip = getthis;
				if (!zip.empty())
				{
					lua_pushfstring(L, "lstg.Archive(%llu, \"%s\")", self->uuid, zip.getFileArchiveName().data());
				}
				else
				{
					lua_pushfstring(L, "lstg.Archive(%llu)", self->uuid);
				}
				return 1;
			}
		};
		
		luaL_Reg tMethods[] =
		{
			{ "IsValid", &Function::IsValid },
			{ "EnumFiles", &Function::EnumFiles },
			{ "ListFiles", &Function::ListFiles },
			{ "FileExist", &Function::FileExist },
			{ "GetName", &Function::GetName },
			{ "GetPriority", &Function::GetPriority },
			{ "SetPriority", &Function::SetPriority },
			{ NULL, NULL }
		};

		luaL_Reg tMetaTable[] =
		{
			{ "__tostring", &Function::Meta_ToString },
			{ NULL, NULL }
		};
		
		RegisterMethodD(L, LUASTG_LUA_TYPENAME_ARCHIVE, tMethods, tMetaTable);
	}

	void Archive::CreateAndPush(lua_State* L, uint64_t uuid)noexcept {
		Archive::Wrapper* p = static_cast<Archive::Wrapper*>(lua_newuserdata(L, sizeof(Archive::Wrapper))); // udata
		p->uuid = uuid;
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_ARCHIVE); // udata mt
		lua_setmetatable(L, -2); // udata 
	}
}
