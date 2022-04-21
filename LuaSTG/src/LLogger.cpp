#include "LLogger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/wincolor_sink.h"
#include "platform/KnownDirectory.hpp"
#include "platform/CommandLine.hpp"
#include "Config.h"
#include "LConfig.h"

static bool enable_console = false;
static bool open_console = false;
static bool wait_console = false;
static void OpenWin32Console();
static void CloseWin32Console();

namespace slow
{
    bool checkDirectory(std::wstring& out)
    {
        std::wstring path; // APPDATA
        if (platform::KnownDirectory::makeAppDataW(APP_COMPANY, APP_PRODUCT, path))
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
        std::vector<std::string> args(platform::CommandLine::get());
        for (auto const& v : args)
        {
            if (v == "--log-window")
            {
                enable_console = true;
            }
            if (v == "--log-window-wait")
            {
                wait_console = true;
            }
        }

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
        if (open_console)
        {
            auto sink_console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
            sink_console->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
            sinks.emplace_back(sink_console);
        }
    #endif

        auto logger = std::make_shared<spdlog::logger>("luastg", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        //logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
        //logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
        //logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L] [%n]%$ %v");
        logger->flush_on(spdlog::level::info);
        
        spdlog::set_default_logger(logger);
    }
    void closeLogger()
    {
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
    if (enable_console && AllocConsole())
    {
        open_console = true;
        HWND window = GetConsoleWindow();
        HMENU menu = GetSystemMenu(window, FALSE);
        RemoveMenu(menu, SC_CLOSE, MF_BYCOMMAND);
        SetWindowTextA(window, LUASTG_INFO);
        SetConsoleOutputCP(CP_UTF8);
        ShowWindow(window, SW_MAXIMIZE);
    }
}
void CloseWin32Console()
{
    if (open_console)
    {
        if (wait_console)
        {
            std::wstring_view const exit_msg(L"按 ESC 关闭引擎日志窗口 | Press ESC to close the engine log window\n");
            WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), exit_msg.data(), exit_msg.length(), NULL, NULL);
            while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0x8000)
            {
                Sleep(1);
            }
        }
        FreeConsole();
    }
}
