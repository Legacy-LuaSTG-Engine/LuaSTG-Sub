#include <string>
#include <string_view>
#include <filesystem>

#include "ResourceMgr.h"
#include "AppFrame.h"

#include "E2DFileManager.hpp"
#include "E2DCodePage.hpp"
#include "E2DFilePath.hpp"

using namespace std;
using namespace LuaSTGPlus;

ResourceMgr::ResourceMgr()
	: m_GlobalResourcePool(this, ResourcePoolType::Global), m_StageResourcePool(this, ResourcePoolType::Stage)
{
}

void ResourceMgr::ClearAllResource()LNOEXCEPT
{
	m_GlobalResourcePool.Clear();
	m_StageResourcePool.Clear();
	m_ActivedPool = ResourcePoolType::Global;
	m_GlobalImageScaleFactor = 1.;
	m_GlobalSoundEffectVolume = 1.0f;
	m_GlobalMusicVolume = 1.0f;
}

LNOINLINE bool ResourceMgr::LoadFile(const wchar_t* path, fcyRefPointer<fcyMemStream>& outBuf, const wchar_t *packname)LNOEXCEPT
{
	std::string utf8path = Eyes2D::String::UTF16ToUTF8(path);//文件路径
	if (packname != nullptr) {
		std::string utf8pack = Eyes2D::String::UTF16ToUTF8(packname); // 压缩包名
		return LoadFile(utf8path.c_str(), outBuf, utf8pack.c_str());
	}
	else {
		return LoadFile(utf8path.c_str(), outBuf, nullptr);
	}
}

LNOINLINE bool ResourceMgr::LoadFile(const char* path, fcyRefPointer<fcyMemStream>& outBuf, const char *packname)LNOEXCEPT
{
	std::string utf8path = path;//文件路径
	Eyes2D::Platform::PathFormatLinux(utf8path);
	
	Eyes2D::IO::FileManager& FMGR = LFMGR;
	fcyStream* stream = nullptr;
	if (packname != nullptr) {
		stream = FMGR.LoadFile(utf8path.c_str(), packname);
	}
	else {
		stream = FMGR.LoadFile(utf8path.c_str());
	}

	if (stream != nullptr) {
		outBuf.DirectSet((fcyMemStream*)stream);
		return true;
	}
	else {
		return false;
	}
}

bool ResourceMgr::ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT
{
	fcyRefPointer<fcyMemStream> tBuf;

	// 读取文件
	if (LoadFile(path, tBuf))
	{
		// 打开本地文件
		fcyRefPointer<fcyFileStream> pFile;
		try
		{
			pFile.DirectSet(new fcyFileStream(target, true));
			if (FCYFAILED(pFile->SetLength(0)))
			{
				LERROR("ResourceMgr: 无法清空文件'%s' (fcyFileStream::SetLength 失败)", target);
				return false;
			}
			if (tBuf->GetLength() > 0)
			{
				if (FCYFAILED(pFile->WriteBytes((fcData)tBuf->GetInternalBuffer(), tBuf->GetLength(), nullptr)))
				{
					LERROR("ResourceMgr: 无法向文件'%s'写出数据", target);
					return false;
				}
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("ResourceMgr: 无法分配足够内存来向文件'%s'写出数据", target);
			return false;
		}
		catch (const fcyException& e)
		{
			LERROR("ResourceMgr: 打开本地文件'%s'失败 (异常信息'%m' 源'%m')", target, e.GetDesc(), e.GetSrc());
			return false;
		}
	}
	return true;
}

LNOINLINE bool ResourceMgr::ExtractRes(const char* path, const char* target)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		wstring tTarget = fcyStringHelper::MultiByteToWideChar(target, CP_UTF8);
		return ExtractRes(tPath.c_str(), tTarget.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: 转换字符编码时无法分配内存");
	}
	return false;
}

