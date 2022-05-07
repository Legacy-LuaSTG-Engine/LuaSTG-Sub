#include <string>
#include <filesystem>
#include "AppFrame.h"
#include "LuaWrapper\LuaWrapper.hpp"

#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"
#include "utility/path.hpp"

#include "ResourcePassword.hpp"
#include "Config.h"

static bool listFilesS(lua_State* L, const char* dir, const char* ext, int& index) {
	//传入的肯定是utf8格式的搜索目录和拓展名
	// ??? t
	std::string searchpath = dir;//搜索路径
	std::filesystem::path searchdir = std::filesystem::path(utility::encoding::to_wide(searchpath));//路径，需要转换为UTF16
	
	std::string_view extendpath = ext;//拓展名
	size_t extendsize = extendpath.size();//拓展名长度
	size_t pathsize = 0;//文件路径长度
	
	if (std::filesystem::is_directory(searchdir)) {
		for (auto& f : std::filesystem::directory_iterator(searchdir)) {
			if (std::filesystem::is_directory(f.path()) || std::filesystem::is_regular_file(f.path())) {
				std::string path = f.path().string();//文件路径
				pathsize = path.size();
				
				//检查拓展名匹配
				std::string_view compare(&(path[pathsize - extendsize]), extendsize);//要比较的尾部
				if ((extendsize > 0) && ((path[pathsize - extendsize - 1] != '.') || (extendpath != compare))) {
					continue;//拓展名不匹配
				}
				
				lua_pushinteger(L, index);// ??? t index
				lua_createtable(L, 1, 0);// ??? t index t //一个数组元素，没有非数组元素
				lua_pushinteger(L, 1);// ??? t index t 1
				std::string u8path = utility::encoding::to_utf8(f.path().wstring());
				lua_pushstring(L, u8path.c_str());// ??? t index t 1 path
				lua_settable(L, -3);// ??? t index t
				lua_settable(L, -3);// ??? t
				index++;
			}
		}
		return true;
	}
	else {
		return false;
	}
}
static bool listFilesA(lua_State* L, const char* dir, const char* ext, const char* packname, int& index) {
	// ??? t
	
	auto& FMGR = GFileManager();
	
	std::string searchpath = dir;
	utility::path::to_slash(searchpath);//格式化为Linux风格
	if ((searchpath == ".") || (searchpath == "./") || (searchpath == "/")) {
		searchpath = "";//去掉".","./","/"这类路径
	}
	else if ((searchpath.size() > 1) && (searchpath.back() != '/')) {
		searchpath.push_back('/');//补充缺失的斜杠
	}
	std::string_view frompath = searchpath;//搜索路径
	if (frompath.find("..", 0, 2) != std::string_view::npos) {
		return false;//不能使用".."这种路径！
	}
	
	std::string frompack = packname;//要搜索的压缩包路径
	std::string_view expath = ext;//拓展名
	size_t exsize = expath.size();
	size_t pathsize = 0;
	
	for (unsigned int select = 0; select < FMGR.getFileArchiveCount(); select++) {
		auto& zip = FMGR.getFileArchive(select);
		if (!zip.empty()) {
			std::string zipname(zip.getFileArchiveName());
			if ((frompack.size() > 0) && (frompack != zipname)) {
				continue;//没有命中压缩包
			}
			for (size_t pos = 0; pos < zip.getCount(); pos++) {
				std::string filename(zip.getName(pos));//文件名
				pathsize = filename.size();
				
				//开始检查路径是否命中
				if (frompath.size() >= pathsize) {
					continue;//文件名都不够长，跳过
				}
				std::string_view leftpath(&(filename[0]), frompath.size());
				std::string_view rightpath(&(filename[frompath.size()]), pathsize - frompath.size());
				if ((rightpath.find('/', 0) != std::string_view::npos) || (leftpath != frompath)) {
					continue;//不是目标目录的文件，或者前半路径不匹配
				}
				if ((exsize > 0) && (rightpath.size() > (exsize + 1))) {//拓展名长度要大于0且剩余文件名要长过点号+拓展名
					std::string_view compare(&(filename[pathsize - exsize]), exsize);
					if ((filename[pathsize - exsize - 1] != '.') || (expath != compare)) {
						continue;//拓展名没有命中
					}
				}
				
				lua_pushinteger(L, index);// ??? t index
				lua_createtable(L, 2, 0);// ??? t index t //2个数组元素，没有非数组元素
				lua_pushinteger(L, 1);// ??? t index t 1
				lua_pushstring(L, filename.c_str());// ??? t index t 1 path
				lua_settable(L, -3);// ??? t index t
				lua_pushinteger(L, 2);// ??? t index t 2
				lua_pushstring(L, zipname.c_str());// ??? t index t 2 pack
				lua_settable(L, -3);// ??? t index t
				lua_settable(L, -3);// ??? t
				index++;
			}
		}
	}
	
	return true;
}
static bool findFiles(lua_State* L, const char* path, const char* ext, const char* packname) noexcept
{
	// 尝试从各个资源包加载
	lua_newtable(L); // ??? t
	int index = 1;
	
	//搜索压缩包内文件
	listFilesA(L, path, ext, packname, index);
	
	//不限定packname时对文件系统进行查找
	if (std::string_view(packname).size() <= 0) {
		listFilesS(L, path, ext, index);
	}
	
	return true;
}
static bool extractRes(const char* path, const char* target) noexcept
{
	// 读取文件
	fcyRefPointer<fcyMemStream> tBuf;
	if (GFileManager().loadEx(path, ~tBuf)) {
		// 打开本地文件
		fcyRefPointer<fcyFileStream> pFile;
		try {
			pFile.DirectSet(new fcyFileStream(fcyStringHelper::MultiByteToWideChar(target).c_str(), true));
			if (FCYFAILED(pFile->SetLength(0))) {
				spdlog::error("[luastg] ExtractRes: 无法清空文件'{}' (fcyFileStream::SetLength 失败)", target);
				return false;
			}
			if (tBuf->GetLength() > 0) {
				if (FCYFAILED(pFile->WriteBytes((fcData) tBuf->GetInternalBuffer(), tBuf->GetLength(), nullptr))) {
					spdlog::error("[luastg] ExtractRes: 无法向文件'{}'写出数据", target);
					return false;
				}
			}
		}
		catch (const fcyException& e) {
			spdlog::error("[luastg] ExtractRes: 打开本地文件'{}'失败 (异常信息'{}' 源'{}')", target, e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const std::bad_alloc&) {
			spdlog::error("[luastg] ExtractRes: 内存不足");
			return false;
		}
	}
	return true;
}

inline std::string_view luaL_to_string_view(lua_State* L, int idx)
{
	size_t len = 0;
	char const* str = luaL_checklstring(L, idx, &len);
	return std::string_view(str, len);
}
inline void lua_push_string_view(lua_State* L, std::string_view const& str)
{
	lua_pushlstring(L, str.data(), str.length());
}

void LuaSTGPlus::FileManagerWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct Wrapper
	{
		static int LoadArchive(lua_State* L)
		{
			// path ???
			std::string_view const path = luaL_to_string_view(L, 1);
			int const argc = lua_gettop(L);
			bool ret = false;
			if (argc >= 3)
			{
				// path lv pw
				std::string_view const pw = luaL_to_string_view(L, 3);
				ret = GFileManager().loadFileArchive(path, pw);
			}
			else if (argc == 2)
			{
				if (lua_isnumber(L, 2))
				{
					// path lv
					ret = GFileManager().loadFileArchive(path);
				}
				else
				{
					// path pw
					std::string_view const pw = luaL_to_string_view(L, 2);
					ret = GFileManager().loadFileArchive(path, pw);
				}
			}
			else if (argc == 1)
			{
				ret = GFileManager().loadFileArchive(path);
			}
			if (ret)
			{
				auto& zip = GFileManager().getFileArchive(path);
				if (!zip.empty())
				{
					ArchiveWrapper::CreateAndPush(L, zip.getUUID());
				}
				else
				{
					lua_pushnil(L);
				}
			}
			else
			{
				lua_pushnil(L);
			}
			return 1;
		}
		static int UnloadArchive(lua_State* L)
		{
			std::string_view const name = luaL_to_string_view(L, 1);
			if (GFileManager().containFileArchive(name))
			{
				GFileManager().unloadFileArchive(name);
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushboolean(L, false);
			}
			return 1;
		}
		static int UnloadAllArchive(lua_State* L)
		{
			std::ignore = L;
			GFileManager().unloadAllFileArchive();
			return 0;
		}
		static int ArchiveExist(lua_State* L)
		{
			std::string_view const name = luaL_to_string_view(L, 1);
			lua_pushboolean(L, GFileManager().containFileArchive(name));
			return 1;
		}
		static int GetArchive(lua_State* L)
		{
			std::string_view const name = luaL_to_string_view(L, 1);
			auto& zip = GFileManager().getFileArchive(name);
			if (!zip.empty())
			{
				ArchiveWrapper::CreateAndPush(L, zip.getUUID());
			}
			else
			{
				lua_pushnil(L);
			}
			return 1;
		}
		static int EnumArchives(lua_State* L)
		{
			size_t const count = GFileManager().getFileArchiveCount();
			lua_createtable(L, (int)count, 0);							// ??? t 
			for (size_t index = 0; index < count; index += 1)
			{
				auto& ref = GFileManager().getFileArchive(index);
				if (!ref.empty())
				{
					lua_pushinteger(L, (lua_Integer)index + 1);			// ??? t index 
					lua_createtable(L, 2, 0);							// ??? t index tt 
					lua_pushinteger(L, 1);								// ??? t index tt 1 
					lua_push_string_view(L, ref.getFileArchiveName());	// ??? t index tt 1 s
					lua_settable(L, -3);								// ??? t index tt 
					lua_pushinteger(L, 2);								// ??? t index tt 2 
					lua_pushinteger(L, 0);								// ??? t index tt 2 priority 
					lua_settable(L, -3);								// ??? t index tt 
					lua_settable(L, -3);								// ??? t 
				}
			}
			return 1;
		}

		static int EnumFiles(lua_State* L) {
			// path ext 
			std::string searchpath = luaL_checkstring(L, 1);
			std::string extpath = "";
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
			std::filesystem::path path = searchpath;
			unsigned int index = 1;
			if (std::filesystem::is_directory(path)) {
				for (auto& p : std::filesystem::directory_iterator(path)) {
					if (std::filesystem::is_regular_file(p.path()) || std::filesystem::is_directory(p.path())) {
						if (checkext && ((p.path().extension().string() != extpath) || std::filesystem::is_directory(p.path()))) {
							continue;
						}
						lua_pushinteger(L, index); // ??? path t index 
						lua_newtable(L); // ??? path t index tt 
						lua_pushinteger(L, 1); // ??? path t index tt 1 
						std::string u8path = utility::encoding::to_utf8(p.path().wstring());
						if (std::filesystem::is_directory(p.path())) {
							u8path.push_back('/');//在目录路径后面手动添加一个分隔符
						}
						lua_pushstring(L, u8path.c_str()); // ??? path t index tt 1 fpath 
						lua_settable(L, -3); // ??? path t index tt 
						lua_pushinteger(L, 2); // ??? path t index tt 2 
						lua_pushboolean(L, std::filesystem::is_directory(p.path())); // ??? path t index tt 2 bool //为目录时该项为真
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
			std::string searchpath = luaL_checkstring(L, 1);
			std::string extpath = "";
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
			std::filesystem::path path = searchpath;
			unsigned int index = 1;
			if (std::filesystem::is_directory(path)) {
				for (auto& p : std::filesystem::directory_iterator(path)) {
					if (std::filesystem::is_regular_file(p.path()) || std::filesystem::is_directory(p.path())) {
						std::string _s = p.path().extension().string();
						if (checkext && ((p.path().extension().string() != extpath) || std::filesystem::is_directory(p.path()))) {
							continue;
						}
						lua_pushinteger(L, index); // ??? path t index 
						lua_newtable(L); // ??? path t index tt 
						lua_pushinteger(L, 1); // ??? path t index tt 1 
						std::string u8path = utility::encoding::to_utf8(p.path().wstring());
						if (std::filesystem::is_directory(p.path())) {
							u8path.push_back('/');//在目录路径后面手动添加一个分隔符
						}
						lua_pushstring(L, u8path.c_str()); // ??? path t index tt 1 fpath 
						lua_settable(L, -3); // ??? path t index tt 
						lua_pushinteger(L, 2); // ??? path t index tt 2 
						lua_pushboolean(L, std::filesystem::is_directory(p.path())); // ??? path t index tt 2 bool //为目录时该项为真
						lua_settable(L, -3); // ??? path t index tt 
						lua_settable(L, -3); // ??? path t 
						index++;
					}
				}
			}
			for (size_t z = 0; z < GFileManager().getFileArchiveCount(); z++) {
				auto& zip = GFileManager().getFileArchive(z);
				if (!zip.empty()) {
					std::string_view frompath = searchpath; //目标路径
					for (size_t f = 0; f < zip.getCount(); f++) {
						std::string topath(zip.getName(f)); //要比较的路径
						if (frompath.size() >= topath.size()) {
							continue; // 短的直接pass
						}
						else {
							std::string_view path_head(&topath[0], frompath.size()); //前导部分
							if (path_head == frompath) {
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
									std::filesystem::path checkpath(topath);
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
		static int FileExist(lua_State* L)
		{
			std::string_view const path = luaL_to_string_view(L, 1);
			bool const respack = lua_toboolean(L, 2);
			if (respack)
				lua_pushboolean(L, GFileManager().containEx(path));
			else
				lua_pushboolean(L, GFileManager().contain(path));
			return 1;
		}
		static int FileExistEx(lua_State* L)
		{
			std::string_view const path = luaL_to_string_view(L, 1);
			lua_pushboolean(L, GFileManager().containEx(path));
			return 1;
		}
		
		static int AddSearchPath(lua_State* L) {
			std::string_view const path = luaL_to_string_view(L, 1);
			GFileManager().addSearchPath(path);
			return 0;
		}
		static int RemoveSearchPath(lua_State* L) {
			std::string_view const path = luaL_to_string_view(L, 1);
			GFileManager().removeSearchPath(path);
			return 0;
		}
		static int ClearSearchPath(lua_State* L) {
			std::ignore = L;
			GFileManager().clearSearchPath();
			return 1;
		}

		static int SetCurrentDirectory(lua_State* L)
		{
			std::string_view const path = luaL_to_string_view(L, 1);
			std::error_code ec;
			std::filesystem::current_path(utility::encoding::to_wide(path), ec);
			if (ec)
			{
				lua_pushboolean(L, false);
				lua_push_string_view(L, ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else
			{
				lua_pushboolean(L, true);
				return 1;
			}
		}
		static int GetCurrentDirectory(lua_State* L)
		{
			std::error_code ec;
			std::filesystem::path path = std::filesystem::current_path(ec);
			if (ec)
			{
				lua_pushnil(L);
				lua_push_string_view(L, ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else
			{
				std::string str = utility::encoding::to_utf8(path.wstring());
				utility::path::to_slash(str);
				lua_push_string_view(L, str);
				return 1;
			}
		}
		static int CreateDirectory(lua_State* L)
		{
			std::string_view const path = luaL_to_string_view(L, 1);
			std::error_code ec;
			bool result = std::filesystem::create_directories(utility::encoding::to_wide(path), ec);
			lua_pushboolean(L, result);
			if (ec)
			{
				lua_push_string_view(L, ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else
			{
				return 1;
			}
		}
		static int RemoveDirectory(lua_State* L)
		{
			std::string_view const path = luaL_to_string_view(L, 1);
			std::error_code ec;
			uintmax_t result = std::filesystem::remove_all(utility::encoding::to_wide(path), ec);
			lua_pushboolean(L, result != static_cast<std::uintmax_t>(-1));
			if (ec)
			{
				lua_push_string_view(L, ec.message());
				lua_pushinteger(L, ec.value());
				return 3;
			}
			else
			{
				return 1;
			}
		}
		static int DirectoryExist(lua_State* L)
		{
			std::string_view const path = luaL_to_string_view(L, 1);
			if (path.empty())
			{
				lua_pushboolean(L, true); // 相对路径 "" 永远是存在的
				return 1;
			}
			bool const respack = lua_toboolean(L, 2);
			if (GFileManager().getType(path) == LuaSTG::Core::FileType::Directory)
			{
				lua_pushboolean(L, true);
				return 1;
			}
			if (respack)
			{
				auto hasDir = [](std::string_view const& p) -> bool
				{
					for (size_t idx = 0; idx < GFileManager().getFileArchiveCount(); idx += 1)
					{
						auto& zip = GFileManager().getFileArchive(idx);
						if (zip.getType(p) == LuaSTG::Core::FileType::Directory)
						{
							return true;
						}
					}
					return false;
				};
				bool has_dir = false;
				if (path.back() != '/' || path.back() != '\\')
				{
					std::string path_dir(path);
					path_dir.push_back('/');
					has_dir = hasDir(path_dir);
				}
				else
				{
					has_dir = hasDir(path);
				}
				lua_pushboolean(L, has_dir);
				return 1;
			}
			lua_pushboolean(L, false);
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
		static int RenderTextInSpace(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 1, &len);

			fcyVec3 pos = fcyVec3((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4));
			fcyVec3 rvec = fcyVec3((float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), (float)luaL_checknumber(L, 7));
			fcyVec3 dvec = fcyVec3((float)luaL_checknumber(L, 8), (float)luaL_checknumber(L, 9), (float)luaL_checknumber(L, 10));

			BlendMode blend = TranslateBlendMode(L, 11);
			fcyColor color = *static_cast<fcyColor*>(luaL_checkudata(L, 12, LUASTG_LUA_TYPENAME_COLOR));
			const bool ret = LAPP.FontRenderer_RenderTextInSpace(
				str, len,
				pos, rvec, dvec,
				blend,
				color);
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
		
		{ "MeasureString", &FR_Wrapper::MeasureString }, // 应该弃用
		{ "MeasureStringWidth", &FR_Wrapper::MeasureStringWidth }, // 应该弃用
		{ "DrawText", &FR_Wrapper::DrawText }, // 应该弃用
		
		{ "MeasureTextBoundary", &FR_Wrapper::MeasureTextBoundary },
		{ "MeasureTextAdvance", &FR_Wrapper::MeasureTextAdvance },
		{ "RenderText", &FR_Wrapper::RenderText },
		{ "RenderTextInSpace", &FR_Wrapper::RenderTextInSpace },
		
		{ "GetFontLineHeight", &FR_Wrapper::GetFontLineHeight },
		{ "GetFontAscender", &FR_Wrapper::GetFontAscender },
		{ "GetFontDescender", &FR_Wrapper::GetFontDescender },
		
		{ NULL, NULL },
	};
	
	struct C_Wrapper
	{
		static int LoadPack(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			if (lua_isstring(L, 2))
			{
				const char* pwd = luaL_checkstring(L, 2);
				if (!GFileManager().loadFileArchive(p, pwd))
				{
					spdlog::error("[luastg] LoadPack: 无法装载资源包'{}'，文件不存在或不是合法的资源包格式", p);
					lua_pushboolean(L, false);
					return 1;
				}
			}
			else
			{
				if (!GFileManager().loadFileArchive(p))
				{
					spdlog::error("[luastg] LoadPack: 无法装载资源包'{}'，文件不存在或不是合法的资源包格式", p);
					lua_pushboolean(L, false);
					return 1;
				}
			}
			lua_pushboolean(L, true);
			return 1;
		}
		static int LoadPackSub(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			if (!GFileManager().loadFileArchive(p, LuaSTGPlus::GetGameName()))
			{
				spdlog::error("[luastg] LoadPackSub: 无法装载资源包'{}'，文件不存在或不是合法的资源包格式", p);
				lua_pushboolean(L, false);
				return 1;
			}
			lua_pushboolean(L, true);
			return 1;
		}
		static int UnloadPack(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			GFileManager().unloadFileArchive(p);
			return 0;
		}
		static int ExtractRes(lua_State* L)LNOEXCEPT
		{
			const char* pArgPath = luaL_checkstring(L, 1);
			const char* pArgTarget = luaL_checkstring(L, 2);
			if (!extractRes(pArgPath, pArgTarget))
				return luaL_error(L, "failed to extract resource '%s' to '%s'.", pArgPath, pArgTarget);
			return 0;
		}
		static int FindFiles(lua_State* L)LNOEXCEPT
		{
			// searchpath extendname packname
			findFiles(L, luaL_checkstring(L, 1), luaL_optstring(L, 2, ""), luaL_optstring(L, 3, ""));
			return 1;
		}
	};
	
	luaL_Reg compat_lib[] = {
		{ "LoadPack", &C_Wrapper::LoadPack },
		{ "LoadPackSub", &C_Wrapper::LoadPackSub },
		{ "UnloadPack", &C_Wrapper::UnloadPack },
		#ifndef USING_ENCRYPTION
		{ "ExtractRes", &C_Wrapper::ExtractRes },
		#endif // !USING_ENCRYPTION
		{ "FindFiles", &C_Wrapper::FindFiles },
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
