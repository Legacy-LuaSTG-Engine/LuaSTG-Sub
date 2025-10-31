#include "core/ShellIntegration.hpp"
#include "core/Logger.hpp"
#include <cassert>
#include <format>
#include <filesystem>
#include "utf8.hpp"
#include <windows.h>
#include <shlobj_core.h>

using std::string_literals::operator ""s;
using std::string_view_literals::operator ""sv;

namespace {
	struct StringResource {
		std::string_view text;
		LCID locale;
		bool fallback;
	};

#define SR(LANG, SUB_LANG, STR, FALLBACK) \
	StringResource{ \
		.text = (STR), \
		.locale = MAKELANGID((LANG), (SUB_LANG)), \
		.fallback = (FALLBACK), \
	}

	std::string getUserDefaultLocaleName() {
		WCHAR buffer[LOCALE_NAME_MAX_LENGTH]{};
		int const length = GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
		if (length < 1) {
			return "en-US"s;
		}
		return utf8::to_string(std::wstring_view(buffer, static_cast<size_t>(length)));
	}

	LCID getUserDefaultLocaleNameToLCID() {
		WCHAR buffer[LOCALE_NAME_MAX_LENGTH]{};
		int const length = GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
		assert(length >= 1);
		if (length < 1) {
			return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
		}
		LCID const id = LocaleNameToLCID(buffer, 0);
		assert(id != 0);
		if (id == 0) {
			return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
		}
		return id;
	}

	constexpr auto no_text{ "<null>"sv };

	template<size_t N>
	std::string_view const& i18n(StringResource const (&resources)[N]) {
		LCID const locale = getUserDefaultLocaleNameToLCID();
		StringResource const* fallback{};
		for (size_t i = 0; i < N; i += 1) {
			StringResource const& v = resources[i];
			if (v.fallback) {
				fallback = &v;
			}
			if (v.locale == locale) {
				return v.text;
			}
		}
		assert(fallback != nullptr);
		if (fallback != nullptr) {
			return fallback->text;
		}
		return no_text;
	}

	std::u8string_view u8sv(std::string_view const s) {
		return { reinterpret_cast<char8_t const*>(s.data()), s.size() };
	}
	std::string_view u8sv(std::u8string_view const s) {
		return { reinterpret_cast<char const*>(s.data()), s.size() };
	}

	bool open(std::string_view const op, std::string_view const url) {
		const auto url_w{ utf8::to_wstring(url) };
		SHELLEXECUTEINFOW info{};
		info.cbSize = sizeof(info);
		info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC;
		info.lpVerb = L"open";
		info.lpFile = url_w.c_str();
		info.nShow = SW_SHOWDEFAULT;
		const auto result = ShellExecuteExW(&info);
		if (!result) {
			core::Logger::error(
				"[core::ShellIntegration] {} failed: "
				"ShellExecuteExW failed {} (last error {})"sv,
				op,
				reinterpret_cast<size_t>(info.hInstApp),
				GetLastError()
			);
		}
		return result != FALSE;
	}
}

namespace {
	constexpr StringResource open_file_title[]{
		// en
		SR(LANG_ENGLISH, SUBLANG_ENGLISH_US, "Open file"sv, true),
		// zh
		SR(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED, "打开文件"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG, "開啟檔案"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_MACAU, "開啟檔案"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL, "開啟檔案"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE, "打开文件"sv, false),
		// jp
		SR(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN, "ファイルを開く"sv, false),
		// kr
		SR(LANG_KOREAN, SUBLANG_KOREAN, "파일 열기"sv, false),
	};
	constexpr StringResource open_file_message[]{
		// en
		SR(LANG_ENGLISH, SUBLANG_ENGLISH_US, "About to open file:\r\n{}"sv, true),
		// zh
		SR(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED, "即将打开文件：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG, "即將開啟檔案：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_MACAU, "即將開啟檔案：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL, "即將開啟檔案：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE, "即将打开文件：\r\n{}"sv, false),
		// jp
		SR(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN, "ファイルを開く予定です：\r\n{}"sv, false),
		// kr
		SR(LANG_KOREAN, SUBLANG_KOREAN, "파일을 열 예정입니다:\r\n{}"sv, false),
	};

	constexpr StringResource open_directory_title[]{
		// en
		SR(LANG_ENGLISH, SUBLANG_ENGLISH_US, "Open folder"sv, true),
		// zh
		SR(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED, "打开文件夹"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG, "開啟資料夾"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_MACAU, "開啟資料夾"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL, "開啟資料夾"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE, "打开文件夹"sv, false),
		// jp
		SR(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN, "フォルダーを開く"sv, false),
		// kr
		SR(LANG_KOREAN, SUBLANG_KOREAN, "폴더 열기"sv, false),
	};
	constexpr StringResource open_directory_message[]{
		// en
		SR(LANG_ENGLISH, SUBLANG_ENGLISH_US, "About to open folder:\r\n{}"sv, true),
		// zh
		SR(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED, "即将打开文件夹：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG, "即將開啟資料夾：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_MACAU, "即將開啟資料夾：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL, "即將開啟資料夾：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE, "即将打开文件夹：\r\n{}"sv, false),
		// jp
		SR(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN, "フォルダーを開く予定です：\r\n{}"sv, false),
		// kr
		SR(LANG_KOREAN, SUBLANG_KOREAN, "폴더를 열 예정입니다:\r\n{}"sv, false),
	};

