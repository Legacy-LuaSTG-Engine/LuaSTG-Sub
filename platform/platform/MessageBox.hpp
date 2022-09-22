#pragma once
#include <string_view>

// FUCK MICROSOFT

#ifdef MessageBox
#undef MessageBox
#endif

namespace platform
{
    class MessageBox
    {
    public:
        static bool Warning(std::string_view title, std::string_view message);
        static void Error(std::string_view title, std::string_view message);
        static bool WarningFromWindow(std::string_view title, std::string_view message, void* window);
        static void ErrorFromWindow(std::string_view title, std::string_view message, void* window);
    };
}
