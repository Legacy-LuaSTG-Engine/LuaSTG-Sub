#include "luastg_config_generated.h"
#include "AppFrame.h"
#include "Config.h"
#include "core/Logger.hpp"
#include "core/FileSystem.hpp"
#include <ranges>
#include <algorithm>

#include "windows/HResultChecker.hpp"

using std::string_view_literals::operator""sv;
using std::string_literals::operator""s;

namespace {
    constexpr bool not_space(const char c) {
        switch (c) {
        case '\t': case '\n': case '\v': case '\f': case '\r': case ' ':
            return false;
        default:
            return true;
        }
    }

    std::string_view trim(const std::string_view s) {
        const auto begin = std::ranges::find_if(s, not_space);
        if (begin == s.end()) return {};
        const auto end = std::ranges::find_if(s | std::views::reverse, not_space).base();
        return { begin, end };
    }

    std::vector<std::string_view> split(const std::string_view s, const char sep) {
        std::vector<std::string_view> r;
        for (size_t i = 0, j = s.find(sep, 0); ; i = j + 1, j = s.find(sep, j + 1)) {
            if (j != std::string_view::npos) {
                r.push_back(trim(s.substr(i, j - i)));
            }
            else {
                r.push_back(trim(s.substr(i)));
                break;
            }
        }
        return r;
    }
}

namespace luastg {
    bool AppFrame::OnLoadLaunchScriptAndFiles() {
    #ifdef LUASTG_CONFIGURATION_LUA_SCRIPT_ENABLE
        constexpr auto raw_script_paths{ LUASTG_CONFIGURATION_LUA_SCRIPT_PATHS ""sv };
        const auto script_paths{ split(raw_script_paths, ';')};

        auto found = false;
        auto script_path{ ""s };
        core::SmartReference<core::IData> script;
        for (const auto& path : script_paths) {
            if (core::FileSystemManager::readFile(path, script.put())) {
                found = true;
                script_path = path;
                break;
            }
        }

        if (!found) {
            return true; // not an error
        }
        if (SafeCallScript(static_cast<const char*>(script->data()), script->size(), script_path.c_str())) {
            core::Logger::info("[luastg] load configuration from script '{}'"sv, script_path);
        }
        else {
            core::Logger::error("[luastg] failed to load configuration from script '{}'"sv, script_path);
            return false; // this is error
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
        core::SmartReference<core::IData> src;
        bool is_load = false;
        for (auto& v : entry_scripts)
        {
            if (core::FileSystemManager::readFile(v, src.put()))
            {
                if (SafeCallScript((char const*)src->data(), src->size(), v.data()))
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
