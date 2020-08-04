#pragma once
#include <string>

namespace app {
	bool getLocalAppDataDirectory(std::wstring& out);

	bool getRoamingAppDataDirectory(std::wstring& out);
};
