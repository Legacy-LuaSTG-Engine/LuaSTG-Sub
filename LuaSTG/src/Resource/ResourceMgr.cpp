#include "ResourceMgr.h"

#include <string>
#include <string_view>
#include <filesystem>

#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"
#include "utility/path.hpp"

#include "AppFrame.h"

using namespace std;
using namespace LuaSTGPlus;

ResourceMgr::ResourceMgr() :
    m_GlobalResourcePool(this, ResourcePoolType::Global),
    m_StageResourcePool(this, ResourcePoolType::Stage) {
}

// 文件操作

bool ResourceMgr::ExtractRes(const char* path, const char* target) noexcept {
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
        catch (const bad_alloc&) {
            spdlog::error("[luastg] ExtractRes: 内存不足");
            return false;
        }
    }
    return true;
}

bool listFilesS(lua_State* L, const char* dir, const char* ext, int& index) {
    //传入的肯定是utf8格式的搜索目录和拓展名
    // ??? t
    string searchpath = dir;//搜索路径
    filesystem::path searchdir = filesystem::path(utility::encoding::to_wide(searchpath));//路径，需要转换为UTF16
    
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
                string u8path = utility::encoding::to_utf8(f.path().wstring());
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
    
    auto& FMGR = GFileManager();
    
    string searchpath = dir;
    utility::path::to_slash(searchpath);//格式化为Linux风格
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
    
    for (unsigned int select = 0; select < FMGR.getFileArchiveCount(); select++) {
        auto& zip = FMGR.getFileArchive(select);
        if (!zip.empty()) {
            string zipname(zip.getFileArchiveName());
            if ((frompack.size() > 0) && (frompack != zipname)) {
                continue;//没有命中压缩包
            }
            for (long long pos = 0; pos < zip.getCount(); pos++) {
                string filename(zip.getName(pos));//文件名
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

bool ResourceMgr::FindFiles(lua_State* L, const char* path, const char* ext, const char* packname) noexcept {
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

// 资源池管理

void ResourceMgr::ClearAllResource() noexcept {
    m_GlobalResourcePool.Clear();
    m_StageResourcePool.Clear();
    m_ActivedPool = ResourcePoolType::Global;
    m_GlobalImageScaleFactor = 1.0f;
}

ResourcePoolType ResourceMgr::GetActivedPoolType() noexcept {
    return m_ActivedPool;
}

void ResourceMgr::SetActivedPoolType(ResourcePoolType t) noexcept {
    m_ActivedPool = t;
}

ResourcePool* ResourceMgr::GetActivedPool() noexcept {
    return GetResourcePool(m_ActivedPool);
}

ResourcePool* ResourceMgr::GetResourcePool(ResourcePoolType t) noexcept {
    switch (t) {
        case ResourcePoolType::Global:
            return &m_GlobalResourcePool;
        case ResourcePoolType::Stage:
            return &m_StageResourcePool;
        default:
            return nullptr;
    }
}

// 自动查找资源池资源

fcyRefPointer<ResTexture> ResourceMgr::FindTexture(const char* name) noexcept {
    fcyRefPointer<ResTexture> tRet;
    if (!(tRet = m_StageResourcePool.GetTexture(name)))
        tRet = m_GlobalResourcePool.GetTexture(name);
    return tRet;
}

fcyRefPointer<ResSprite> ResourceMgr::FindSprite(const char* name) noexcept {
    fcyRefPointer<ResSprite> tRet;
    if (!(tRet = m_StageResourcePool.GetSprite(name)))
        tRet = m_GlobalResourcePool.GetSprite(name);
    return tRet;
}

fcyRefPointer<ResAnimation> ResourceMgr::FindAnimation(const char* name) noexcept {
    fcyRefPointer<ResAnimation> tRet;
    if (!(tRet = m_StageResourcePool.GetAnimation(name)))
        tRet = m_GlobalResourcePool.GetAnimation(name);
    return tRet;
}

fcyRefPointer<ResMusic> ResourceMgr::FindMusic(const char* name) noexcept {
    fcyRefPointer<ResMusic> tRet;
    if (!(tRet = m_StageResourcePool.GetMusic(name)))
        tRet = m_GlobalResourcePool.GetMusic(name);
    return tRet;
}

fcyRefPointer<ResSound> ResourceMgr::FindSound(const char* name) noexcept {
    fcyRefPointer<ResSound> tRet;
    if (!(tRet = m_StageResourcePool.GetSound(name)))
        tRet = m_GlobalResourcePool.GetSound(name);
    return tRet;
}

fcyRefPointer<ResParticle> ResourceMgr::FindParticle(const char* name) noexcept {
    fcyRefPointer<ResParticle> tRet;
    if (!(tRet = m_StageResourcePool.GetParticle(name)))
        tRet = m_GlobalResourcePool.GetParticle(name);
    return tRet;
}

fcyRefPointer<ResFont> ResourceMgr::FindSpriteFont(const char* name) noexcept {
    fcyRefPointer<ResFont> tRet;
    if (!(tRet = m_StageResourcePool.GetSpriteFont(name)))
        tRet = m_GlobalResourcePool.GetSpriteFont(name);
    return tRet;
}

fcyRefPointer<ResFont> ResourceMgr::FindTTFFont(const char* name) noexcept {
    fcyRefPointer<ResFont> tRet;
    if (!(tRet = m_StageResourcePool.GetTTFFont(name)))
        tRet = m_GlobalResourcePool.GetTTFFont(name);
    return tRet;
}

fcyRefPointer<ResFX> ResourceMgr::FindFX(const char* name) noexcept {
    fcyRefPointer<ResFX> tRet;
    if (!(tRet = m_StageResourcePool.GetFX(name)))
        tRet = m_GlobalResourcePool.GetFX(name);
    return tRet;
}

// 其他资源操作

bool ResourceMgr::GetTextureSize(const char* name, fcyVec2& out) noexcept {
    fcyRefPointer<ResTexture> tRet = FindTexture(name);
    if (!tRet)
        return false;
    out.x = static_cast<float>(tRet->GetTexture()->GetWidth());
    out.y = static_cast<float>(tRet->GetTexture()->GetHeight());
    return true;
}

void ResourceMgr::CacheTTFFontString(const char* name, const char* text, size_t len) noexcept {
    fcyRefPointer<ResFont> f = FindTTFFont(name);
    if (f)
        f->GetFontProvider()->CacheStringU8(text, len);
    else
        spdlog::error("[luastg] CacheTTFFontString: 缓存字形时未找到指定字体'{}'", name);
}

// 其他

#ifdef LDEVVERSION
bool ResourceMgr::g_ResourceLoadingLog = true;
#else
bool ResourceMgr::g_ResourceLoadingLog = false;
#endif

void ResourceMgr::SetResourceLoadingLog(bool b) { g_ResourceLoadingLog = b; }

bool ResourceMgr::GetResourceLoadingLog() { return g_ResourceLoadingLog; }
