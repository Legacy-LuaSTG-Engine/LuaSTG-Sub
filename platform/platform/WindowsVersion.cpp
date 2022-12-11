#include "WindowsVersion.hpp"

#pragma comment(lib, "Version.lib")

namespace platform
{
	namespace detail
	{
		static bool read = false;
		static std::string_view name = "";
		static bool eos = false;

		void refresh()
		{
			if (read)
			{
				return;
			}

		#define MAKE(N,S) name = N; eos = S; read = true; return

			// Windows 11

			if (IsWindows10BuildOrGreater(22622))
			{
				MAKE("Windows 11 22H2+", false);
			}
			if (WindowsVersion::Is11Build22621())
			{
				MAKE("Windows 11 22H2", false);
			}
			if (WindowsVersion::Is11Build22000())
			{
				MAKE("Windows 11 21H2", false);
			}
			if (WindowsVersion::Is11())
			{
				MAKE("Windows 11", false);
			}

			// Windows 10

			if (IsWindows10BuildOrGreater(19046))
			{
				MAKE("Windows 10 22H2+", false);
			}
			if (WindowsVersion::Is10Build19045())
			{
				MAKE("Windows 10 22H2", false);
			}
			if (WindowsVersion::Is10Build19044())
			{
				MAKE("Windows 10 21H2", false);
			}
			if (WindowsVersion::Is10Build19043())
			{
				MAKE("Windows 10 21H1", true);
			}
			if (WindowsVersion::Is10Build19042())
			{
				MAKE("Windows 10 20H2", false);
			}
			if (WindowsVersion::Is10Build19041())
			{
				MAKE("Windows 10 2004", true);
			}
			if (WindowsVersion::Is10Build18363())
			{
				MAKE("Windows 10 1909", true);
			}
			if (WindowsVersion::Is10Build18362())
			{
				MAKE("Windows 10 1903", true);
			}
			if (WindowsVersion::Is10Build17763())
			{
				MAKE("Windows 10 1809", true);
			}
			if (WindowsVersion::Is10Build17134())
			{
				MAKE("Windows 10 1803", true);
			}
			if (WindowsVersion::Is10Build16299())
			{
				MAKE("Windows 10 1709", true);
			}
			if (WindowsVersion::Is10Build15063())
			{
				MAKE("Windows 10 1703", true);
			}
			if (WindowsVersion::Is10Build14393())
			{
				MAKE("Windows 10 1607", true);
			}
			if (WindowsVersion::Is10Build10586())
			{
				MAKE("Windows 10 1511", true);
			}
			if (WindowsVersion::Is10Build10240())
			{
				MAKE("Windows 10 1507", true);
			}
			if (WindowsVersion::Is10())
			{
				MAKE("Windows 10", true);
			}

			// Windows 8

			if (WindowsVersion::Is8Point1())
			{
				MAKE("Windows 8.1", false);
			}
			if (WindowsVersion::Is8())
			{
				MAKE("Windows 8", true);
			}

			// Windows 7

			if (WindowsVersion::Is7SP1WithPlatformUpdate())
			{
				MAKE("Windows 7 SP1 With Platform Update", true);
			}
			if (WindowsVersion::Is7SP1())
			{
				MAKE("Windows 7 SP1", true);
			}
			if (WindowsVersion::Is7())
			{
				MAKE("Windows 7", true);
			}

			// Windows Vista

			if (IsWindowsVistaSP2OrGreater())
			{
				MAKE("Windows Vista SP2", true);
			}
			if (IsWindowsVistaSP1OrGreater())
			{
				MAKE("Windows Vista SP1", true);
			}
			if (IsWindowsVistaOrGreater())
			{
				MAKE("Windows Vista", true);
			}

			// Windows XP

			if (IsWindowsXPSP3OrGreater())
			{
				MAKE("Windows XP SP3", true);
			}
			if (IsWindowsXPSP2OrGreater())
			{
				MAKE("Windows XP SP2", true);
			}
			if (IsWindowsXPSP1OrGreater())
			{
				MAKE("Windows XP SP1", true);
			}
			if (IsWindowsXPOrGreater())
			{
				MAKE("Windows XP", true);
			}

			// Old

			MAKE("Windows ?", true);

		#undef MAKE
		}

		static bool read_dxgi = false;
		static bool dxgi_1_2 = false;

