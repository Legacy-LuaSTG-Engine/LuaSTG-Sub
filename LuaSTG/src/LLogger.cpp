#include "LLogger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/wincolor_sink.h"
#include "Common/SystemDirectory.hpp"
#include "Config.h"
#include "LConfig.h"

static void OpenWin32Console();
static void CloseWin32Console();

namespace slow {
    bool checkDirectory(std::wstring& out)
    {
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
    
    void openLogger()
    {
    #ifdef USING_CONSOLE_OUTPUT
        OpenWin32Console();
    #endif

        std::vector<spdlog::sink_ptr> sinks;

        std::wstring path = L"";
        std::wstring file = L"engine.log";
        #ifdef USING_SYSTEM_DIRECTORY
        checkDirectory(path);
        #endif
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path + file, true);
        sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
        sinks.emplace_back(sink);

    #ifdef USING_CONSOLE_OUTPUT
        auto sink_console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
        sink_console->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
        sinks.emplace_back(sink_console);
    #endif

        auto logger = std::make_shared<spdlog::logger>("luastg", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        //logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
        //logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
        //logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L] [%n]%$ %v");
        logger->flush_on(spdlog::level::info);
        
        spdlog::set_default_logger(logger);
    }
    void closeLogger() {
        if (auto logger = spdlog::get("luastg"))
        {
            logger->flush();
        }
        spdlog::drop_all();
        spdlog::shutdown();
    #ifdef USING_CONSOLE_OUTPUT
        CloseWin32Console();
    #endif
    }
};

#define NOMINMAX
#include <Windows.h>

void OpenWin32Console()
{
    AllocConsole();
    HWND window = GetConsoleWindow();
    HMENU menu = GetSystemMenu(window, FALSE);
    RemoveMenu(menu, SC_CLOSE, MF_BYCOMMAND);
    SetWindowTextA(window, LUASTG_INFO);
    SetConsoleOutputCP(CP_UTF8);
    ShowWindow(window, SW_MAXIMIZE);
}
void CloseWin32Console()
{
    FreeConsole();
}
