#include "AppFrame.h"
#include "GameResource/ResourcePassword.hpp"
#include "LuaBinding/LuaAppFrame.hpp"
#include "LuaBinding/LuaCustomLoader.hpp"
#include "LuaBinding/LuaInternalSource.hpp"
#include "LuaBinding/LuaWrapper.hpp"
extern "C" {
#include "lua_cjson.h"
#include "lfs.h"
extern int luaopen_utf8(lua_State* L);
extern int luaopen_string_pack(lua_State* L);
}
//#include "lua_xlsx_csv.h"
#include "lua_steam.h"
#include "LuaBinding/lua_xinput.hpp"
#include "LuaBinding/lua_random.hpp"

#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"
#include "platform/CommandLine.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

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
            spdlog::error("[luajit] StackTraceback时发生错误：{}", lua_tostring(L, -1)); // ??? errmsg t errmsg
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
                spdlog::error("[luajit] 编译'{}'失败：{}", desc, lua_tostring(L, -1));
                MessageBoxW(
                    (HWND)m_pAppModel->getWindow()->getNativeHandle(),
                    utility::encoding::to_wide(
                        fmt::format("编译'{}'失败：{}", desc, lua_tostring(L, -1))
                    ).c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error("[luastg] 记录日志时出错");
            }
            lua_pop(L, 2);
            return false;
        }
        if (0 != lua_pcall(L, 0, 0, lua_gettop(L) - 1)) // ??? f _/e
        {
            try
            {
                spdlog::error("[luajit] 运行'{}'时出错：{}", desc, lua_tostring(L, -1));
                MessageBoxW(
                    (HWND)m_pAppModel->getWindow()->getNativeHandle(),
                    utility::encoding::to_wide(
                        fmt::format("运行'{}'时出错：\n{}", desc, lua_tostring(L, -1))
                    ).c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error("[luastg] 记录日志时出错");
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
                spdlog::error("[luajit] 调用全局函数'{}'时出错：{}", name, lua_tostring(L, -1));
                MessageBoxW(
                    (HWND)m_pAppModel->getWindow()->getNativeHandle(),
                    utility::encoding::to_wide(
                        fmt::format("调用全局函数'{}'时出错：\n{}", name, lua_tostring(L, -1))
                    ).c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK
                );
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error("[luastg] 记录日志时出错");
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
                spdlog::error("[luajit] 调用全局函数'{}'时出错：全局函数'{}'不存在", name, name);
                /*
                MessageBox(
                    (HWND)m_pAppModel->getWindow()->getNativeHandle(),
                    tErrorInfo.c_str(),
                    L"LuaSTGPlus脚本错误",
                    MB_ICONERROR | MB_OK);
                //*/
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error("[luastg] 记录日志时出错");
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
                spdlog::error("[luajit] 调用全局函数'{}'时出错：{}", name, lua_tostring(L, -1));
                MessageBoxW(
                    (HWND)m_pAppModel->getWindow()->getNativeHandle(),
                    utility::encoding::to_wide(
                        fmt::format("调用全局函数'{}'时出错：\n{}", name, lua_tostring(L, -1))
                    ).c_str(),
                    L"程序异常中止",
                    MB_ICONERROR | MB_OK);
            }
            catch (const std::bad_alloc&)
            {
                spdlog::error("[luastg] 记录日志时出错");
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
                spdlog::info("[luastg] 在资源包'{}'中加载脚本'{}'", packname, path);
            else
                spdlog::info("[luastg] 加载脚本'{}'", path);
        }
        bool loaded = false;
        std::vector<uint8_t> src;
        if (packname)
        {
            auto& arc = GFileManager().getFileArchive(packname);
            if (!arc.empty())
            {
                loaded = arc.load(path, src);
            }
        }
        else
        {
            loaded = GFileManager().loadEx(path, src);
        }
        if (!loaded)
        {
            spdlog::error("[luastg] 无法加载文件'{}'", path);
            luaL_error(L, "can't load file '%s'", path);
            return;
        }
        if (0 != luaL_loadbuffer(L, (char const*)src.data(), (size_t)src.size(), luaL_checkstring(L, 1)))
        {
            const char* tDetail = lua_tostring(L, -1);
            spdlog::error("[luajit] 编译'{}'失败：{}", path, tDetail);
            luaL_error(L, "failed to compile '%s': %s", path, tDetail);
            return;
        }
        lua_call(L, 0, LUA_MULTRET);//这个一般只会在lua代码调用，外层已经有pcall了
    }
    
    bool AppFrame::OnOpenLuaEngine()
    {
        // 加载lua虚拟机
        spdlog::info("[luajit] {}", LUAJIT_VERSION);
        L = luaL_newstate();
        if (!L)
        {
            spdlog::error("[luajit] 无法创建luajit引擎");
            return false;
        }
        if (0 == luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
        {
            spdlog::error("[luajit] 无法启动jit模式");
        }
        lua_gc(L, LUA_GCSTOP, 0);  // 初始化时关闭GC
        {
            spdlog::info("[luajit] 注册标准库与内置包");
            luaL_openlibs(L);  // 内建库 (lua build in lib)
            lua_register_custom_loader(L); // 加强版 package 库 (require)
            
            if (!SafeCallScript(LuaInternalSource_1().c_str(), LuaInternalSource_1().length(), "internal.main")) {
                spdlog::error("[luajit] 内置脚本'internal.main'出错");
                return false;
            }
            
            luaopen_cjson(L);
            luaopen_lfs(L);
            //lua_xlsx_open(L);
            //lua_csv_open(L);
            lua_steam_open(L);
            lua_xinput_open(L);
            luaopen_random(L);
            luaopen_utf8(L);
            luaopen_string_pack(L);
            lua_settop(L, 0);
            
            RegistBuiltInClassWrapper(L);  // 注册内建类 (luastg lib)
            lua_settop(L, 0);
            
            // 设置命令行参数
            spdlog::info("[luajit] 储存命令行参数");
            std::vector<std::string> args(platform::CommandLine::get());
            if (!args.empty())
            {
                // 打印命令行参数，隐藏不需要的命令行参数
                std::vector<std::string_view> args_lua;
                for (size_t idx = 0; idx < args.size(); idx += 1)
                {
                    spdlog::info("[luajit] [{}] {}", idx, args[idx]);
                    if (args[idx] != "--log-window" &&
                        args[idx] != "--log-window-wait")
                    {
                        args_lua.emplace_back(args[idx]);
                    }
                }
                // 储存
                lua_getglobal(L, "lstg");                       // ? t
                lua_createtable(L, (int)args_lua.size(), 0);    // ? t t
                for (int idx = 0; idx < (int)args_lua.size(); idx += 1)
                {
                    lua_pushstring(L, args_lua[idx].data());    // ? t t s
                    lua_rawseti(L, -2, idx + 1);                // ? t t
                }
                lua_setfield(L, -2, "args");                    // ? t
                lua_pop(L, 1);                                  // ?
            }
            
            if (!SafeCallScript(LuaInternalSource_2().c_str(), LuaInternalSource_2().length(), "internal.api")) {
                spdlog::error("[luajit] 内置脚本'internal.api'出错");
                return false;
            }
        }
        lua_gc(L, LUA_GCRESTART, -1);  // 重启GC
        
        return true;
    }
};
