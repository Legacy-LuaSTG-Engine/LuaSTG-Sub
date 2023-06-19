﻿#include <string>
#include <filesystem>
#include "AppFrame.h"
#include "LuaWrapper\LuaWrapper.hpp"

#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"
#include "utility/path.hpp"

#ifdef DrawText
#undef DrawText
#endif

using namespace std;
using namespace LuaSTGPlus;

void FileManagerWrapper::Register(lua_State* L)LNOEXCEPT {
	
	struct Wrapper {
		static int LoadArchive(lua_State* L) {
			// path ???
			int argn = lua_gettop(L);
			bool ret = false;
			switch (argn) {
			case 1:
				// path
				ret = GFileManager().loadFileArchive(luaL_checkstring(L, 1));
				break;
			case 2:
				if (lua_isnumber(L, 2)) {
					// path lv
					ret = GFileManager().loadFileArchive(luaL_checkstring(L, 1));
				}
				else {
					// path pw
					ret = GFileManager().loadFileArchive(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
				}
				break;
			case 3:
				// path lv pw
				ret = GFileManager().loadFileArchive(luaL_checkstring(L, 1), luaL_checkstring(L, 3));
				break;
			}
			if (ret) {
				auto& zip = GFileManager().getFileArchive(luaL_checkstring(L, 1));
				if (!zip.empty()) {
					ArchiveWrapper::CreateAndPush(L, zip.getUUID());
				}
				else {
					lua_pushnil(L);
				}
			}
			else {
				lua_pushnil(L);
			}
			return 1;
		}
		static int UnloadArchive(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			if (GFileManager().containFileArchive(s)) {
				GFileManager().unloadFileArchive(s);
				lua_pushboolean(L, true);
			}
			else {
				lua_pushboolean(L, false);
			}
			return 1;
		}
		static int UnloadAllArchive(lua_State* L) {
			GFileManager().unloadAllFileArchive();
			return 0;
		}
		static int ArchiveExist(lua_State* L) {
			lua_pushboolean(L,
				GFileManager().containFileArchive(
					luaL_checkstring(L, 1)));
			return 1;
		}
		static int GetArchive(lua_State* L) {
			auto& zip = GFileManager().getFileArchive(luaL_checkstring(L, 1));
			if (!zip.empty()) {
				ArchiveWrapper::CreateAndPush(L, zip.getUUID());
			}
			else {
				lua_pushnil(L);
			}
			return 1;
		}
		static int EnumArchives(lua_State* L) {
			lua_newtable(L); // ??? t 
			for (unsigned int index = 1; index <= GFileManager().getFileArchiveCount(); index++) {
				auto& ref = GFileManager().getFileArchive(index - 1);
				if (!ref.empty()) {
					lua_pushinteger(L, (lua_Integer)index); // ??? t index 
					lua_newtable(L); // ??? t index tt 
					lua_pushinteger(L, 1); // ??? t index tt 1 
					lua_pushstring(L, ref.getFileArchiveName().data()); // ??? t index tt 1 s
					lua_settable(L, -3); // ??? t index tt 
					lua_pushinteger(L, 2); // ??? t index tt 2 
					lua_pushinteger(L, 0); // ??? t index tt 2 priority 
					lua_settable(L, -3); // ??? t index tt 
					lua_settable(L, -3); // ??? t 
				}
			}
			return 1;
		}
		static int EnumFiles(lua_State* L) {
			// path ext 
			string searchpath = luaL_checkstring(L, 1);
			string extpath = "";
			bool checkext = false;
			if (lua_gettop(L) == 2) {
				extpath = luaL_checkstring(L, 2);
				extpath = "." + extpath; // 修改成带点格式
				checkext = true;
			}

			utility::path::to_slash(searchpath);
			if ((searchpath == ".") || (searchpath == "/") || (searchpath == "./")) {
				searchpath = "";// "/" or "." 不需要
			}
			else if ((searchpath.size() > 1) && (searchpath.back() != '/')) {
				searchpath.push_back('/');//在搜索路径后面手动添加一个分隔符
			}

			lua_newtable(L); // ??? path t 
			filesystem::path path = searchpath;
			unsigned int index = 1;
			if (filesystem::is_directory(path)) {
				for (auto& p : filesystem::directory_iterator(path)) {
					if (filesystem::is_regular_file(p.path()) || filesystem::is_directory(p.path())) {
						if (checkext && ((p.path().extension().string() != extpath) || filesystem::is_directory(p.path()))) {
							continue;
						}
						lua_pushinteger(L, index); // ??? path t index 
						lua_newtable(L); // ??? path t index tt 
						lua_pushinteger(L, 1); // ??? path t index tt 1 
						string u8path = utility::encoding::to_utf8(p.path().wstring());
						if (filesystem::is_directory(p.path())) {
							u8path.push_back('/');//在目录路径后面手动添加一个分隔符
						}
						lua_pushstring(L, u8path.c_str()); // ??? path t index tt 1 fpath 
						lua_settable(L, -3); // ??? path t index tt 
						lua_pushinteger(L, 2); // ??? path t index tt 2 
						lua_pushboolean(L, filesystem::is_directory(p.path())); // ??? path t index tt 2 bool //为目录时该项为真
						lua_settable(L, -3); // ??? path t index tt 
						lua_settable(L, -3); // ??? path t 
						index++;
					}
				}
			}
			return 1;
		}
		static int EnumFilesEx(lua_State* L) {
			// ??? path ext 
			string searchpath = luaL_checkstring(L, 1);
			string extpath = "";
			bool checkext = false;
			if (lua_gettop(L) == 2) {
				extpath = luaL_checkstring(L, 2);
				extpath = "." + extpath; // 修改成带点格式
				checkext = true;
			}

			utility::path::to_slash(searchpath);
			if ((searchpath == ".") || (searchpath == "/") || (searchpath == "./")) {
				searchpath = "";// "/" or "." 不需要
			}
			else if ((searchpath.size() > 1) && (searchpath.back() != '/')) {
				searchpath.push_back('/');//在搜索路径后面手动添加一个分隔符
			}

			lua_newtable(L); // ??? path t 
			filesystem::path path = searchpath;
			unsigned int index = 1;
			if (filesystem::is_directory(path)) {
				for (auto& p : filesystem::directory_iterator(path)) {
					if (filesystem::is_regular_file(p.path()) || filesystem::is_directory(p.path())) {
						string _s = p.path().extension().string();
						if (checkext && ((p.path().extension().string() != extpath) || filesystem::is_directory(p.path()))) {
							continue;
						}
						lua_pushinteger(L, index); // ??? path t index 
						lua_newtable(L); // ??? path t index tt 
						lua_pushinteger(L, 1); // ??? path t index tt 1 
						string u8path = utility::encoding::to_utf8(p.path().wstring());
						if (filesystem::is_directory(p.path())) {
							u8path.push_back('/');//在目录路径后面手动添加一个分隔符
						}
						lua_pushstring(L, u8path.c_str()); // ??? path t index tt 1 fpath 
						lua_settable(L, -3); // ??? path t index tt 
						lua_pushinteger(L, 2); // ??? path t index tt 2 
						lua_pushboolean(L, filesystem::is_directory(p.path())); // ??? path t index tt 2 bool //为目录时该项为真
						lua_settable(L, -3); // ??? path t index tt 
						lua_settable(L, -3); // ??? path t 
						index++;
					}
				}
			}
			for (auto z = 0; z < GFileManager().getFileArchiveCount(); z++) {
				auto& zip = GFileManager().getFileArchive(z);
				if (!zip.empty()) {
					string_view frompath = searchpath; //目标路径
					int i = 1;
					for (auto f = 0; f < zip.getCount(); f++) {
						string topath(zip.getName(f)); //要比较的路径
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
									filesystem::path checkpath(topath);
									if (checkext && ((checkpath.extension().string() != extpath) || (topath.back() == '/'))) {
										continue;
									}
									lua_pushinteger(L, index);// ??? self searchpath t i 
									lua_newtable(L);// ??? self searchpath t i tt 
									lua_pushinteger(L, 1);// ??? self searchpath t i tt 1 
									lua_pushstring(L, topath.c_str());// ??? self searchpath t i tt 1 s 
									lua_settable(L, -3);// ??? self searchpath t i tt 
									lua_pushinteger(L, 2);// ??? self searchpath t i tt 2 
									lua_pushboolean(L, (topath.back() == '/'));// ??? self searchpath t i tt 2 bool //以分隔符结尾的都是文件夹
									lua_settable(L, -3);// ??? self searchpath t i tt 
									lua_pushinteger(L, 3);// ??? self searchpath t i tt 3 
									lua_pushstring(L, zip.getFileArchiveName().data());// ??? self searchpath t i tt 3 s 
									lua_settable(L, -3);// ??? self searchpath t i tt 
									lua_settable(L, -3);// ??? self searchpath t 
									index++;
								}
							}
						}
					}
				}
			}
			return 1;
		}
		static int FileExist(lua_State* L) {
			// ??? filepath
			lua_pushboolean(L, GFileManager().contain(luaL_checkstring(L, -1)));
			return 1;
		}
		static int FileExistEx(lua_State* L) {
			// ??? filepath
			lua_pushboolean(L, GFileManager().containEx(luaL_checkstring(L, -1)));
			return 1;
		}
	};

	luaL_Reg tMethods[] = {
		{ "LoadArchive", &Wrapper::LoadArchive },
		{ "UnloadArchive", &Wrapper::UnloadArchive },
		{ "UnloadAllArchive", &Wrapper::UnloadAllArchive },
		{ "ArchiveExist", &Wrapper::ArchiveExist },
		{ "GetArchive", &Wrapper::GetArchive },
		{ "EnumArchives", &Wrapper::EnumArchives },
		{ "EnumFiles", &Wrapper::EnumFiles },
		{ "EnumFilesEx", &Wrapper::EnumFilesEx },
		{ "FileExist", &Wrapper::FileExist },
		{ "FileExistEx", &Wrapper::FileExistEx },
		{ NULL, NULL }
	};
	
	struct FR_Wrapper {
		static int SetFontProvider(lua_State* L) {
			lua_pushboolean(L, LAPP.FontRenderer_SetFontProvider(luaL_checkstring(L, 1)));
			return 1;
		}
		static int SetScale(lua_State* L) {
			float a = (float)luaL_checknumber(L, 1);
			float b = (float)luaL_checknumber(L, 2);
			LAPP.FontRenderer_SetScale(fcyVec2(a, b));
			return 0;
		}
		static int MeasureString(lua_State* L) {
			const fcyRect v = LAPP.FontRenderer_MeasureString(luaL_checkstring(L, 1), lua_toboolean(L, 2));
			lua_pushnumber(L, v.a.x);
			lua_pushnumber(L, v.b.x);
			lua_pushnumber(L, v.b.y);
			lua_pushnumber(L, v.a.y);
			return 4;
		}
		static int MeasureStringWidth(lua_State* L) {
			const float v = LAPP.FontRenderer_MeasureStringWidth(luaL_checkstring(L, 1));
			lua_pushnumber(L, v);
			return 1;
		}
		static int DrawText(lua_State* L) {
			fcyColor color = *static_cast<fcyColor*>(luaL_checkudata(L, 6, LUASTG_LUA_TYPENAME_COLOR));
			fcyVec2 pos = fcyVec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3));
			const bool ret = LAPP.FontRenderer_DrawTextW2(
				luaL_checkstring(L, 1),
				pos,
				(float)luaL_checknumber(L, 4),
				TranslateBlendMode(L, 5),
				color);
			lua_pushboolean(L, ret);
			lua_pushnumber(L, (lua_Number)pos.x);
			lua_pushnumber(L, (lua_Number)pos.y);
			return 3;
		}
		
		static int MeasureTextBoundary(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);
			const fcyRect v = LAPP.FontRenderer_MeasureTextBoundary(str, len);
			lua_pushnumber(L, v.a.x);
			lua_pushnumber(L, v.b.x);
			lua_pushnumber(L, v.b.y);
			lua_pushnumber(L, v.a.y);
			return 4;
		}
		static int MeasureTextAdvance(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);
			const fcyVec2 v = LAPP.FontRenderer_MeasureTextAdvance(str, len);
			lua_pushnumber(L, v.x);
			lua_pushnumber(L, v.y);
			return 2;
		}
		static int RenderText(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);
			fcyVec2 pos = fcyVec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3));
			fcyColor color = *static_cast<fcyColor*>(luaL_checkudata(L, 6, LUASTG_LUA_TYPENAME_COLOR));
			const bool ret = LAPP.FontRenderer_RenderText(
				str, len,
				pos, (float)luaL_checknumber(L, 4),
				TranslateBlendMode(L, 5),
				color);
			lua_pushboolean(L, ret);
			lua_pushnumber(L, (lua_Number)pos.x);
			lua_pushnumber(L, (lua_Number)pos.y);
			return 3;
		}
		
		static int GetFontLineHeight(lua_State* L) {
			lua_pushnumber(L, LAPP.FontRenderer_GetFontLineHeight());
			return 1;
		};
		static int GetFontAscender(lua_State* L) {
			lua_pushnumber(L, LAPP.FontRenderer_GetFontAscender());
			return 1;
		};
		static int GetFontDescender(lua_State* L) {
			lua_pushnumber(L, LAPP.FontRenderer_GetFontDescender());
			return 1;
		};
	};
	
	luaL_Reg FR_Method[] = {
		{ "SetFontProvider", &FR_Wrapper::SetFontProvider },
		{ "SetScale", &FR_Wrapper::SetScale },
		
		{ "MeasureString", &FR_Wrapper::MeasureString }, // 应该弃用
		{ "MeasureStringWidth", &FR_Wrapper::MeasureStringWidth }, // 应该弃用
		{ "DrawText", &FR_Wrapper::DrawText }, // 应该弃用
		
		{ "MeasureTextBoundary", &FR_Wrapper::MeasureTextBoundary },
		{ "MeasureTextAdvance", &FR_Wrapper::MeasureTextAdvance },
		{ "RenderText", &FR_Wrapper::RenderText },
		
		{ "GetFontLineHeight", &FR_Wrapper::GetFontLineHeight },
		{ "GetFontAscender", &FR_Wrapper::GetFontAscender },
		{ "GetFontDescender", &FR_Wrapper::GetFontDescender },
		
		{ NULL, NULL }
	};
	
	lua_getglobal(L, "lstg"); // ??? t 
	
	lua_newtable(L); // ??? t t
	luaL_register(L, NULL, tMethods); // ??? t t 
	lua_setfield(L, -2, "FileManager"); // ??? t 
	
	lua_newtable(L); // ??? t t
	luaL_register(L, NULL, FR_Method); // ??? t t 
	lua_setfield(L, -2, "FontRenderer"); // ??? t 
	
	lua_pop(L, 1); // ??? 
}
