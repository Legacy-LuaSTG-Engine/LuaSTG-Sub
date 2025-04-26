#include "LuaBinding/LuaWrapper.hpp"
#include "lua/plus.hpp"
#include "core/FileSystem.hpp"
#include "core/FileSystemOS.hpp"
#include "utility/path.hpp"
#include "AppFrame.h"
#include "utf8.hpp"
#include "GameResource/ResourcePassword.hpp"

namespace {
	bool extractRes(const char* path, const char* target) noexcept {
		core::SmartReference<core::IData> src;
		if (!core::FileSystemManager::readFile(path, src.put())) {
			spdlog::error("[luastg] ExtractRes: 无法读取文件'{}'", path);
			return false;
		}
		if (!core::FileSystemManager::writeFile(target, src.get())) {
			spdlog::error("[luastg] ExtractRes: 无法写入文件'{}'", target);
			return false;
		}
		return true;
	}
}

void luastg::binding::FileManager::Register(lua_State* L)noexcept {
	struct Wrapper {
		static int LoadArchive(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);

			core::SmartReference<core::IFileSystemArchive> archive;
			if (!core::IFileSystemArchive::createFromFile(path, archive.put())) {
				spdlog::error("[luastg] 无法加载资源包'{}'，文件不存在或不是合法的资源包格式（zip）", path);
				ctx.push_value(std::nullopt);
				return 1;
			}

			// 第二个参数位曾经支持设置优先级，现在已失效

			if (ctx.is_string(3)) {
				auto const password = ctx.get_value<std::string_view>(3);
				archive->setPassword(password);
			}
			else if (ctx.is_string(2)) {
				auto const password = ctx.get_value<std::string_view>(2);
				archive->setPassword(password);
			}

			core::FileSystemManager::addFileSystem(path, archive.get());

			Archive::CreateAndPush(L, archive.get());
			return 1;
		}
		static int UnloadArchive(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const name = ctx.get_value<std::string_view>(1);
			core::SmartReference<core::IFileSystemArchive> archive;
			if (core::FileSystemManager::getFileSystemArchiveByPath(name, archive.put())) {
				core::FileSystemManager::removeFileSystem(archive.get());
				ctx.push_value(true);
			}
			else {
				ctx.push_value(false);
			}
			return 1;
		}
		static int UnloadAllArchive(lua_State* L) {
			std::ignore = L;
			core::FileSystemManager::removeAllFileSystem();
			return 0;
		}
		static int ArchiveExist(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const name = ctx.get_value<std::string_view>(1);
			core::SmartReference<core::IFileSystemArchive> archive;
			if (core::FileSystemManager::getFileSystemArchiveByPath(name, archive.put())) {
				ctx.push_value(true);
			}
			else {
				ctx.push_value(false);
			}
			return 1;
		}
		static int GetArchive(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const name = ctx.get_value<std::string_view>(1);
			core::SmartReference<core::IFileSystemArchive> archive;
			if (core::FileSystemManager::getFileSystemArchiveByPath(name, archive.put())) {
				Archive::CreateAndPush(L, archive.get());
			}
			else {
				lua_pushnil(L);
			}
			return 1;
		}
		static int EnumArchives(lua_State* L) {
			lua::stack_t const ctx(L);

			auto const result_table = ctx.create_array();

			core::SmartReference<core::IFileSystemFileSystemEnumerator> enumerator;
			if (!core::FileSystemManager::createFileSystemEnumerator(enumerator.put())) {
				return 1;
			}

			int32_t i{ 1 };
			core::SmartReference<core::IFileSystem> file_system;
			while (enumerator->next(file_system.put())) {
				core::SmartReference<core::IFileSystemArchive> archive;
				if (!file_system->queryInterface(archive.put())) {
					continue;
				}

				auto const entry_table = ctx.create_array();
				ctx.set_array_value(entry_table, 1, archive->getArchivePath());
				ctx.set_array_value(entry_table, 2, 0);

				ctx.set_array_value(result_table, i, entry_table);
				ctx.pop_value();
				++i;
			}

			return 1;
		}

		struct _EnumFilesConfig {
			std::string searchpath;
			std::string searchpath2;
			size_t headlen = 0;
			std::string extpath;
			std::string packname;
			int index = 1;
			bool checkext = false;
			bool checkpack = false;
			bool findfiles = false;
		};
		static _EnumFilesConfig _InitEnumFiles(lua_State* L, bool FindFilesMode = false) {
			lua::stack_t S(L);
			// path ext 

			std::string searchpath(S.get_value<std::string_view>(1));
			utility::path::to_slash(searchpath);
			size_t headlen = 0;
			if (!searchpath.empty() && searchpath.back() != '/') {
				searchpath.push_back('/');
			}
			else if (searchpath.empty()) {
				searchpath.push_back('.');
				headlen = 2;
			}

			std::string searchpath2(S.get_value<std::string_view>(1));
			utility::path::to_slash(searchpath2);
			if (!searchpath2.empty() && searchpath2.back() != '/') {
				searchpath2.push_back('/');
			}
			else if (searchpath2 == "." || searchpath2 == "./") {
				searchpath2 = "";
			}

			std::string extpath = "";
			bool checkext = false;
			if (lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
				std::string_view const argext(S.get_value<std::string_view>(2));
				if (!argext.empty()) {
					extpath.push_back('.');
					extpath.append(argext);
					checkext = true;
				}
			}

			std::string packname = "";
			bool checkpack = false;
			if (FindFilesMode && lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
				std::string_view const argpack(S.get_value<std::string_view>(3));
				if (!argpack.empty()) {
					packname.append(argpack);
					checkpack = true;
				}
			}

			return _EnumFilesConfig{
				.searchpath = std::move(searchpath),
				.searchpath2 = std::move(searchpath2),
				.headlen = headlen,
				.extpath = std::move(extpath),
				.packname = std::move(packname),
				.index = 1,
				.checkext = checkext,
				.checkpack = checkpack,
				.findfiles = FindFilesMode,
			};
		}
		static void _EnumFilesSystem(lua_State* L, _EnumFilesConfig& cfg) {
			lua::stack_t S(L);
			// path ? t 
			std::wstring wextpath(utf8::to_wstring(cfg.extpath));
			std::error_code ec;
			for (auto& p : std::filesystem::directory_iterator(utf8::to_wstring(cfg.searchpath), ec)) {
				bool is_dir = p.is_directory();
				if ((cfg.checkext || cfg.findfiles) && is_dir) {
					continue; // 需要检查拓展名，那就不可能是文件夹了，或者为 FindFiles 模式（忽略文件夹）
				}
				if (p.is_regular_file() || is_dir) {
					if (cfg.checkext && p.path().extension().wstring() != wextpath) {
						continue;
					}
					lua_pushinteger(L, cfg.index);		// path ? t index 
					lua_createtable(L, 2, 0);			// path ? t index tt 
					lua_pushinteger(L, 1);				// path ? t index tt 1 
					std::string u8path(utf8::to_string(p.path().generic_wstring()));
					if (cfg.headlen) u8path = u8path.substr(cfg.headlen);
					if (is_dir) u8path.push_back('/');
					S.push_value(u8path);	// path ? t index tt 1 fpath 
					lua_settable(L, -3);				// path ? t index tt 
					lua_pushinteger(L, 2);				// path ? t index tt 2 
					lua_pushboolean(L, is_dir);			// path ? t index tt 2 bool
					lua_settable(L, -3);				// path ? t index tt 
					lua_settable(L, -3);				// path ? t 
					cfg.index += 1;
				}
			}
		}
		static void _EnumFilesArchive(lua_State* L, _EnumFilesConfig& cfg) {
			lua::stack_t const ctx(L);

			auto const result_table = ctx.index_of_top();

			core::SmartReference<core::IFileSystemFileSystemEnumerator> enumerator;
			if (!core::FileSystemManager::createFileSystemEnumerator(enumerator.put())) {
				return;
			}

			int32_t i{ 1 };
			core::SmartReference<core::IFileSystem> file_system;
			while (enumerator->next(file_system.put())) {
				core::SmartReference<core::IFileSystemArchive> archive;
				if (!file_system->queryInterface(archive.put())) {
					continue; // 必须是压缩包
				}

				if (cfg.checkpack && archive->getArchivePath() != cfg.packname) {
					continue; // 需要匹配压缩包名
				}

				core::SmartReference<core::IFileSystemEnumerator> e;
				if (!archive->createEnumerator(e.put(), cfg.searchpath2)) {
					continue;
				}

				while (e->next()) {
					if (cfg.findfiles && e->getNodeType() != core::FileSystemNodeType::file) {
						continue; // 需要检查拓展名，那就不可能是文件夹了，或者为 FindFiles 模式（仅限文件）
					}
					if (cfg.checkext && !e->getName().ends_with(cfg.extpath)) {
						continue; // 拓展名不匹配
					}

					auto const entry_table = ctx.create_array();
					ctx.set_array_value(entry_table, 1, e->getName());
					if (cfg.findfiles) {
						ctx.set_array_value(entry_table, 2, archive->getArchivePath());
					}
					else {
						ctx.set_array_value(entry_table, 2, e->getNodeType() == core::FileSystemNodeType::directory);
						ctx.set_array_value(entry_table, 3, archive->getArchivePath());
					}

					ctx.set_array_value(result_table, cfg.index, entry_table);
					ctx.pop_value();
					++cfg.index;
				}
			}
		}

		static int EnumFiles(lua_State* L) {
			// path ???

			_EnumFilesConfig cfg = _InitEnumFiles(L);

			// path ??? t 

			lua_newtable(L);

			if (lua_toboolean(L, 3)) {
				_EnumFilesArchive(L, cfg);
			}

			_EnumFilesSystem(L, cfg);

			return 1;
		}
		static int EnumFilesEx(lua_State* L) {
			// path ???

			_EnumFilesConfig cfg = _InitEnumFiles(L);

			// path ??? t 

			lua_newtable(L);

			_EnumFilesArchive(L, cfg);
			_EnumFilesSystem(L, cfg);

			return 1;
		}
		static int FileExist(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);
			auto const all = ctx.get_value<bool>(2);
			if (all) {
				ctx.push_value(core::FileSystemManager::hasFile(path));
			}
			else {
				ctx.push_value(core::FileSystemOS::getInstance()->hasFile(path));
			}
			return 1;
		}
		static int FileExistEx(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);
			ctx.push_value(core::FileSystemManager::hasFile(path));
			return 1;
		}

		static int FindFiles(lua_State* L) {
			// path ???

			_EnumFilesConfig cfg = _InitEnumFiles(L, true);

			// path ??? t 

			lua_newtable(L);

			_EnumFilesArchive(L, cfg);

			if (!cfg.checkpack) {
				_EnumFilesSystem(L, cfg);
			}

			return 1;
		}

		static int AddSearchPath(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);
			core::FileSystemManager::addSearchPath(path);
			return 0;
		}
		static int RemoveSearchPath(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);
			core::FileSystemManager::removeSearchPath(path);
			return 0;
		}
		static int ClearSearchPath(lua_State* L) {
			std::ignore = L;
			core::FileSystemManager::removeAllSearchPath();
			return 0;
		}

		static int SetCurrentDirectory(lua_State* L) {
			lua::stack_t S(L);
			std::string_view const path = S.get_value<std::string_view>(1);
			std::error_code ec;
			std::filesystem::current_path(utf8::to_wstring(path), ec);
			if (ec) {
				lua_pushboolean(L, false);
				S.push_value(ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else {
				lua_pushboolean(L, true);
				return 1;
			}
		}
		static int GetCurrentDirectory(lua_State* L) {
			lua::stack_t S(L);
			std::error_code ec;
			std::filesystem::path path = std::filesystem::current_path(ec);
			if (ec) {
				lua_pushnil(L);
				S.push_value(ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else {
				std::string str = utf8::to_string(path.wstring());
				utility::path::to_slash(str);
				S.push_value(str);
				return 1;
			}
		}
		static int CreateDirectory(lua_State* L) {
			lua::stack_t S(L);
			std::string_view const path = S.get_value<std::string_view>(1);
			std::error_code ec;
			bool result = std::filesystem::create_directories(utf8::to_wstring(path), ec);
			lua_pushboolean(L, result);
			if (ec) {
				S.push_value(ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else {
				return 1;
			}
		}
		static int RemoveDirectory(lua_State* L) {
			lua::stack_t S(L);
			std::string_view const path = S.get_value<std::string_view>(1);
			std::error_code ec;
			uintmax_t result = std::filesystem::remove_all(utf8::to_wstring(path), ec);
			lua_pushboolean(L, result != static_cast<std::uintmax_t>(-1));
			if (ec) {
				S.push_value(ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else {
				return 1;
			}
		}
		static int DirectoryExist(lua_State* L) {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);
			auto const all = ctx.get_value<bool>(2);
			if (path.empty()) {
				ctx.push_value(true); // 相对路径 "" 永远是存在的
				return 1;
			}
			if (all) {
				ctx.push_value(core::FileSystemManager::hasDirectory(path));
			}
			else {
				ctx.push_value(core::FileSystemOS::getInstance()->hasDirectory(path));
			}
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
		{ "EnumFilesEx", &Wrapper::EnumFilesEx }, // 要移除
		{ "FileExist", &Wrapper::FileExist },
		{ "FileExistEx", &Wrapper::FileExistEx }, // 要移除

		{ "AddSearchPath", &Wrapper::AddSearchPath },
		{ "RemoveSearchPath", &Wrapper::RemoveSearchPath },
		{ "ClearSearchPath", &Wrapper::ClearSearchPath },

		{ "SetCurrentDirectory", &Wrapper::SetCurrentDirectory },
		{ "GetCurrentDirectory", &Wrapper::GetCurrentDirectory },
		{ "CreateDirectory", &Wrapper::CreateDirectory },
		{ "RemoveDirectory", &Wrapper::RemoveDirectory },
		{ "DirectoryExist", &Wrapper::DirectoryExist },

		{ NULL, NULL },
	};

	struct FR_Wrapper {
		static int SetFontProvider(lua_State* L) {
			lua_pushboolean(L, LAPP.FontRenderer_SetFontProvider(luaL_checkstring(L, 1)));
			return 1;
		}
		static int SetScale(lua_State* L) {
			float a = (float)luaL_checknumber(L, 1);
			float b = (float)luaL_checknumber(L, 2);
			LAPP.FontRenderer_SetScale(core::Vector2F(a, b));
			return 0;
		}
		static int MeasureTextBoundary(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);
			const core::RectF v = LAPP.FontRenderer_MeasureTextBoundary(str, len);
			lua_pushnumber(L, v.a.x);
			lua_pushnumber(L, v.b.x);
			lua_pushnumber(L, v.b.y);
			lua_pushnumber(L, v.a.y);
			return 4;
		}
		static int MeasureTextAdvance(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);
			const core::Vector2F v = LAPP.FontRenderer_MeasureTextAdvance(str, len);
			lua_pushnumber(L, v.x);
			lua_pushnumber(L, v.y);
			return 2;
		}
		static int RenderText(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);
			auto pos = core::Vector2F((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3));
			const bool ret = LAPP.FontRenderer_RenderText(
				str, len,
				pos, (float)luaL_checknumber(L, 4),
				TranslateBlendMode(L, 5),
				*Color::Cast(L, 6));
			lua_pushboolean(L, ret);
			lua_pushnumber(L, (lua_Number)pos.x);
			lua_pushnumber(L, (lua_Number)pos.y);
			return 3;
		}
		static int RenderTextInSpace(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);

			auto pos = core::Vector3F((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4));
			auto const rvec = core::Vector3F((float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), (float)luaL_checknumber(L, 7));
			auto const dvec = core::Vector3F((float)luaL_checknumber(L, 8), (float)luaL_checknumber(L, 9), (float)luaL_checknumber(L, 10));

			BlendMode blend = TranslateBlendMode(L, 11);
			const bool ret = LAPP.FontRenderer_RenderTextInSpace(
				str, len,
				pos, rvec, dvec,
				blend,
				*Color::Cast(L, 12));
			lua_pushboolean(L, ret);
			lua_pushnumber(L, (lua_Number)pos.x);
			lua_pushnumber(L, (lua_Number)pos.y);
			lua_pushnumber(L, (lua_Number)pos.z);
			return 4;
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

		{ "MeasureTextBoundary", &FR_Wrapper::MeasureTextBoundary },
		{ "MeasureTextAdvance", &FR_Wrapper::MeasureTextAdvance },
		{ "RenderText", &FR_Wrapper::RenderText },
		{ "RenderTextInSpace", &FR_Wrapper::RenderTextInSpace },

		{ "GetFontLineHeight", &FR_Wrapper::GetFontLineHeight },
		{ "GetFontAscender", &FR_Wrapper::GetFontAscender },
		{ "GetFontDescender", &FR_Wrapper::GetFontDescender },

		{ NULL, NULL },
	};

	struct C_Wrapper {
		static int LoadPackSub(lua_State* L)noexcept {
			lua::stack_t const ctx(L);
			auto const path = ctx.get_value<std::string_view>(1);

			core::SmartReference<core::IFileSystemArchive> archive;
			if (!core::IFileSystemArchive::createFromFile(path, archive.put())) {
				spdlog::error("[luastg] 无法加载资源包'{}'，文件不存在或不是合法的资源包格式（zip）", path);
				ctx.push_value(std::nullopt);
				return 1;
			}
			archive->setPassword(GetGameName());

			core::FileSystemManager::addFileSystem(path, archive.get());

			Archive::CreateAndPush(L, archive.get());
			return 1;
		}
		static int ExtractRes(lua_State* L)noexcept {
			const char* pArgPath = luaL_checkstring(L, 1);
			const char* pArgTarget = luaL_checkstring(L, 2);
			if (!extractRes(pArgPath, pArgTarget))
				return luaL_error(L, "failed to extract resource '%s' to '%s'.", pArgPath, pArgTarget);
			return 0;
		}
	};

	luaL_Reg compat_lib[] = {
		{ "LoadPack", &Wrapper::LoadArchive },
		{ "LoadPackSub", &C_Wrapper::LoadPackSub },
		{ "UnloadPack", &Wrapper::UnloadArchive },
		#ifndef USING_ENCRYPTION
		{ "ExtractRes", &C_Wrapper::ExtractRes },
		#endif // !USING_ENCRYPTION
		{ "FindFiles", &Wrapper::FindFiles },
		{ NULL, NULL },
	};

	luaL_register(L, "lstg", compat_lib); // ??? t 

	lua_newtable(L); // ??? t t
	luaL_register(L, NULL, tMethods); // ??? t t 
	lua_setfield(L, -2, "FileManager"); // ??? t 

	lua_newtable(L); // ??? t t
	luaL_register(L, NULL, FR_Method); // ??? t t 
	lua_setfield(L, -2, "FontRenderer"); // ??? t 

	lua_pop(L, 1); // ??? 
}
