#pragma once

#ifndef __UTF8_H__
#define __UTF8_H__

#include <string>
#include <string_view>

namespace utf8
{
	std::string to_string(std::wstring_view wstr);
	std::wstring to_wstring(std::string_view str);
	inline std::string_view remove_bom(std::string_view str)
	{
		if (str.starts_with("\xEF\xBB\xBF")) {
			return str.substr(3);
		}
		else {
			return str;
		}
	}
}

#endif