bool listFilesS(lua_State* L, const char* dir, const char* ext, int& index) {
	//传入的肯定是utf8格式的搜索目录和拓展名
		// ??? t 
	string searchpath = dir;//搜索路径
	filesystem::path searchdir = filesystem::path(Eyes2D::String::UTF8ToUTF16(searchpath));//路径，需要转换为UTF16

	string_view extendpath = ext;//拓展名
	size_t extendsize = extendpath.size();//拓展名长度
	size_t pathsize = 0;//文件路径长度

	if (filesystem::is_directory(searchdir)) {
		for (auto& f : filesystem::directory_iterator(searchdir)) {
			if (filesystem::is_directory(f.path()) || filesystem::is_regular_file(f.path())) {
				string path = f.path().string();//文件路径
				pathsize = path.size();

				//检查拓展名匹配
				string_view compare = string_view(&(path[pathsize - extendsize]), extendsize);//要比较的尾部
				if ((extendsize > 0) && ((path[pathsize - extendsize - 1] != '.') || (extendpath != compare))) {
					continue;//拓展名不匹配
				}

				lua_pushinteger(L, index);// ??? t index 
				lua_createtable(L, 1, 0);// ??? t index t //一个数组元素，没有非数组元素
				lua_pushinteger(L, 1);// ??? t index t 1 
				string u8path = Eyes2D::String::UTF16ToUTF8(f.path().wstring());
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

bool listFilesA(lua_State* L, const char* dir, const char* ext, const char* packname, int& index) {
	// ??? t 
	
	Eyes2D::IO::FileManager& FMGR = LFMGR;
	Eyes2D::IO::Archive* zip = nullptr;

	string searchpath = dir;
	Eyes2D::Platform::PathFormatLinux(searchpath);//格式化为Linux风格
	if ((searchpath == ".") || (searchpath == "./") || (searchpath == "/")) {
		searchpath = "";//去掉".","./","/"这类路径
	}
	else if ((searchpath.size() > 1) && (searchpath.back() != '/')) {
		searchpath.push_back('/');//补充缺失的斜杠
	}
	string_view frompath = searchpath;//搜索路径
	if (frompath.find("..", 0, 2) != string_view::npos) {
		return false;//不能使用".."这种路径！
	}

	string frompack = packname;//要搜索的压缩包路径
	string_view expath = ext;//拓展名
	size_t exsize = expath.size();
	size_t pathsize = 0;

	for (unsigned int select = 0; select < FMGR.GetArchiveCount(); select++) {
		zip = FMGR.GetArchive(select);
		if (zip != nullptr) {
			string zipname = zip->GetArchivePath();
			if ((frompack.size() > 0) && (frompack != zipname)) {
				continue;//没有命中压缩包
			}
			for (long long pos = 0; pos < zip->GetFileCount(); pos++) {
				string filename = zip->GetFileName(pos);//文件名
				pathsize = filename.size();

				//开始检查路径是否命中
				if (frompath.size() >= pathsize) {
					continue;//文件名都不够长，跳过
				}
				string_view leftpath = string_view(&(filename[0]), frompath.size());
				string_view rightpath = string_view(&(filename[frompath.size()]), pathsize - frompath.size());
				if ((rightpath.find('/', 0) != string_view::npos) || (leftpath != frompath)) {
					continue;//不是目标目录的文件，或者前半路径不匹配
				}
				if ((exsize > 0) && (rightpath.size() > (exsize + 1))) {//拓展名长度要大于0且剩余文件名要长过点号+拓展名
					string_view compare = string_view(&(filename[pathsize - exsize]), exsize);
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

LNOINLINE bool ResourceMgr::FindFiles(lua_State *L,const char* path,const char *ext, const char *packname)LNOEXCEPT
{
	// 尝试从各个资源包加载
	lua_newtable(L); // ??? t 
	int index = 1;

	//搜索压缩包内文件
	::listFilesA(L, path, ext, packname, index);

	//不限定packname时对文件系统进行查找
	if (string_view(packname).size() <= 0) {
		::listFilesS(L, path, ext, index);
	}

	return true;
}
