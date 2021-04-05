#include "AppFrame.h"
#include "StringFormat.hpp"
#include "ResourcePassword.hpp"
#include "LuaWrapper/LuaAppFrame.hpp"
#include "LuaWrapper/LuaWrapper.hpp"
#include "LuaWrapper/LW_SteamAPI.h"
#include "LuaWrapper/LuaCustomLoader.hpp"
#include "LuaWrapper/LuaStringToEnum.hpp"
#include "LuaWrapper/LuaInternalSource.hpp"

namespace LuaSTGPlus
{
    static int StackTraceback(lua_State *L)
    {
        // errmsg
        int ret = 0;
        
        lua_getfield(L, LUA_GLOBALSINDEX, "debug");								// errmsg t
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);														// errmsg
            return 1;
        }
        
        lua_getfield(L, -1, "traceback");										// errmsg t f
        if (!lua_isfunction(L, -1) && !lua_iscfunction(L, -1))
        {
            lua_pop(L, 2);														// errmsg
            return 1;
        }
        
        lua_pushvalue(L, 1);													// errmsg t f errmsg
        lua_pushinteger(L, 2);													// errmsg t f errmsg 2
        ret = lua_pcall(L, 2, 1, 0);											// errmsg t msg
        if (0 != ret)
        {
            LWARNING("执行stacktrace时发生错误。(%m)", lua_tostring(L, -1));    // errmsg t errmsg
            lua_pop(L, 2);                                                  // errmsg
            return 1;
        }
        
        return 1;
    }
    
    bool AppFrame::SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT
    {
        lua_pushcfunction(L, &StackTraceback);			// ... c
        
        if (0 != luaL_loadbuffer(L, source, len, desc))
        {
            try
            {
                std::wstring tErrorInfo = StringFormat(
                    L"脚本'%m'编译失败: %m",
                    desc,
                    lua_tostring(L, -1)
                );

                LERROR("脚本错误：%s", tErrorInfo.c_str());
                MessageBox(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
                    tErrorInfo.c_str(),
                    L"LuaSTGPlus脚本错误",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                LERROR("尝试写出脚本错误时发生内存不足错误");
            }
            
            lua_pop(L, 2);
            return false;
        }
        // ... c s
        if (0 != lua_pcall(L, 0, 0, lua_gettop(L) - 1))
        {
            try
            {
                std::wstring tErrorInfo = StringFormat(
                    L"脚本'%m'中产生未处理的运行时错误:\n\t%m",
                    desc,
                    lua_tostring(L, -1)
                );
                
                LERROR("脚本错误：%s", tErrorInfo.c_str());
                MessageBox(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
                    tErrorInfo.c_str(),
                    L"LuaSTGPlus脚本错误",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                LERROR("尝试写出脚本错误时发生内存不足错误");
            }
            
            lua_pop(L, 2);
            return false;
        }
        
        lua_pop(L, 1);
        return true;
    }
    
    bool AppFrame::UnsafeCallGlobalFunction(const char* name, int retc)LNOEXCEPT
    {
        lua_getglobal(L, name); // ... f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_call(L, 0, retc);
            return true;
        }
        return false;
    }
    
    bool AppFrame::SafeCallGlobalFunction(const char* name, int retc)LNOEXCEPT
    {
        lua_pushcfunction(L, &StackTraceback);	// ... c
        int tStacktraceIndex = lua_gettop(L);
        
        lua_getglobal(L, name);					// ... c f
        if (0 != lua_pcall(L, 0, retc, tStacktraceIndex))
        {
            try
            {
                std::wstring tErrorInfo = StringFormat(
                    L"执行函数'%m'时产生未处理的运行时错误:\n\t%m",
                    name,
                    lua_tostring(L, -1)
                );
                
                LERROR("脚本错误：%s", tErrorInfo.c_str());
                MessageBox(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
                    tErrorInfo.c_str(),
                    L"LuaSTGPlus脚本错误",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                LERROR("尝试写出脚本错误时发生内存不足错误");
            }
            
            lua_pop(L, 2);
            return false;
        }
        
        lua_remove(L, tStacktraceIndex);
        return true;
    }
    
    bool AppFrame::SafeCallGlobalFunctionB(const char* name, int argc, int retc)LNOEXCEPT
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
                std::wstring tErrorInfo = StringFormat(
                        L"执行函数'%m'时产生未处理的运行时错误:\n\t函数'%m'不存在",
                        name, name);
                LERROR("脚本错误：%s", tErrorInfo.c_str());
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
                LERROR("尝试写出脚本错误时发生内存不足错误");
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
                std::wstring tErrorInfo = StringFormat(
                    L"执行函数'%m'时产生未处理的运行时错误:\n\t%m",
                    name,
                    lua_tostring(L, lua_gettop(L)));
                LERROR("脚本错误：%s", tErrorInfo.c_str());
                MessageBox(
                    m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
                    tErrorInfo.c_str(),
                    L"LuaSTGPlus脚本错误",
                    MB_ICONERROR | MB_OK);
            }
            catch (const std::bad_alloc&)
            {
                LERROR("尝试写出脚本错误时发生内存不足错误");
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
    
    LNOINLINE void AppFrame::LoadScript(const char* path,const char *packname)LNOEXCEPT
    {
        if (ResourceMgr::GetResourceLoadingLog()) {
            LINFO("装载脚本'%m'", path);
        }
        fcyRefPointer<fcyMemStream> tMemStream;
        if (!m_ResourceMgr.LoadFile(path, tMemStream, packname))
        {
            luaL_error(L, "can't load script '%s'", path);
            return;
        }
        if (luaL_loadbuffer(L, (fcStr)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength(), luaL_checkstring(L, 1)))
        {
            tMemStream = nullptr;
            const char* tDetail = lua_tostring(L, -1);
            LERROR("编译脚本'%m'失败: %m", path, tDetail);
            luaL_error(L, "failed to compile '%s': %s", path, tDetail);
            return;
        }
        tMemStream = nullptr;
        lua_call(L, 0, LUA_MULTRET);//保证DoFile后有返回值
    }
    
    bool AppFrame::OnOpenLuaEngine()
    {
        // 加载lua虚拟机
        LINFO("开始初始化Lua虚拟机 版本: %m", LVERSION_LUA);
        L = luaL_newstate();
        if (!L)
        {
            LERROR("无法初始化Lua虚拟机");
            return false;
        }
        if (0 == luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
            LWARNING("无法启动JIT模式");
        lua_gc(L, LUA_GCSTOP, 0);  // 初始化时关闭GC
        {
            luaL_openlibs(L);  // 内建库 (lua build in lib)
            if (!SafeCallScript(LuaInternalSource_1().c_str(), LuaInternalSource_1().length(), "internal")) {
                LERROR("内置资源出错");
            }
            lua_register_custom_loader(L); // 加强版 package 库 (require)
            luaopen_steam(L); // Steam API
            //luaopen_lfs(L);  // 文件系统库 (file system)
            //luaopen_cjson(L);  // CJSON库 (json)
            lua_settop(L, 0);// 不知道为什么弄了6个table在栈顶……
            RegistBuiltInClassWrapper(L);  // 注册内建类 (luastg lib)
            Xrysnow::InitStringToEnumHash(L); // 准备属性hash
        }
        lua_gc(L, LUA_GCRESTART, -1);  // 重启GC
        
        // 设置命令行参数
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
                lua_pushstring(L, v.c_str());	// ? t t s
                lua_rawseti(L, -2, idx + 1);	// ? t t
            }
            lua_setfield(L, -2, "args");		// ? t
            lua_pop(L, 1);						// ?
            ::LocalFree(argv);
        }
        
        return true;
    }
    
    bool AppFrame::OnLoadLaunchScriptAndFiles()
    {
        #ifdef USING_LAUNCH_FILE
            const wchar_t* launch_file_w = L"launch";
            const char* launch_file = "launch";
            fcyRefPointer<fcyMemStream> tMemStream;
            LINFO("加载初始化脚本'%s'", launch_file_w);
            if (m_ResourceMgr.LoadFile(launch_file_w, tMemStream)) {
                if (SafeCallScript((fcStr)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength(), launch_file))
                {
                    LINFO("成功加载初始化脚本'%s'", launch_file_w);
                }
            }
            else {
                LWARNING("找不到文件'%s'", launch_file_w);
            }
        #endif // USING_LAUNCH_FILE
        
        #ifdef USING_ENCRYPTION
            if (!m_FileManager.LoadArchive("data", 0, GetGameName().c_str())) {
                #ifdef LDEVVERSION
                    LWARNING("找不到资源包'data'");
                #endif //LDEVVERSION
            }
        #endif
        
        return true;
    };
    
    bool AppFrame::OnLoadMainScriptAndFiles()
    {
        const wchar_t* entry_file_array_w[] = {
            L"core.lua",
            L"main.lua",
            L"src/main.lua",
        };
        const char* entry_file_array[] = {
            "core.lua",
            "main.lua",
            "src/main.lua",
        };
        const int entry_file_count = sizeof(entry_file_array) / sizeof(char*);
        fcyRefPointer<fcyMemStream> tMemStream;
        
        for (int idx = 0; idx < entry_file_count; idx++)
        {
            const char* entry_file = entry_file_array[idx];
            const wchar_t* entry_file_w = entry_file_array_w[idx];
            #ifdef USING_ENCRYPTION
                if (m_FileManager.ArchiveExist("data")) {
                    auto* zip = m_FileManager.GetArchive("data");
                    if (zip->FileExist(entry_file)) {
                        auto* steam = zip->LoadEncryptedFile(entry_file, GetGameName().c_str());
                        if (steam) {
                            fcyMemStream* mms = (fcyMemStream*)steam;
                            #ifdef LDEVVERSION
                                LINFO("加载入口点脚本'%s'", entry_file_w);
                            #endif // LDEVVERSION
                            if (SafeCallScript((fcStr)mms->GetInternalBuffer(), (size_t)mms->GetLength(), entry_file)) {
                                #ifdef LDEVVERSION
                                    LINFO("成功加载入口点脚本'%s'", entry_file_w);
                                #endif // LDEVVERSION
                                break; // finish
                            }
                            mms = nullptr;
                            steam->Release();
                        }
                    }
                }
            #else // USING_ENCRYPTION
                if (m_ResourceMgr.LoadFile(entry_file_w, tMemStream)) {
                    #ifdef LDEVVERSION
                        LINFO("加载入口点脚本'%s'", entry_file_w);
                    #endif // LDEVVERSION
                    if (SafeCallScript((fcStr)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength(), entry_file)) {
                        #ifdef LDEVVERSION
                            LINFO("成功加载入口点脚本'%s'", entry_file_w);
                        #endif // LDEVVERSION
                        break; // finish
                    }
                }
            #endif // USING_ENCRYPTION
        }
        
        return true;
    }
};