		bool check_dxgi_1_2()
		{
			if (read_dxgi)
			{
				return dxgi_1_2;
			}
			BOOL update = FALSE;
			if (HMODULE dll_dxgi = LoadLibraryW(L"dxgi.dll"))
			{
				if (decltype(CreateDXGIFactory1) * api_CreateDXGIFactory1 = (decltype(CreateDXGIFactory1)*)GetProcAddress(dll_dxgi, "CreateDXGIFactory1"))
				{
					Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
					if (SUCCEEDED(api_CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory))))
					{
						Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
						if (SUCCEEDED(dxgi_factory.As(&dxgi_factory2)))
						{
							update = TRUE;
						}
					}
				}
				FreeLibrary(dll_dxgi);
			}
			read_dxgi = true;
			dxgi_1_2 = update;
			return dxgi_1_2;
		}
	}

	std::string_view WindowsVersion::GetName()
	{
		detail::refresh();
		return detail::name;
	}

	bool WindowsVersion::IsEndOfSupport()
	{
		detail::refresh();
		return detail::eos;
	}

	bool WindowsVersion::Is7() { return IsWindows7OrGreater(); }
	bool WindowsVersion::Is7SP1() { return IsWindows7SP1OrGreater(); }
	bool WindowsVersion::Is7SP1WithPlatformUpdate() { return IsWindows7SP1OrGreater() && detail::check_dxgi_1_2(); }

	bool WindowsVersion::Is8() { return IsWindows8OrGreater(); }
	bool WindowsVersion::Is8Point1() { return IsWindows8Point1OrGreater(); }

	bool WindowsVersion::Is10() { return IsWindows10OrGreater(); }
	bool WindowsVersion::Is10Build10240() { return IsWindows10BuildOrGreater(10240); }
	bool WindowsVersion::Is10Build10586() { return IsWindows10BuildOrGreater(10586); }
	bool WindowsVersion::Is10Build14393() { return IsWindows10BuildOrGreater(14393); }
	bool WindowsVersion::Is10Build15063() { return IsWindows10BuildOrGreater(15063); }
	bool WindowsVersion::Is10Build16299() { return IsWindows10BuildOrGreater(16299); }
	bool WindowsVersion::Is10Build17134() { return IsWindows10BuildOrGreater(17134); }
	bool WindowsVersion::Is10Build17763() { return IsWindows10BuildOrGreater(17763); }
	bool WindowsVersion::Is10Build18362() { return IsWindows10BuildOrGreater(18362); }
	bool WindowsVersion::Is10Build18363() { return IsWindows10BuildOrGreater(18363); }
	bool WindowsVersion::Is10Build19041() { return IsWindows10BuildOrGreater(19041); }
	bool WindowsVersion::Is10Build19042() { return IsWindows10BuildOrGreater(19042); }
	bool WindowsVersion::Is10Build19043() { return IsWindows10BuildOrGreater(19043); }
	bool WindowsVersion::Is10Build19044() { return IsWindows10BuildOrGreater(19044); }
	bool WindowsVersion::Is10Build19045() { return IsWindows10BuildOrGreater(19045); }

	bool WindowsVersion::Is11() { return IsWindows11OrGreater(); }
	bool WindowsVersion::Is11Build22000() { return IsWindows10BuildOrGreater(22000); }
	bool WindowsVersion::Is11Build22621() { return IsWindows10BuildOrGreater(22621); }

	struct VersionStringBuffer
	{
		std::array<char, 32> buffer{};
		std::string_view view{ "0.0.0.0" };
	};

	static bool GetFileVersionString(std::wstring_view file, VersionStringBuffer& sbuffer)
	{
		std::array<WCHAR, (MAX_PATH + 1)> windir{};
		UINT const windir_len = GetWindowsDirectoryW(windir.data(), MAX_PATH);
		assert(windir_len > 0);
		if (windir_len > 0)
		{
			std::wstring sys32dir;
			sys32dir.append(windir.data(), windir_len);
			sys32dir.append(std::wstring_view(L"\\System32\\"));
			sys32dir.append(file);

			// get file metadata size
			DWORD const metadata_size = GetFileVersionInfoSizeW(sys32dir.c_str(), NULL);
			assert(metadata_size > 0);
			if (metadata_size > 0)
			{
				// get file metadata
				std::vector<BYTE> metadata(metadata_size);
				BOOL const status = GetFileVersionInfoW(sys32dir.c_str(), 0, metadata_size, metadata.data());
				assert(status);
				if (status)
				{
					// parser file metadata
					VS_FIXEDFILEINFO* info = NULL;
					UINT read = 0;
					BOOL const query_status = VerQueryValueW(metadata.data(), L"\\", (LPVOID*)&info, &read);
					assert(query_status);
					if (query_status)
					{
						// format version
						int length = std::snprintf(
							sbuffer.buffer.data(),
							sbuffer.buffer.size(),
							"%d.%d.%d.%d",
							(info->dwProductVersionMS >> 16) & 0xFFFF,
							info->dwProductVersionMS & 0xFFFF,
							(info->dwProductVersionLS >> 16) & 0xFFFF,
							info->dwProductVersionLS & 0xFFFF);
						sbuffer.view = std::string_view(
							sbuffer.buffer.data(),
							(size_t)length);
						return true;
					}
				}
			}
		}
		return false;
	}

	std::string_view WindowsVersion::GetKernelVersionString()
	{
		VersionStringBuffer version_ntoskrnl_exe;
		VersionStringBuffer version_kernel32_dll;
		GetFileVersionString(L"ntoskrnl.exe", version_ntoskrnl_exe);
		GetFileVersionString(L"kernel32.dll", version_kernel32_dll);
		static std::string buf;
		buf.reserve(13 + 23 + 10 + 23);
		buf.clear();
		buf.append("NT OS Kernel ");
		buf.append(version_ntoskrnl_exe.view);
		buf.append(" Kernel32 ");
		buf.append(version_kernel32_dll.view);
		return buf;
	}
}
