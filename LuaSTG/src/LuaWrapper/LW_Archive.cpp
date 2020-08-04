#include <string>
#include <string_view>
#include "Global.h"
#include "AppFrame.h"
#include "LuaWrapper\LuaWrapper.hpp"
#include "E2DFilePath.hpp"
#include "E2DFileManager.hpp"

using namespace std;
using namespace LuaSTGPlus;
using namespace Eyes2D::IO;

struct ArchiveWrapper::Wrapper {
	unsigned int uid;
	Archive* handle;
};

void ArchiveWrapper::Register(lua_State* L)LNOEXCEPT {
	struct Function {
		static int IsValid(lua_State* L) {
			// self
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_ARCHIVE));
			::lua_pushboolean(L, (LFMGR.GetArchiveByUID(p->uid) != nullptr));
			return 1;
		}
		static int EnumFiles(lua_State* L) {
			// ??? self searchpath
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -2, LUASTG_LUA_TYPENAME_ARCHIVE));
			Archive* zip = LFMGR.GetArchiveByUID(p->uid);
			if (zip != nullptr) {
				string frompathattr = luaL_checkstring(L, -1);
				Eyes2D::Platform::PathFormatLinux(frompathattr);//转换为'/'分隔符
				if ((frompathattr.size() == 1) && (frompathattr.back() == '/')) {
					frompathattr.pop_back();//根目录不需要分隔符
				}
				else if ((frompathattr.size() > 0) && (frompathattr.back() != '/')) {
					frompathattr.push_back('/');//补充一个分隔符
				}
				string_view frompath = frompathattr; //目标路径
				lua_newtable(L);// ??? self searchpath t 
				int i = 1;
				for (long long index = 0; index < zip->GetFileCount(); index++) {
					string topath = zip->GetFileName(index); //要比较的路径
					if (frompath.size() >= topath.size()) {
						continue; // 短的直接pass
					}
					else {
						string_view path(&topath[0], frompath.size()); //前导部分
						if (path == frompath) {
							string_view path2(&topath[frompath.size()], topath.size() - frompath.size());//剩余部分
							int count = 0;
							for (auto& i : path2) {
								if (i == '/') {
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
		static int ListFiles(lua_State* L) {
			// ??? self
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_ARCHIVE));
			Archive* zip = LFMGR.GetArchiveByUID(p->uid);
			if (zip != nullptr) {
				lua_newtable(L);// ??? self t 
				int i = 1;
				for (long long index = 0; index < zip->GetFileCount(); index++) {
					string topath = zip->GetFileName(index);

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
		static int FileExist(lua_State* L) {
			// ??? self path
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -2, LUASTG_LUA_TYPENAME_ARCHIVE));
			Archive* zip = LFMGR.GetArchiveByUID(p->uid);
			if (zip != nullptr) {
				string frompath = luaL_checkstring(L, -1);
				Eyes2D::Platform::PathFormatLinux(frompath);//转换为'/'分隔符
				lua_pushboolean(L, zip->FileExist(frompath.c_str()));
			}
			else {
				lua_pushboolean(L, false);
			}
			return 1;
		}
		static int GetName(lua_State* L) {
			// ??? self
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_ARCHIVE));
			Archive* zip = LFMGR.GetArchiveByUID(p->uid);
			if (zip != nullptr) {
				lua_pushstring(L, zip->GetArchivePath());
			}
			else {
				lua_pushnil(L);
			}
			return 1;
		}
		static int GetPriority(lua_State* L) {
			// ??? self
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_ARCHIVE));
			Archive* zip = LFMGR.GetArchiveByUID(p->uid);
			if (zip != nullptr) {
				lua_pushinteger(L, zip->GetPriority());
			}
			else {
				lua_pushnil(L);
			}
			return 0;
		}
		static int SetPriority(lua_State* L) {
			// ??? self priority
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -2, LUASTG_LUA_TYPENAME_ARCHIVE));
			int priority = luaL_checkinteger(L, -1);
			LFMGR.SetArchivePriorityByUID(p->uid, priority);
			return 0;
		}

		static int Meta_ToString(lua_State* L)LNOEXCEPT
		{
			// ??? self 
			ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_ARCHIVE));
			lua_pushfstring(L, "lstg.Archive object");
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
	
	LuaSTGPlus::RegisterMethodD(L, LUASTG_LUA_TYPENAME_ARCHIVE, tMethods, tMetaTable);
}

void ArchiveWrapper::CreateAndPush(lua_State* L, unsigned int uid)LNOEXCEPT {
	ArchiveWrapper::Wrapper* p = static_cast<ArchiveWrapper::Wrapper*>(lua_newuserdata(L, sizeof(ArchiveWrapper::Wrapper))); // udata
	new(p) ArchiveWrapper::Wrapper(); // udata
	p->uid = uid;
	luaL_getmetatable(L, LUASTG_LUA_TYPENAME_ARCHIVE); // udata mt
	lua_setmetatable(L, -2); // udata 
}
