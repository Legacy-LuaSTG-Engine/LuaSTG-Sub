#include "luastg_config_generated.h"
#include "AppFrame.h"
#include "Config.h"
#include "core/Logger.hpp"
#include "core/FileSystem.hpp"
#include <ranges>
#include <algorithm>

namespace {
    using std::string_view_literals::operator""sv;
    using std::string_literals::operator""s;

    const std::string_view entry_scripts[3] = {
        "core.lua"sv,
        "main.lua"sv,
        "src/main.lua"sv,
    };

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

    bool AppFrame::OnLoadMainScriptAndFiles() {
        core::Logger::info("[luastg] loading main script");
        core::SmartReference<core::IData> src;
        for (const auto& path : entry_scripts) {
            if (!core::FileSystemManager::readFile(path, src.put())) {
                continue;
            }
            if (SafeCallScript(static_cast<const char*>(src->data()), src->size(), path.data())) {
                core::Logger::info("[luastg] main script '{}' loaded", path);
                return true;
            }
        }
        core::Logger::error("[luastg] main script not found");
        return false;
    }
}
