#pragma once
#include <string>
#include <string_view>

namespace platform
{
	class KnownDirectory
	{
	public:
		static bool getLocalAppDataW(std::wstring& buf);
		static bool getRoamingAppDataW(std::wstring& buf);
		static bool makeAppDataW(std::wstring_view const& company, std::wstring_view const& product, std::wstring& buf);

		static bool getLocalAppData(std::string& buf);
		static bool getRoamingAppData(std::string& buf);
		static bool makeAppData(std::string_view const& company, std::string_view const& product, std::string& buf);
	};
};
