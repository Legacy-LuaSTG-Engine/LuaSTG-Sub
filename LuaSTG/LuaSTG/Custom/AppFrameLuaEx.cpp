#include "AppFrame.h"
#include "Config.h"
#include "Core/FileManager.hpp"
#include "Core/InitializeConfigure.hpp"

namespace LuaSTGPlus
{
    bool AppFrame::OnLoadLaunchScriptAndFiles()
    {
        bool is_launch_loaded = false;
        #ifdef USING_LAUNCH_FILE
        spdlog::info("[luastg] 加载初始化脚本");
        std::vector<uint8_t> src;
        if (GFileManager().loadEx("launch", src))
        {
            if (SafeCallScript((char const*)src.data(), src.size(), "launch"))
            {
                is_launch_loaded = true;
                spdlog::info("[luastg] 加载脚本'launch'");
            }
            else
            {
                spdlog::error("[luastg] 加载初始化脚本'launch'失败");
            }
        }
        #endif
        if (!is_launch_loaded)
        {
            Core::InitializeConfigure config;
            if (config.loadFromFile("config.json"))
            {
                spdlog::info("[luastg] 发现配置文件'config.json'");
                LAPP.SetWindowed(config.windowed);
                LAPP.SetDefaultWindowStyle(config.windowed ? Core::Graphics::WindowFrameStyle::Fixed : Core::Graphics::WindowFrameStyle::None);
                LAPP.SetVsync(config.vsync);
                LAPP.SetResolution(config.width, config.height, config.refresh_rate_numerator, config.refresh_rate_denominator);
                if (!config.gpu.empty())
                {
                    LAPP.SetPreferenceGPU(config.gpu.c_str(), config.dgpu_trick);
                }
                is_launch_loaded = true;
            }
        }
        #ifdef USING_LAUNCH_FILE
        if (!is_launch_loaded)
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
        std::vector<uint8_t> src;
        bool is_load = false;
        for (auto& v : entry_scripts)
        {
            if (GFileManager().loadEx(v, src))
            {
                if (SafeCallScript((char const*)src.data(), src.size(), v.data()))
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
