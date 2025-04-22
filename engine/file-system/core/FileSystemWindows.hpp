#pragma once
#include <string>

namespace win32 {
	bool isFilePathCaseCorrect(std::string_view const& path, std::string& correct);
	bool isFilePathCaseCorrect(std::string const& path, std::string& correct);
	bool isFilePathCaseCorrect(std::wstring_view const& path, std::wstring& correct);
	bool isFilePathCaseCorrect(std::wstring const& path, std::wstring& correct);
}
