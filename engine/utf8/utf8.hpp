#pragma once

#ifndef _UTF8_H_20250101_
#define _UTF8_H_20250101_

#include <string>
#include <string_view>

namespace utf8 {
	static_assert(CHAR_BIT == 8);
	inline std::string to_string(std::u8string_view const& str) {
		return { reinterpret_cast<char const*>(str.data()), str.size() };
	}
	std::string to_string(std::wstring_view const& str);
	std::wstring to_wstring(std::string_view const& str);
	inline std::wstring to_wstring(std::u8string_view const& str) {
		return to_wstring(std::string_view(reinterpret_cast<char const*>(str.data()), str.size()));
	}
	inline std::string_view remove_bom(std::string_view const& str) {
		if (str.starts_with("\xEF\xBB\xBF")) {
			return str.substr(3);
		}
		return str;
	}
}

#endif // _UTF8_H_20250101_
