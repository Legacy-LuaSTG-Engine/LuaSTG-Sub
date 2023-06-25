#pragma once

namespace Platform
{
    class ApplicationSingleInstance
    {
    private:
        static constexpr std::string_view error_message_en_us_uuid{"UUID cannot be empty."};
        static constexpr std::string_view error_message_zh_cn_uuid{"UUID不能为空。"};
        static constexpr std::string_view error_message_en_us_convert{"Convert string encoding failed."};
        static constexpr std::string_view error_message_zh_cn_convert{"转换字符串编码失败。"};
        static constexpr std::string_view error_message_en_us_initialize{"Single instance application model has been enabled."};
        static constexpr std::string_view error_message_zh_cn_initialize{"单实例应用程序模型已被启用。"};
        static constexpr std::string_view error_message_en_us_create{"Launch single application instance failed."};
        static constexpr std::string_view error_message_zh_cn_create{"启动单实例应用程序失败。"};
        static constexpr std::string_view error_message_en_us_test{"Launch multiple application instances is not allowed, please terminate the existing instance."};
        static constexpr std::string_view error_message_zh_cn_test{"无法启动多个应用程序实例，请终止已存在的实例。"};
        HANDLE m_event{};
        std::string m_title{"Error"};
    private:
        static inline bool IsChineseSimplified()
        {
            LANGID const language_chinese_simplified = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
            // TODO: This is a deprecated API
        #pragma warning(push)
        #pragma warning(disable: 6387)
            LANGID const language_user_default = LANGIDFROMLCID(LocaleNameToLCID(LOCALE_NAME_USER_DEFAULT, 0));
        #pragma warning(pop)
            return language_user_default == language_chinese_simplified;
        }
        static inline std::wstring Utf8ToWide(std::string_view const str)
        {
            std::wstring wstr;
            int const length = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
            if (length <= 0) {
                return wstr;
            }
            wstr.resize(static_cast<size_t>(length));
            int const result = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wstr.data(), length);
            if (length != result) {
                wstr.clear();
            }
            return wstr;
        }
        static inline std::string_view LocalizeString(std::string_view const en_us, std::string_view const zh_cn)
        {
            return IsChineseSimplified() ? zh_cn : en_us;
        }
        void ShowErrorWindow(std::string_view const text)
        {
            std::wstring wide_text(Utf8ToWide(text));
            std::wstring wide_title(Utf8ToWide(m_title));
            MessageBoxW(
                nullptr,
                wide_text.c_str(),
                wide_title.c_str(),
                MB_ICONERROR | MB_OK
            );
        }
        void Clear()
        {
            if (m_event) {
                CloseHandle(m_event);
                m_event = nullptr;
            }
        }
    public:
        void SetErrorWindowTitle(std::string_view const title)
        {
            m_title = title;
        }
        void Initialize(std::string_view const uuid)
        {
            if (m_event != nullptr) {
                ShowErrorWindow(LocalizeString(error_message_en_us_initialize, error_message_zh_cn_initialize));
                std::exit(EXIT_FAILURE);
            }
            if (uuid.empty()) {
                ShowErrorWindow(LocalizeString(error_message_en_us_uuid, error_message_zh_cn_uuid));
                std::exit(EXIT_FAILURE);
            }
            std::string name;
            name.append("Local\\ApplicationInstance-");
            name.append(uuid);
            std::wstring wide_name(Utf8ToWide(name));
            if (wide_name.empty()) {
                // unlikely
                ShowErrorWindow(LocalizeString(error_message_en_us_convert, error_message_zh_cn_convert));
                std::exit(EXIT_FAILURE);
            }
            SetLastError(0);
            m_event = CreateEventExW(nullptr, wide_name.c_str(), 0, EVENT_ALL_ACCESS);
            if (nullptr == m_event) {
                ShowErrorWindow(LocalizeString(error_message_en_us_create, error_message_zh_cn_create));
                std::exit(EXIT_FAILURE);
            }
            if (ERROR_ALREADY_EXISTS == GetLastError()) {
                Clear();
                ShowErrorWindow(LocalizeString(error_message_en_us_test, error_message_zh_cn_test));
                std::exit(EXIT_FAILURE);
            }
        }
    public:
        ApplicationSingleInstance() = default;
        ApplicationSingleInstance(std::string_view const title) : m_title(title) {}
        ApplicationSingleInstance(std::string_view const title, std::string_view const uuid) : m_title(title)
        {
            Initialize(uuid);
        }
        ApplicationSingleInstance(ApplicationSingleInstance const&) = delete;
        ApplicationSingleInstance(ApplicationSingleInstance&&) = delete;
        ~ApplicationSingleInstance()
        {
            Clear();
        }
    };
}
