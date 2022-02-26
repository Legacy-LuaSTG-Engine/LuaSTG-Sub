#pragma once
#include <string>

namespace windows
{
	bool getLocalAppDataDirectory(std::wstring& out);
	bool getRoamingAppDataDirectory(std::wstring& out);
	bool makeApplicationRoamingAppDataDirectory(const std::wstring& company, const std::wstring& product, std::wstring& out);
};