	constexpr StringResource open_url_title[]{
		// en
		SR(LANG_ENGLISH, SUBLANG_ENGLISH_US, "Open website"sv, true),
		// zh
		SR(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED, "打开网站"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG, "打開網站"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_MACAU, "打開網站"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL, "開啟網站"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE, "打开网站"sv, false),
		// jp
		SR(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN, "Web サイトを開く"sv, false),
		// kr
		SR(LANG_KOREAN, SUBLANG_KOREAN, "웹 사이트 열기"sv, false),
	};
	constexpr StringResource open_url_message[]{
		// en
		SR(LANG_ENGLISH, SUBLANG_ENGLISH_US, "About to open website:\r\n{}"sv, true),
		// zh
		SR(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED, "即将打开网站：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG, "即將開啟網站：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_MACAU, "即將開啟網站：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL, "即將開啟網站：\r\n{}"sv, false),
		SR(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE, "即将打开网站：\r\n{}"sv, false),
		// jp
		SR(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN, "Web サイトを開く予定です：\r\n{}"sv, false),
		// kr
		SR(LANG_KOREAN, SUBLANG_KOREAN, "웹 사이트를 열 예정입니다:\r\n{}"sv, false),
	};
}

namespace core {
	bool ShellIntegration::openFile(std::string_view const path) {
		try {
			const auto fs_path{ std::filesystem::path(u8sv(path)) };
			const auto fs_abs_path{ std::filesystem::absolute(fs_path.lexically_normal()) };
			if (!std::filesystem::is_regular_file(fs_abs_path)) {
				Logger::error("[core::ShellIntegration] openFile failed: file does not exists"sv);
				return false;
			}
			const auto path_u8{ fs_abs_path.u8string() };
			const auto path_u8_v{ u8sv(path_u8) };

			const auto title{ i18n(open_file_title) };
			const auto message{ std::vformat(i18n(open_file_message), std::make_format_args(path_u8_v)) };
			const auto title_w{ utf8::to_wstring(title) };
			const auto message_w{ utf8::to_wstring(message) };
			if (MessageBoxW(nullptr, message_w.c_str(), title_w.c_str(), MB_ICONWARNING | MB_OKCANCEL) != IDOK) {
				Logger::warn("[core::ShellIntegration] openFile canceled by user"sv);
				return false;
			}

			const auto path_w{ fs_abs_path.wstring() };
			OPENASINFO info{};
			info.pcszFile = path_w.c_str();
			info.oaifInFlags = OAIF_EXEC | OAIF_HIDE_REGISTRATION;
			const auto result = SHOpenWithDialog(nullptr, &info);
			if (FAILED(result)) {
				Logger::error("[core::ShellIntegration] openFile failed: SHOpenWithDialog failed (HRESULT 0x{:08x})"sv, static_cast<uint32_t>(result));
			}
			return SUCCEEDED(result);
		}
		catch (std::exception const& e) {
			Logger::error("[core::ShellIntegration] openFile failed: {}"sv, e.what());
		}
		return false;
	}
	bool ShellIntegration::openDirectory(std::string_view const path) {
		try {
			const auto fs_path{ std::filesystem::path(u8sv(path)) };
			const auto fs_abs_path{ std::filesystem::absolute(fs_path.lexically_normal()) };
			if (!std::filesystem::is_directory(fs_abs_path)) {
				Logger::error("[core::ShellIntegration] openDirectory failed: directory does not exists"sv);
				return false;
			}
			const auto path_u8{ fs_abs_path.u8string() };
			const auto path_u8_v{ u8sv(path_u8) };

			const auto title{ i18n(open_directory_title) };
			const auto message{ std::vformat(i18n(open_directory_message), std::make_format_args(path_u8_v)) };
			const auto title_w{ utf8::to_wstring(title) };
			const auto message_w{ utf8::to_wstring(message) };
			if (MessageBoxW(nullptr, message_w.c_str(), title_w.c_str(), MB_ICONWARNING | MB_OKCANCEL) != IDOK) {
				Logger::warn("[core::ShellIntegration] openDirectory canceled by user"sv);
				return false;
			}

			return open("openDirectory"sv, path_u8_v);
		}
		catch (std::exception const& e) {
			Logger::error("[core::ShellIntegration] openDirectory failed: {}"sv, e.what());
		}
		return false;
	}
	bool ShellIntegration::openUrl(std::string_view const url) {
		try {
			const auto title{ i18n(open_url_title) };
			const auto message{ std::vformat(i18n(open_url_message), std::make_format_args(url)) };
			const auto title_w{ utf8::to_wstring(title) };
			const auto message_w{ utf8::to_wstring(message) };
			if (MessageBoxW(nullptr, message_w.c_str(), title_w.c_str(), MB_ICONWARNING | MB_OKCANCEL) != IDOK) {
				Logger::warn("[core::ShellIntegration] openUrl canceled by user"sv);
				return false;
			}
			return open("openUrl"sv, url);
		}
		catch (std::exception const& e) {
			Logger::error("[core::ShellIntegration] openUrl failed: {}"sv, e.what());
		}
		return false;
	}
}
