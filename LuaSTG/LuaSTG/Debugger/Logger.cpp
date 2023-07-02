#include "Debugger/Logger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/wincolor_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "Platform/HResultChecker.hpp"
#include "Platform/CommandLineArguments.hpp"
#include "Core/InitializeConfigure.hpp"
#include "utf8.hpp"

namespace LuaSTG::Debugger
{
    static bool enable_console = false;
    static bool open_console = false;
    static bool wait_console = false;
    static void openWin32Console();
    static void closeWin32Console();
    static std::string make_time_path();

    void Logger::create()
    {
        Core::InitializeConfigure config;
        config.loadFromFile("config.json");

    #ifdef USING_CONSOLE_OUTPUT
        enable_console = Platform::CommandLineArguments::Get().IsOptionExist("--log-window");
        wait_console = Platform::CommandLineArguments::Get().IsOptionExist("--log-window-wait");
        openWin32Console();
    #endif

        std::vector<spdlog::sink_ptr> sinks;

        if (config.log_file_enable)
        {
            std::string parser_path;
            if (config.log_file_path.empty())
            {
                parser_path = "engine.log";
            }
            else
            {
                Core::InitializeConfigure::parserFilePath(config.log_file_path, parser_path, true);
            }
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(utf8::to_wstring(parser_path), true);
            sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
            sinks.emplace_back(sink);
        }
        
        if (config.persistent_log_file_enable)
        {
            std::filesystem::path path;
            if (config.persistent_log_file_directory.empty())
            {
                std::filesystem::path directory(utf8::to_wstring("logs/")); // TODO: 这里写死了
                path = directory / utf8::to_wstring(make_time_path());
            }
            else
            {
                std::string parser_path;
                Core::InitializeConfigure::parserDirectory(config.persistent_log_file_directory, parser_path, true);
                std::filesystem::path directory(utf8::to_wstring(parser_path));
                path = directory / utf8::to_wstring(make_time_path());
            }
            auto persistent_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.wstring(), true);
            persistent_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
            sinks.emplace_back(persistent_sink);
        }
        
    #if 0
        auto sink_debugger = std::make_shared<spdlog::sinks::windebug_sink_mt>();
        sink_debugger->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
        sinks.emplace_back(sink_debugger);
    #endif

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

        struct HResultCheckerCallback
        {
            static void WriteError(std::string_view const message)
            {
                spdlog::error("[luastg] {}", message);
            }
        };

        Platform::HResultChecker::SetPrintCallback(&HResultCheckerCallback::WriteError);
    }
    void Logger::destroy()
    {
        if (auto logger = spdlog::get("luastg"))
        {
            logger->flush();
        }
        
        Platform::HResultChecker::SetPrintCallback();

        spdlog::drop_all();
        spdlog::shutdown();

    #ifdef USING_CONSOLE_OUTPUT
        closeWin32Console();
    #endif

        // 清理 log 文件

        Core::InitializeConfigure config;
        config.loadFromFile("config.json");

        if (config.persistent_log_file_enable)
        {
            std::filesystem::path path;
            if (config.persistent_log_file_directory.empty())
            {
                std::filesystem::path directory(utf8::to_wstring("logs/")); // TODO: 这里写死了
                path = directory;
            }
            else
            {
                std::string parser_path;
                Core::InitializeConfigure::parserDirectory(config.persistent_log_file_directory, parser_path, false);
                std::filesystem::path directory(utf8::to_wstring(parser_path));
                path = directory;
            }
            // 下面就全都用宽字符了吧，省心
            std::error_code ec;
            std::vector<std::filesystem::path> logs;
            for (auto& it : std::filesystem::directory_iterator(path, ec)) {
                if (it.is_regular_file(ec) && it.path().extension() == L".log") {
                    logs.push_back(it.path());
                }
            }
            if (logs.size() > static_cast<size_t>(config.persistent_log_file_max_count)) {
                std::sort(logs.begin(), logs.end(), [](std::filesystem::path const& a, std::filesystem::path const& b) -> bool
                    {
                        return a.filename().generic_wstring() < b.filename().generic_wstring();
                    });
                size_t const remove_count = logs.size() - static_cast<size_t>(config.persistent_log_file_max_count);
                for (size_t idx = 0; idx < remove_count; idx += 1) {
                    std::filesystem::remove(logs[idx], ec);
                }
            }
        }
    }
};

#include "Platform/CleanWindows.hpp"

namespace LuaSTG::Debugger
{
    void openWin32Console()
    {
        if (enable_console && AllocConsole())
        {
            open_console = true;
            HWND window = GetConsoleWindow();
            HMENU menu = GetSystemMenu(window, FALSE);
            RemoveMenu(menu, SC_CLOSE, MF_BYCOMMAND);
            SetWindowTextW(window, L"" LUASTG_INFO);
            ShowWindow(window, SW_MAXIMIZE);
            SetConsoleOutputCP(CP_UTF8);
        }
    }
    void closeWin32Console()
    {
        if (open_console)
        {
            if (wait_console)
            {
                std::wstring_view const exit_msg(L"按 ESC 关闭引擎日志窗口 | Press ESC to close the engine log window\n");
                WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), exit_msg.data(), (DWORD)exit_msg.length(), NULL, NULL);
                while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0x8000)
                {
                    Sleep(1);
                }
            }
            FreeConsole();
        }
    }
    std::string make_time_path()
    {
        SYSTEMTIME current_time{};
        GetLocalTime(&current_time);

        std::array<char, 32> buffer{};
        int const length = std::snprintf(
            buffer.data(), buffer.size(),
            "%04u-%02u-%02u-%02u-%02u-%02u-%03u",
            current_time.wYear,
            current_time.wMonth,
            current_time.wDay,
            current_time.wHour,
            current_time.wMinute,
            current_time.wSecond,
            current_time.wMilliseconds);
        assert(length > 0);

        std::string path;
        path.append("engine-");
        path.append(std::string_view(buffer.data(), static_cast<size_t>(length)));
        path.append(".log");
        return path;
    }
}
