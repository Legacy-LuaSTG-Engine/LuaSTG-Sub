#include "AppFrame.h"
#include "Config.h"
#include "Core/FileManager.hpp"

namespace LuaSTGPlus
{
    bool AppFrame::OnLoadLaunchScriptAndFiles()
    {
        #ifdef USING_LAUNCH_FILE
        fcyRefPointer<fcyMemStream> tMemStream;
        spdlog::info("[luastg] 加载初始化脚本");
        if (GFileManager().loadEx("launch", ~tMemStream))
        {
            if (SafeCallScript((fcStr)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength(), "launch"))
            {
                spdlog::info("[luastg] 加载脚本'launch'");
            }
            else
            {
                spdlog::error("[luastg] 加载初始化脚本'launch'失败");
            }
        }
        else
        {
            spdlog::error("[luastg] 找不到文件'launch'");
        }
        #endif
        
        return true;
    };
    
    bool AppFrame::OnLoadMainScriptAndFiles()
    {
        spdlog::info("[luastg] 加载入口点脚本");
        std::string_view entry_scripts[3] = {
            "core.lua",
            "main.lua",
            "src/main.lua",
        };
        fcyRefPointer<fcyMemStream> source;
        bool is_load = false;
        for (auto& v : entry_scripts)
        {
            if (GFileManager().loadEx(v, ~source))
            {
                if (SafeCallScript((fcStr)source->GetInternalBuffer(), (size_t)source->GetLength(), v.data()))
                {
                    spdlog::info("[luastg] 加载脚本'{}'", v);
                    is_load = true;
                    break;
                }
            }
        }
        if (!is_load)
        {
            spdlog::error("[luastg] 找不到文件'{}'、'{}'或'{}'", entry_scripts[0], entry_scripts[1], entry_scripts[2]);
        }
        return true;
    }
}
