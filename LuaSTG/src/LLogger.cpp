#include "LLogger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "Config.h"
#include "Common/SystemDirectory.hpp"

namespace slow {
    bool checkDirectory(std::wstring& out) {
        std::wstring path; // APPDATA
        if (windows::makeApplicationRoamingAppDataDirectory(APP_COMPANY, APP_PRODUCT, path))
        {
            out.append(path);
            out.push_back(L'\\');
            return true;
        }
        else
        {
            return false;
        }
    }
    
    void openLogger() {
        std::wstring path = L"";
        std::wstring file = L"engine.log";
        #ifdef USING_SYSTEM_DIRECTORY
        checkDirectory(path);
        #endif
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path + file, true);
        
        auto logger = std::make_shared<spdlog::logger>("luastg", sink);
        logger->set_level(spdlog::level::trace);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
        //logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
        //logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L] [%n]%$ %v");
        logger->flush_on(spdlog::level::info);
        
        spdlog::set_default_logger(logger);
    }
    void closeLogger() {
        if (auto logger = spdlog::get("luastg")) {
            logger->flush();
        }
        spdlog::drop_all();
        spdlog::shutdown();
    }
};
