#include "AppFrame.h"
#include "StringFormat.hpp"
#include "ResourcePassword.hpp"
#include "LuaWrapper/LuaAppFrame.hpp"
#include "LuaWrapper/LuaCustomLoader.hpp"
#include "LuaWrapper/LuaInternalSource.hpp"
#include "LuaWrapper/LuaWrapper.hpp"
#include "lua_steam.h"

namespace LuaSTGPlus
{
    static int StackTraceback(lua_State *L) noexcept
    {
        // errmsg
        int ret = 0;
        
        lua_getfield(L, LUA_GLOBALSINDEX, "debug");            // ??? errmsg t
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);                                     // ??? errmsg
            return 1;
        }
        lua_getfield(L, -1, "traceback");                      // ??? errmsg t f
        if (!lua_isfunction(L, -1) && !lua_iscfunction(L, -1))
        {
            lua_pop(L, 2);                                     // ??? errmsg
            return 1;
        }
        
        lua_pushvalue(L, 1);         // ??? errmsg t f errmsg
        lua_pushinteger(L, 2);       // ??? errmsg t f errmsg 2
        ret = lua_pcall(L, 2, 1, 0); // ??? errmsg t msg
        if (0 != ret)
        {
            spdlog::error(u8"[luajit] StackTraceback时发生错误：{}", lua_tostring(L, -1)); // ??? errmsg t errmsg
            lua_pop(L, 2);                                                               // ??? errmsg
            return 1;
        }
        
        return 1;
    }
    
    bool AppFrame::SafeCallScript(const char* source, size_t len, const char* desc) noexcept
    {
        lua_pushcfunction(L, &StackTraceback);          // ??? f
        if (0 != luaL_loadbuffer(L, source, len, desc)) // ??? f f/s
        {
            try
            {
                spdlog::error(u8"[luajit] 编译'{}'失败：{}", desc, lua_tostring(L, -1));
                std::wstring tErrorInfo = StringFormat(
                    L"编译'%m'失败：%m",
                    desc,
                    lua_tostring(L, -1)
                );
                MessageBoxW(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : NULL,
                    tErrorInfo.c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error(u8"[luastg] 记录日志时出错");
            }
            lua_pop(L, 2);
            return false;
        }
        if (0 != lua_pcall(L, 0, 0, lua_gettop(L) - 1)) // ??? f _/e
        {
            try
            {
                spdlog::error(u8"[luajit] 运行'{}'时出错：{}", desc, lua_tostring(L, -1));
                std::wstring tErrorInfo = StringFormat(
                    L"运行'%m'时出错：\n\t%m",
                    desc,
                    lua_tostring(L, -1)
                );
                MessageBoxW(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : NULL,
                    tErrorInfo.c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error(u8"[luastg] 记录日志时出错");
            }
            lua_pop(L, 2);
            return false;
        }
        lua_pop(L, 1);
        return true;
    }
    
    bool AppFrame::UnsafeCallGlobalFunction(const char* name, int retc) noexcept
    {
        lua_getglobal(L, name); // ... f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_call(L, 0, retc);
            return true;
        }
        return false;
    }
    
    bool AppFrame::SafeCallGlobalFunction(const char* name, int retc) noexcept
    {
        lua_pushcfunction(L, &StackTraceback); // ... f
        int tStacktraceIndex = lua_gettop(L);
        lua_getglobal(L, name);                // ... f f
        if (0 != lua_pcall(L, 0, retc, tStacktraceIndex))
        {
            try
            {
                spdlog::error(u8"[luajit] 调用全局函数'{}'时出错：{}", name, lua_tostring(L, -1));
                std::wstring tErrorInfo = StringFormat(
                    L"调用全局函数'%m'时出错：\n\t%m",
                    name,
                    lua_tostring(L, -1)
                );
                MessageBoxW(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : NULL,
                    tErrorInfo.c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error(u8"[luastg] 记录日志时出错");
            }
            lua_pop(L, 2);
            return false;
        }
        lua_remove(L, tStacktraceIndex);
        return true;
    }
    
    bool AppFrame::SafeCallGlobalFunctionB(const char* name, int argc, int retc) noexcept
    {
        const int base_stack = lua_gettop(L) - argc;
        //																// ? ...
        lua_pushcfunction(L, &StackTraceback);							// ? ... trace
        lua_getglobal(L, name);											// ? ... trace func
        if (lua_type(L, lua_gettop(L)) != LUA_TFUNCTION)
        {
            //															// ? ... trace nil
            try
            {
                spdlog::error(u8"[luajit] 调用全局函数'{}'时出错：全局函数'{}'不存在", name, name);
                /*
                MessageBox(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : NULL,
                    tErrorInfo.c_str(),
                    L"LuaSTGPlus脚本错误",
                    MB_ICONERROR | MB_OK);
                //*/
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error(u8"[luastg] 记录日志时出错");
            }
            lua_pop(L, argc + 2); 										// ?
            return false;
        }
        if (argc > 0)
        {
            lua_insert(L, base_stack + 1);								// ? func ... trace
            lua_insert(L, base_stack + 1);								// ? trace func ...
        }
        if (0 != lua_pcall(L, argc, retc, base_stack + 1))
        {
            //															// ? trace errmsg
            try
            {
                spdlog::error(u8"[luajit] 调用全局函数'{}'时出错：{}", name, lua_tostring(L, -1));
                std::wstring tErrorInfo = StringFormat(
                    L"调用全局函数'%m'时出错：\n\t%m",
                    name,
                    lua_tostring(L, -1)
                );
                MessageBoxW(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : NULL,
                    tErrorInfo.c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK);
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error(u8"[luastg] 记录日志时出错");
            }
            lua_pop(L, 2);												// ?
            return false;
        }
        else
        {
            if (retc > 0)
            {
                //														// ? trace ...
                lua_remove(L, base_stack + 1);							// ? ...
            }
            else
            {
                //														// ? trace
                lua_pop(L, 1);											// ?
            }
            return true;
        }
    }
    
    void AppFrame::LoadScript(const char* path, const char* packname) noexcept
    {
        if (ResourceMgr::GetResourceLoadingLog())
        {
            if (packname)
                spdlog::info(u8"[luastg] 在资源包'{}'中加载脚本'{}'", packname, path);
            else
                spdlog::info(u8"[luastg] 加载脚本'{}'", path);
        }
        fcyRefPointer<fcyMemStream> tMemStream;
        if (!m_ResourceMgr.LoadFile(path, tMemStream, packname))
        {
            spdlog::error(u8"[luastg] 无法加载文件'{}'", path);
            luaL_error(L, "can't load file '%s'", path);
            return;
        }
        if (0 != luaL_loadbuffer(L, (fcStr)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength(), luaL_checkstring(L, 1)))
        {
            const char* tDetail = lua_tostring(L, -1);
            spdlog::error(u8"[luajit] 编译'{}'失败：{}", path, tDetail);
            luaL_error(L, "failed to compile '%s': %s", path, tDetail);
            return;
        }
        lua_call(L, 0, LUA_MULTRET);//这个一般只会在lua代码调用，外层已经有pcall了
    }
    
    bool AppFrame::OnOpenLuaEngine()
    {
        // 加载lua虚拟机
        spdlog::info(u8"[luajit] {}", LUAJIT_VERSION);
        L = luaL_newstate();
        if (!L)
        {
            spdlog::error(u8"[luajit] 无法创建luajit引擎");
            return false;
        }
        if (0 == luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
        {
            spdlog::error(u8"[luajit] 无法启动jit模式");
        }
        lua_gc(L, LUA_GCSTOP, 0);  // 初始化时关闭GC
        {
            spdlog::info(u8"[luajit] 注册标准库与内置包");
            luaL_openlibs(L);  // 内建库 (lua build in lib)
            lua_register_custom_loader(L); // 加强版 package 库 (require)
            
            if (!SafeCallScript(LuaInternalSource_1().c_str(), LuaInternalSource_1().length(), "internal.main")) {
                spdlog::error(u8"[luajit] 内置脚本'internal.main'出错");
                return false;
            }
            
            //luaopen_lfs(L);  // 文件系统库 (file system)
            //luaopen_cjson(L);  // CJSON库 (json)
            lua_steam_open(L); // Steam API
            RegistBuiltInClassWrapper(L);  // 注册内建类 (luastg lib)
            lua_settop(L, 0);
            
            // 设置命令行参数
            spdlog::info(u8"[luajit] 储存命令行参数");
            const WCHAR* cmd = ::GetCommandLineW();
            int argc = 0;
            WCHAR** argv = ::CommandLineToArgvW(cmd, &argc);
            if (argv != NULL)
            {
                lua_getglobal(L, "lstg");			// ? t
                lua_createtable(L, argc, 0);		// ? t t
                for (int idx = 0; idx < argc; idx++)
                {
                    std::string v = fcyStringHelper::WideCharToMultiByte(argv[idx], CP_UTF8);
                    spdlog::info(u8"[luajit] [{}] {}", idx + 1, v);
                    lua_pushstring(L, v.c_str());	// ? t t s
                    lua_rawseti(L, -2, idx + 1);	// ? t t
                }
                lua_setfield(L, -2, "args");		// ? t
                lua_pop(L, 1);						// ?
                ::LocalFree(argv);
            }
            
            if (!SafeCallScript(LuaInternalSource_2().c_str(), LuaInternalSource_2().length(), "internal.api")) {
                spdlog::error(u8"[luajit] 内置脚本'internal.api'出错");
                return false;
            }
        }
        lua_gc(L, LUA_GCRESTART, -1);  // 重启GC
        
        return true;
    }
    
    bool AppFrame::OnLoadLaunchScriptAndFiles()
    {
        #ifdef USING_ENCRYPTION
        spdlog::info(u8"[luastg] 加载资源包");
        if (!m_FileManager.LoadArchive("data", 0, GetGameName().c_str()))
        {
            if (!m_FileManager.LoadArchive("data.zip", 0, GetGameName().c_str()))
            {
                spdlog::error(u8"[luastg] 找不到文件'data'或'data.zip'");
            }
        }
        #endif
        
        #ifdef USING_LAUNCH_FILE
        fcyRefPointer<fcyMemStream> tMemStream;
        spdlog::info(u8"[luastg] 加载初始化脚本");
        if (m_ResourceMgr.LoadFile(u8"launch", tMemStream))
        {
            if (SafeCallScript((fcStr)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength(), "launch"))
            {
                spdlog::info(u8"[luastg] 加载脚本'launch'");
            }
            else
            {
                spdlog::error(u8"[luastg] 加载初始化脚本'launch'失败");
            }
        }
        else
        {
            spdlog::error(u8"[luastg] 找不到文件'launch'");
        }
        #endif
        
        return true;
    };
    
    bool AppFrame::OnLoadMainScriptAndFiles()
    {
        spdlog::info(u8"[luastg] 加载入口点脚本");
        std::string entry_scripts[3] = {
            u8"core.lua",
            u8"main.lua",
            u8"src/main.lua",
        };
        fcyRefPointer<fcyMemStream> source;
        bool is_load = false;
        #ifndef USING_ENCRYPTION
        for (auto& v : entry_scripts)
        {
            if (m_ResourceMgr.LoadFile(v.c_str(), source))
            {
                if (SafeCallScript((fcStr)source->GetInternalBuffer(), (size_t)source->GetLength(), v.c_str()))
                {
                    spdlog::info(u8"[luastg] 加载脚本'{}'", v.c_str());
                    is_load = true;
                    break;
                }
            }
        }
        #else
        auto* zip = m_FileManager.GetArchive("data");
        if (zip == nullptr)
        {
            auto* zip = m_FileManager.GetArchive("data.zip");
            if (zip == nullptr)
            {
                spdlog::error(u8"[luastg] 资源包'data'或'data.zip'不存在");
                return false;
            }
        }
        for (auto& v : entry_scripts)
        {
            if (zip->FileExist(v.c_str()))
            {
                auto* steam = zip->LoadEncryptedFile(v.c_str(), GetGameName().c_str());
                if (steam)
                {
                    fcyMemStream* source = (fcyMemStream*)steam;
                    if (SafeCallScript((fcStr)source->GetInternalBuffer(), (size_t)source->GetLength(), v.c_str()))
                    {
                        spdlog::info(u8"[luastg] 加载脚本'{}'", v.c_str());
                        is_load = true;
                        break;
                    }
                    steam->Release();
                }
            }
        }
        #endif
        if (!is_load)
        {
            spdlog::error(u8"[luastg] 找不到文件'{}'、'{}'或'{}'", entry_scripts[0], entry_scripts[1], entry_scripts[2]);
        }
        return true;
    }
};
