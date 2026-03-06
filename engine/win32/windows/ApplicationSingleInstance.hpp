#pragma once
#include <string>
#include <string_view>

namespace Platform
{
    class ApplicationSingleInstance
    {
    private:
        typedef void* HANDLE;
        HANDLE m_event{};
        std::string m_title{"Error"};
    private:
        void ShowErrorWindow(std::string_view const text);
        void Clear();
    public:
        void SetErrorWindowTitle(std::string_view const title);
        void Initialize(std::string_view const uuid);
    public:
        ApplicationSingleInstance();
        ApplicationSingleInstance(std::string_view const title);
        ApplicationSingleInstance(std::string_view const title, std::string_view const uuid);
        ApplicationSingleInstance(ApplicationSingleInstance const&) = delete;
        ApplicationSingleInstance(ApplicationSingleInstance&&) = delete;
        ~ApplicationSingleInstance();
    };
}
