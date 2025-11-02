#pragma once
#include <string_view>

namespace Platform
{
	class WindowsVersion
	{
	public:
		static std::string_view GetName();

		static bool IsEndOfSupport();

		static bool Is7(); // Windows 7
		static bool Is7SP1(); // Windows 7 SP1
		static bool Is7SP1WithPlatformUpdate(); // Windows 7 SP1 + Platform Update (KB2670838)

		static bool Is8(); // Windows 8
		static bool Is8Point1(); // Windows 8.1

		static bool Is10(); // Windows 10
		static bool Is10Build10240(); // Windows 10 1507
		static bool Is10Build10586(); // Windows 10 1511
		static bool Is10Build14393(); // Windows 10 1607
		static bool Is10Build15063(); // Windows 10 1703
		static bool Is10Build16299(); // Windows 10 1709
		static bool Is10Build17134(); // Windows 10 1803
		static bool Is10Build17763(); // Windows 10 1809
		static bool Is10Build18362(); // Windows 10 1903
		static bool Is10Build18363(); // Windows 10 1909
		static bool Is10Build19041(); // Windows 10 2004
		static bool Is10Build19042(); // Windows 10 20H2
		static bool Is10Build19043(); // Windows 10 21H1
		static bool Is10Build19044(); // Windows 10 21H2
		static bool Is10Build19045(); // Windows 10 22H2

		static bool Is11(); // Windows 11
		static bool Is11Build22000(); // Windows 11 21H2
		static bool Is11Build22621(); // Windows 11 22H2
		static bool Is11Build22631(); // Windows 11 23H2
		static bool Is11Build26100(); // Windows 11 24H2
		static bool Is11Build26200(); // Windows 11 25H2

		static std::string_view GetKernelVersionString();
	};
}
