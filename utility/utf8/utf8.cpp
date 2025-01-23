#include "utf8.hpp"
#include <cassert>
#include <windows.h>

namespace utf8 {
	static_assert(CHAR_BIT == 8);
	std::string to_string(std::wstring_view const& str) {
		std::string buffer;
		if (str.empty()) {
			return buffer;
		}
		int const length = WideCharToMultiByte(
			CP_UTF8, 0,
			str.data(), static_cast<int>(str.size()),
			nullptr, 0,
			nullptr, nullptr);
		if (length <= 0) {
			assert(false);
			return buffer;
		}
		buffer.resize(static_cast<size_t>(length));
		int const result = WideCharToMultiByte(
			CP_UTF8, 0,
			str.data(), static_cast<int>(str.length()),
			buffer.data(), length,
			nullptr, nullptr);
		if (result != length) {
			assert(false);
			buffer.clear();
		}
		return buffer;
	}
	std::wstring to_wstring(std::string_view const& str) {
		std::wstring buffer;
		if (str.empty()) {
			return buffer;
		}
		int const length = MultiByteToWideChar(
			CP_UTF8, 0,
			str.data(), static_cast<int>(str.length()),
			nullptr, 0);
		if (length <= 0) {
			assert(false);
			return buffer;
		}
		buffer.resize(static_cast<size_t>(length));
		int const result = MultiByteToWideChar(
			CP_UTF8, 0,
			str.data(), static_cast<int>(str.length()),
			buffer.data(), length);
		if (result != length) {
			assert(false);
			buffer.clear();
		}
		return buffer;
	}
}
