#pragma once
#include <vector>
#include <string>
#include <string_view>

namespace Platform
{
    class CommandLineArguments
    {
    private:
        std::vector<std::string> m_args;
    public:
        bool Update();
        bool IsOptionExist(std::string_view option);
    public:
        CommandLineArguments();
        ~CommandLineArguments();
    public:
        static CommandLineArguments& Get();
    };
}
