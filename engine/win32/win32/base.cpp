#include "win32/base.hpp"
#include <format>
#include <windows.h>

using std::string_view_literals::operator ""sv;

namespace {
	void default_logger_writer(std::string_view const message) {
		std::printf("%.*s", static_cast<int>(message.size()), message.data());
	}

	std::function<void(std::string_view)> g_logger_writer{ default_logger_writer };

	int wide_to_utf8_length(std::wstring_view const& wide) {
		return WideCharToMultiByte(
			CP_UTF8, 0,
			wide.data(), static_cast<int>(wide.size()),
			nullptr, 0,
			nullptr, nullptr
		);
	}

	int wide_to_utf8(std::wstring_view const& wide, std::string& utf8) {
		return WideCharToMultiByte(
			CP_UTF8, 0,
			wide.data(), static_cast<int>(wide.size()),
			utf8.data(), static_cast<int>(utf8.size()),
			nullptr, nullptr
		);
	}

	std::string to_utf8(std::wstring_view const& wide) {
		std::string utf8;
		if (wide.empty()) {
			return utf8;
		}
		const auto count = wide_to_utf8_length(wide);
		if (count <= 0) {
			return utf8;
		}
		utf8.resize(static_cast<size_t>(count));
		if (wide_to_utf8(wide, utf8) != count) {
			utf8.resize(0);
		}
		return utf8;
	}

	std::string format_message(HRESULT const hr) {
		WCHAR* buffer{};
		const auto result = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			static_cast<DWORD>(hr),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			reinterpret_cast<PWSTR>(&buffer),
			0,
			nullptr
		);
		if (result > 0) {
			auto message = to_utf8(std::wstring_view(buffer, result));
			LocalFree(buffer);
			if (message.ends_with("\r\n"sv)) {
				message.resize(message.size() - 2);
			}
			return message;
		}
		if (buffer != nullptr) {
			LocalFree(buffer);
		}
		return std::string{ "unknown error"sv };
	}
}

namespace win32 {
	void set_logger_writer(std::function<void(std::string_view)> const& writer) {
		g_logger_writer = writer ? writer : default_logger_writer;
	}

#ifndef NDEBUG
#define SL , std::source_location const& sl
#define PSL , sl
#else
#define SL
#define PSL
#endif

	std::string hresult_error::message() const {
		return format_message(m_code);
	}

	hresult check_hresult(hresult const hr SL) {
		if (FAILED(hr)) {
			const auto message = format_message(hr);
			g_logger_writer(std::format(
			#ifndef NDEBUG
				"{}:{}: "
			#endif
				"(0x{:08x}) {}",
			#ifndef NDEBUG
				sl.file_name(), sl.line(),
			#endif
				static_cast<uint32_t>(hr), message
			));
		}
		return hr;
	}

	hresult check_hresult(hresult const hr, std::string_view const api SL) {
		if (FAILED(hr)) {
			const auto message = format_message(hr);
			g_logger_writer(std::format(
			#ifndef NDEBUG
				"{}:{}: "
			#endif
				"{} failed: (0x{:08x}) {}",
			#ifndef NDEBUG
				sl.file_name(), sl.line(),
			#endif
				api, static_cast<uint32_t>(hr), message
			));
		}
		return hr;
	}

	bool check_hresult_as_boolean(hresult const hr SL) {
		return SUCCEEDED(check_hresult(hr PSL));
	}

	bool check_hresult_as_boolean(hresult const hr, std::string_view const api SL) {
		return SUCCEEDED(check_hresult(hr, api PSL));
	}

	void check_hresult_throw_if_failed(hresult const hr SL) {
		if (!check_hresult_as_boolean(hr PSL)) {
			throw hresult_error(hr, "unknown call");
		}
	}

	void check_hresult_throw_if_failed(hresult const hr, std::string_view const api SL) {
		if (!check_hresult_as_boolean(hr, api PSL)) {
			throw hresult_error(hr, std::string(api));
		}
	}

#undef SL
#undef PSL
}
