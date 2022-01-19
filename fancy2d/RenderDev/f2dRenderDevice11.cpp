#include "RenderDev/f2dRenderDevice11.h"

#include <fcyMisc/fcyStringHelper.h>
#include <fcyOS/fcyDebug.h>

#include "RenderDev/f2dTexture11.h"

#include "Engine/f2dEngineImpl.h"
#include <VersionHelpers.h>

VERSIONHELPERAPI
static IsWindows11OrGreater()
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_BUILDNUMBER, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN10);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN10);
	osvi.dwBuildNumber = 22000;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

VERSIONHELPERAPI
static IsWindows10VersionOrGreater(DWORD build)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_BUILDNUMBER, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN10);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN10);
	osvi.dwBuildNumber = build;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

static std::string bytes_count_to_string(DWORDLONG size)
{
	int count = 0;
	char buffer[64] = {};
	if (size < 1024) // B
	{
		count = std::snprintf(buffer, 64, "%u B", (unsigned int)size);
	}
	else if (size < (1024 * 1024)) // KB
	{
		count = std::snprintf(buffer, 64, "%.2f KB", (double)size / 1024.0);
	}
	else if (size < (1024 * 1024 * 1024)) // MB
	{
		count = std::snprintf(buffer, 64, "%.2f MB", (double)size / 1048576.0);
	}
	else // GB
	{
		count = std::snprintf(buffer, 64, "%.2f GB", (double)size / 1073741824.0);
	}
	return std::string(buffer, count);
}

static char const* windows_version_to_string()
{
	if (IsWindowsServer())
	{
		return "Windows Server（如果此程序能运行，也是一件美事）";
	}

	// Windows 11

	if (IsWindows10VersionOrGreater(22001))
	{
		return "Windows 11 21H2+";
	}
	if (IsWindows10VersionOrGreater(22000))
	{
		return "Windows 11 21H2";
	}
	if (IsWindows11OrGreater())
	{
		return "Windows 11";
	}

	// Windows 10

	if (IsWindows10VersionOrGreater(19045))
	{
		return "Windows 10 21H2+";
	}
	if (IsWindows10VersionOrGreater(19044))
	{
		return "Windows 10 21H2";
	}
	if (IsWindows10VersionOrGreater(19043))
	{
		return "Windows 10 21H1";
	}
	if (IsWindows10VersionOrGreater(19042))
	{
		return "Windows 10 20H2";
	}
	if (IsWindows10VersionOrGreater(19041))
	{
		return "Windows 10 2004（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10VersionOrGreater(18363))
	{
		return "Windows 10 1909";
	}
	if (IsWindows10VersionOrGreater(18362))
	{
		return "Windows 10 1903（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10VersionOrGreater(17763))
	{
		return "Windows 10 1809";
	}
	if (IsWindows10VersionOrGreater(17134))
	{
		return "Windows 10 1803（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10VersionOrGreater(16299))
	{
		return "Windows 10 1709（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10VersionOrGreater(15063))
	{
		return "Windows 10 1703（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10VersionOrGreater(14393))
	{
		return "Windows 10 1607";
	}
	if (IsWindows10VersionOrGreater(10586))
	{
		return "Windows 10 1511（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10VersionOrGreater(10240))
	{
		return "Windows 10 1507 (RTM)（不受微软支持的版本，此程序仍然可以运行，可以考虑升级到 Windows 10 更新的版本）";
	}
	if (IsWindows10OrGreater())
	{
		return "Windows 10 DEV（不受微软支持的早期开发版本，此程序可能无法正常运行，可以考虑升级到 Windows 10 更新的版本）";
	}

	// Windows 8

	if (IsWindows8Point1OrGreater())
	{
		return "Windows 8.1";
	}
	if (IsWindows8OrGreater())
	{
		return "Windows 8（不受微软支持的版本，此程序仍然可以运行，可以考虑升级 Windows 8.1）";
	}

	// Windows 7

	if (IsWindows7SP1OrGreater())
	{
		BOOL b7 = FALSE;
		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
		if (SUCCEEDED(CreateDXGIFactory1(IID_IDXGIFactory1, &dxgi_factory)))
		{
			Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
			if (SUCCEEDED(dxgi_factory.As(&dxgi_factory2)))
			{
				b7 = TRUE;
			}
		}
		if (b7)
		{
			return "Windows 7 SP1 With Platform Update（不受微软支持的版本，此程序仍然可以运行）";
		}
		else
		{
			return "Windows 7 SP1（不受微软支持的版本，此程序可运行的最低要求版本，建议安装 Platform Update for Windows 7，即 KB2670838：https://www.microsoft.com/en-us/download/details.aspx?id=36805）";
		}
	}
	if (IsWindows7OrGreater())
	{
		return "Windows 7（不受微软支持的版本，不符合此程序可运行的最低要求版本，程序可能无法正常运行，可以考虑升级 Windows 7 SP1）";
	}

	// Windows ?

	return "Windows ?（如果此程序能运行，也是一件美事）";
}
static char const* d3d_feature_level_to_string(bool b, D3D_FEATURE_LEVEL level)
{
	if (b)
	{
		switch (level)
		{
		case D3D_FEATURE_LEVEL_12_1: return "功能级别 12.1";
		case D3D_FEATURE_LEVEL_12_0: return "功能级别 12.0";
		case D3D_FEATURE_LEVEL_11_1: return "功能级别 11.1";
		case D3D_FEATURE_LEVEL_11_0: return "功能级别 11.0";
		case D3D_FEATURE_LEVEL_10_1: return "功能级别 10.1";
		case D3D_FEATURE_LEVEL_10_0: return "功能级别 10.0";
		default: return "功能级别 ??.?（如果你遇到这种情况请告诉我）";
		}
	}
	else
	{
		return "不支持，或功能级别过低";
	}
}
static char const* d3d_feature_level_to_string2(D3D_FEATURE_LEVEL level)
{
	switch (level)
	{
	case 0xc200: return "12.2";
	case D3D_FEATURE_LEVEL_12_1: return "12.1";
	case D3D_FEATURE_LEVEL_12_0: return "12.0";
	case D3D_FEATURE_LEVEL_11_1: return "11.1";
	case D3D_FEATURE_LEVEL_11_0: return "11.0";
	case D3D_FEATURE_LEVEL_10_1: return "10.1";
	default:
	case D3D_FEATURE_LEVEL_10_0: return "10.0";
	}
}

#ifdef max
#undef max // FUCK YOU!
#endif

#include "spdlog/spdlog.h"

static void get_system_memory_status()
{
	MEMORYSTATUSEX info = { sizeof(MEMORYSTATUSEX) };
	if (GlobalMemoryStatusEx(&info))
	{
		spdlog::info("[fancy2d] 系统内存使用情况：\n"
			"    使用百分比：{}%\n"
			"    总物理内存：{}\n"
			"    剩余物理内存：{}\n"
			"    当前进程可提交内存限制：{}\n"
			"    当前进程剩余的可提交内存：{}\n"
			"    当前进程用户模式内存空间限制*1：{}\n"
			"    当前进程剩余的用户模式内存空间：{}\n"
			"        *1 此项反映此程序实际上能用的最大内存，在 32 位应用程序上此项一般为 2 GB，修改 Windows 操作系统注册表后可能为 1 到 3 GB"
			, info.dwMemoryLoad
			, bytes_count_to_string(info.ullTotalPhys)
			, bytes_count_to_string(info.ullAvailPhys)
			, bytes_count_to_string(info.ullTotalPageFile)
			, bytes_count_to_string(info.ullAvailPageFile)
			, bytes_count_to_string(info.ullTotalVirtual)
			, bytes_count_to_string(info.ullAvailVirtual)
		);
	}
	else
	{
		spdlog::error("[fancy2d] 无法获取系统内存使用情况");
	}
}

// 类主体

bool f2dRenderDevice11::selectAdapter()
{
	HRESULT hr = 0;

	// 公共参数

	UINT d3d11_creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	d3d11_creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL const target_levels_downlevel[3] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
	};
	D3D_FEATURE_LEVEL const target_levels[4] = {
		D3D_FEATURE_LEVEL_11_1, // Windows 7 没有这个
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	// 枚举所有图形设备

	spdlog::info("[fancy2d] 枚举所有图形设备");
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter_temp;
	bool link_to_output = false;
	for (UINT idx = 0; bHR = dxgi_factory->EnumAdapters1(idx, &dxgi_adapter_temp); idx += 1)
	{
		// 检查此设备是否支持 Direct3D 11 并获取特性级别
		bool supported_d3d11 = false;
		D3D_FEATURE_LEVEL level_info = D3D_FEATURE_LEVEL_10_0;
		hr = gHR = D3D11CreateDevice(dxgi_adapter_temp.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, d3d11_creation_flags, target_levels, 4, D3D11_SDK_VERSION, NULL, &level_info, NULL);
		if (FAILED(hr))
		{
			// 处理 Windows 7 的情况
			hr = gHR = D3D11CreateDevice(dxgi_adapter_temp.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, d3d11_creation_flags, target_levels_downlevel, 3, D3D11_SDK_VERSION, NULL, &level_info, NULL);
		}
		if (SUCCEEDED(hr))
		{
			supported_d3d11 = true;
		}
		// 获取图形设备信息
		std::string dev_name = "<NULL>";
		auto adapter_flags_to_string = [](UINT flags) -> char const*
		{
			if ((flags & DXGI_ADAPTER_FLAG_REMOTE) && (flags & DXGI_ADAPTER_FLAG_SOFTWARE))
			{
				return "远程软件设备";
			}
			else if (flags & DXGI_ADAPTER_FLAG_REMOTE)
			{
				return "远程硬件设备";
			}
			else if (flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				return "软件设备";
			}
			else
			{
				return "硬件设备";
			}
		};
		DXGI_ADAPTER_DESC1 desc_ = {};
		if (bHR = gHR = dxgi_adapter_temp->GetDesc1(&desc_))
		{
			bool soft_dev_type = (desc_.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) || (desc_.Flags & DXGI_ADAPTER_FLAG_REMOTE);
			dev_name = fcyStringHelper::WideCharToMultiByte(desc_.Description);
			spdlog::info("[fancy2d] 图形设备[{}]：\n"
				"    设备名称：{}\n"
				"    设备类型：{}\n"
				"    Direct3D 11 支持情况：{}{}\n"
				"    专用显存：{}\n"
				"    专用内存：{}\n"
				"    共享内存：{}\n"
				"    供应商 ID：0x{:08X}\n"
				"    设备 ID：0x{:08X}\n"
				"    子系统 ID：0x{:08X}\n"
				"    修订号：0x{:08X}\n"
				"    设备 LUID：{:08X}-{:08X}"
				, idx
				, dev_name
				, adapter_flags_to_string(desc_.Flags)
				, d3d_feature_level_to_string(supported_d3d11, level_info), soft_dev_type ? "（注：软件或远程设备性能不足以运行游戏，仅供开发使用）" : ""
				, bytes_count_to_string(desc_.DedicatedVideoMemory)
				, bytes_count_to_string(desc_.DedicatedSystemMemory)
				, bytes_count_to_string(desc_.SharedSystemMemory)
				, desc_.VendorId
				, desc_.DeviceId
				, desc_.SubSysId
				, desc_.Revision
				, static_cast<DWORD>(desc_.AdapterLuid.HighPart), desc_.AdapterLuid.LowPart
			);
			if (soft_dev_type)
			{
				supported_d3d11 = false; // 排除软件或远程设备
			}
		}
		else
		{
			spdlog::error("[fancy2d] IDXGIAdapter1::GetDesc1 调用失败，无法获取图形设备的信息");
			spdlog::info("[fancy2d] 图形设备[{}]：*无法读取设备信息", idx);
		}
		// 枚举显示输出
		bool has_linked_output = false;
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output_temp;
		for (UINT odx = 0; bHR = dxgi_adapter_temp->EnumOutputs(odx, &dxgi_output_temp); odx += 1)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput6> dxgi_output_temp6;
			dxgi_output_temp.As(&dxgi_output_temp6);
			DXGI_OUTPUT_DESC1 o_desc = {};
			bool read_o_desc = false;
			UINT comp_sp_flags = 0;
			if (dxgi_output_temp6)
			{
				if (!(bHR = gHR = dxgi_output_temp6->CheckHardwareCompositionSupport(&comp_sp_flags)))
				{
					comp_sp_flags = 0;
					spdlog::error("[fancy2d] IDXGIOutput6::CheckHardwareCompositionSupport 调用失败");
				}
				if (bHR = gHR = dxgi_output_temp6->GetDesc1(&o_desc))
				{
					read_o_desc = true;
				}
				else
				{
					spdlog::error("[fancy2d] IDXGIOutput6::GetDesc1 调用失败");
				}
			}
			if (!read_o_desc)
			{
				DXGI_OUTPUT_DESC desc_ = {};
				if (bHR = gHR = dxgi_output_temp->GetDesc(&desc_))
				{
					std::memcpy(o_desc.DeviceName, desc_.DeviceName, sizeof(o_desc.DeviceName));
					o_desc.DesktopCoordinates = desc_.DesktopCoordinates;
					o_desc.AttachedToDesktop = desc_.AttachedToDesktop;
					o_desc.Rotation = desc_.Rotation;
					o_desc.Monitor = desc_.Monitor;
					read_o_desc = true;
				}
			}
			if (read_o_desc)
			{
				auto comp_flags_to_string = [](UINT flags) -> std::string
				{
					std::string buffer;
					if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN)
					{
						buffer += "全屏";
					}
					if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)
					{
						if (!buffer.empty())
						{
							buffer += "、";
						}
						buffer += "窗口";
					}
					if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED)
					{
						if (!buffer.empty())
						{
							buffer += "、";
						}
						buffer += "鼠标指针缩放";
					}
					if (!buffer.empty())
					{
						return buffer;
					}
					return "不支持";
				};
				auto rotate_to_string = [](DXGI_MODE_ROTATION rot) -> char const*
				{
					switch (rot)
					{
					default:
					case DXGI_MODE_ROTATION_UNSPECIFIED: return "未知";
					case DXGI_MODE_ROTATION_IDENTITY: return "无";
					case DXGI_MODE_ROTATION_ROTATE90: return "90 度";
					case DXGI_MODE_ROTATION_ROTATE180: return "180 度";
					case DXGI_MODE_ROTATION_ROTATE270: return "270 度";
					}
				};
				spdlog::info("[fancy2d] 图形设备[{}]关联的显示输出设备[{}]：\n"
					"    连接状态：{}\n"
					"    显示区域：({}, {}) ({} x {})\n"
					"    旋转状态：{}\n"
					"    硬件表面合成支持情况：{}"
					, idx, odx
					, o_desc.AttachedToDesktop ? "已连接" : "未连接"
					, o_desc.DesktopCoordinates.left, o_desc.DesktopCoordinates.top, o_desc.DesktopCoordinates.right - o_desc.DesktopCoordinates.left, o_desc.DesktopCoordinates.bottom - o_desc.DesktopCoordinates.top
					, rotate_to_string(o_desc.Rotation)
					, comp_flags_to_string(comp_sp_flags)
				);
				has_linked_output = true;
			}
			else
			{
				spdlog::error("[fancy2d] IDXGIOutput::GetDesc 调用失败，无法获取显示输出设备的信息");
				spdlog::info("[fancy2d] 显示输出设备[{}]：*无法读取设备信息", odx);
			}
		}
		dxgi_output_temp.Reset();
		// 选择这个设备
		if (supported_d3d11 && !dxgi_adapter)
		{
			dxgi_adapter = dxgi_adapter_temp;
			m_DevName = dev_name;
			if (has_linked_output)
			{
				link_to_output = true;
			}
		}
	}
	dxgi_adapter_temp.Reset();

	// 获取图形设备

	if (dxgi_adapter)
	{
		spdlog::info("[fancy2d] 已选择图形设备：{}", m_DevName);
		if (!link_to_output)
		{
			spdlog::warn("[fancy2d] 选择的图形设备：{}，似乎没有连接到任何显示输出，这可能导致独占全屏时通过 PCI-E 复制显示画面或者 Desktop Window Manager（DWM，桌面窗口管理器）介入进行桌面合成，性能可能会下降", m_DevName);
		}
	}
	else
	{
		spdlog::critical("[fancy2d] 没有可用的图形设备");
		spdlog::info("[fancy2d] 可能导致没有可用的图形设备的情况：\n"
			"    1、硬件过于老旧\n"
			"    2、在虚拟机中运行此程序*1\n"
			"    3、驱动程序未安装或不是最新\n"
			"    4、图形设备未正常连接\n"
			"    5、其他意外情况*2\n"
			"        *1 除非已安装虚拟机软件提供的图形驱动程序\n"
			"        *2 比如 Windows 系统玄学 Bug\n"
		);
		return false;
	}

	return true;
}
bool f2dRenderDevice11::checkFeatureSupported()
{
	auto mt_support_to_string = [](D3D11_FEATURE_DATA_THREADING v) -> std::string
	{
		std::string buf;
		if (v.DriverConcurrentCreates)
		{
			buf += "异步资源创建";
		}
		if (v.DriverCommandLists)
		{
			if (!buf.empty())
			{
				buf += "、";
			}
			buf += "多线程命令队列";
		}
		if (buf.empty())
		{
			return buf;
		}
		return "不支持";
	};

	D3D11_FEATURE_DATA_THREADING d3d11_feature_mt = {};
	HRESULT hr_mt = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &d3d11_feature_mt, sizeof(d3d11_feature_mt));

	D3D11_FEATURE_DATA_FORMAT_SUPPORT d3d11_feature_format_rgba32 = { .InFormat = DXGI_FORMAT_R8G8B8A8_UNORM };
	HRESULT hr_fmt_rgba32 = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &d3d11_feature_format_rgba32, sizeof(d3d11_feature_format_rgba32));
	D3D11_FEATURE_DATA_FORMAT_SUPPORT d3d11_feature_format_rgba32_srgb = { .InFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	HRESULT hr_fmt_rgba32_srgb = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &d3d11_feature_format_rgba32_srgb, sizeof(d3d11_feature_format_rgba32_srgb));
	D3D11_FEATURE_DATA_FORMAT_SUPPORT  d3d11_feature_format_bgra32 = { .InFormat = DXGI_FORMAT_B8G8R8A8_UNORM };
	HRESULT hr_fmt_bgra32 = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &d3d11_feature_format_bgra32, sizeof(d3d11_feature_format_bgra32));
	D3D11_FEATURE_DATA_FORMAT_SUPPORT d3d11_feature_format_bgra32_srgb = { .InFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB };
	HRESULT hr_fmt_bgra32_srgb = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &d3d11_feature_format_bgra32_srgb, sizeof(d3d11_feature_format_bgra32_srgb));
	D3D11_FEATURE_DATA_FORMAT_SUPPORT d3d11_feature_format_d24_s8 = { .InFormat = DXGI_FORMAT_D24_UNORM_S8_UINT };
	HRESULT hr_fmt_d24_s8 = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &d3d11_feature_format_d24_s8, sizeof(d3d11_feature_format_d24_s8));

	D3D11_FEATURE_DATA_ARCHITECTURE_INFO d3d11_feature_arch = {};
	HRESULT hr_arch = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_ARCHITECTURE_INFO, &d3d11_feature_arch, sizeof(d3d11_feature_arch));

	D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11_feature_o2 = {};
	HRESULT hr_o2 = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &d3d11_feature_o2, sizeof(d3d11_feature_o2));

#define _FORMAT_INFO_STRING_FMT \
		"        创建二维纹理：{}\n"\
		"        用于顶点颜色：{}\n"\
		"        多级渐进纹理：{}\n"\
		"        自动生成多级渐进纹理：{}\n"\
		"        绑定为渲染目标：{}\n"\
		"        参与像素颜色混合：{}\n"\
		"        作为显示输出：{}\n"

#define _FORMAT_INFO_STRING_FMT2 \
		"        创建二维纹理：{}\n"\
		"        多级渐进纹理：{}\n"\
		"        自动生成多级渐进纹理：{}\n"\
		"        绑定为渲染目标：{}\n"\
		"        参与像素颜色混合：{}\n"\
		"        作为显示输出：{}\n"

#define _FORMAT_INFO_STRING_ARG(x) \
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_BLENDABLE) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_DISPLAY) ? "支持" : "不支持"

#define _FORMAT_INFO_STRING_ARG2(x) \
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_BLENDABLE) ? "支持" : "不支持"\
		, (x.OutFormatSupport & D3D11_FORMAT_SUPPORT_DISPLAY) ? "支持" : "不支持"

	spdlog::info("[fancy2d] Direct3D 11 设备功能支持：\n"
		"    功能级别：{}\n"
		"    R8G8B8A8 格式：\n"
		_FORMAT_INFO_STRING_FMT
		"    R8G8B8A8 sRGB 格式：\n"
		_FORMAT_INFO_STRING_FMT2
		"    B8G8R8A8 格式：\n"
		_FORMAT_INFO_STRING_FMT
		"    B8G8R8A8 sRGB 格式：\n"
		_FORMAT_INFO_STRING_FMT2
		"    D24 S8 格式：\n"
		"        创建二维纹理：{}\n"\
		"        绑定为深度、模板缓冲区：{}\n"\
		"    最大二维纹理尺寸：{}\n"
		"    多线程架构：{}\n"
		"    渲染架构：{}\n"
		"    统一内存架构（UMA）：{}"
		, d3d_feature_level_to_string2(d3d11_level)
		_FORMAT_INFO_STRING_ARG(d3d11_feature_format_rgba32)
		_FORMAT_INFO_STRING_ARG2(d3d11_feature_format_rgba32_srgb)
		_FORMAT_INFO_STRING_ARG(d3d11_feature_format_bgra32)
		_FORMAT_INFO_STRING_ARG2(d3d11_feature_format_bgra32_srgb)
		, (d3d11_feature_format_d24_s8.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? "支持" : "不支持"\
		, (d3d11_feature_format_d24_s8.OutFormatSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) ? "支持" : "不支持"\
		, (d3d11_level >= D3D_FEATURE_LEVEL_11_0) ? "16384 x 16384" : "8192 x 8192"
		, mt_support_to_string(d3d11_feature_mt)
		, (SUCCEEDED(hr_arch) && d3d11_feature_arch.TileBasedDeferredRenderer) ? "Tile Based Deferred Renderer（TBDR）架构" : "未知，桌面平台一般为 Immediate Mode Rendering（IMR）架构"
		, (SUCCEEDED(hr_o2) && d3d11_feature_o2.UnifiedMemoryArchitecture) ? "支持" : "不支持"
	);

#undef _FORMAT_INFO_STRING_FMT
#undef _FORMAT_INFO_STRING_ARG

	if (
		(d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D)
		&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER)
		&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP)
		&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET)
		&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_BLENDABLE)
		&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_DISPLAY)
		)
	{
		d3d11_support_bgra = true;
	}
	else
	{
		spdlog::warn("[fancy2d] 此设备没有完整的 B8G8R8A8 格式支持，程序可能无法正常运行");
	}

	return d3d11_support_bgra;
}
f2dRenderDevice11::f2dRenderDevice11(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)
	: m_pEngine(pEngine)
	, m_CreateThreadID(GetCurrentThreadId())
	, swapchain_width(BackBufferWidth)
	, swapchain_height(BackBufferHeight)
	, swapchain_windowed(false) // 必须以窗口模式启动
	, swapchain_vsync(VSync)
{
	m_hWnd = (HWND)pEngine->GetMainWindow()->GetHandle();
	win32_window = m_hWnd;
	HRESULT hr = 0;

	spdlog::info("[fancy2d] 操作系统版本：{}", windows_version_to_string());
	get_system_memory_status();
	spdlog::info("[fancy2d] 开始创建图形组件");

	spdlog::info("[fancy2d] 开始创建基本 DXGI 组件");

	// 创建 DXGI 工厂

	hr = gHR = CreateDXGIFactory1(IID_IDXGIFactory1, &dxgi_factory);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] CreateDXGIFactory1 调用失败");
		throw fcyWin32COMException("f2dRenderDevice11::f2dRenderDevice11", "CreateDXGIFactory1 failed.", hr);
	}
	{
		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi_factory5;
		hr = gHR = dxgi_factory.As(&dxgi_factory5);
		if (SUCCEEDED(hr))
		{
			BOOL bs = FALSE;
			hr = gHR = dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bs, sizeof(bs));
			if (SUCCEEDED(hr))
			{
				if (IsWindows10OrGreater())
				{
					if (bs)
					{
						dxgi_support_tearing = true;
					}
				}
			}
		}
		spdlog::info("[fancy2d] DXGI 组件功能支持："
			"    呈现时允许画面撕裂：{}"
			, dxgi_support_tearing ? "支持" : "不支持"
		);
	}
	
	// 公共参数

	UINT d3d11_creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	d3d11_creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL const target_levels_downlevel[3] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
	};
	D3D_FEATURE_LEVEL const target_levels[4] = {
		D3D_FEATURE_LEVEL_11_1, // Windows 7 没有这个
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	// 获取图形设备

	if (!selectAdapter())
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::selectAdapter failed, no adapter avalid.");
	}
	
	spdlog::info("[fancy2d] 已创建基本 DXGI 组件：DXGI Factory + 图形设备");

	// 创建 D3D11 组件

	spdlog::info("[fancy2d] 开始创建基本 Direct3D 11 组件");
	hr = gHR = D3D11CreateDevice(dxgi_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, d3d11_creation_flags, target_levels, 4, D3D11_SDK_VERSION, &d3d11_device, &d3d11_level, &d3d11_devctx);
	if (FAILED(hr))
	{
		hr = gHR = D3D11CreateDevice(dxgi_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, d3d11_creation_flags, target_levels_downlevel, 3, D3D11_SDK_VERSION, &d3d11_device, &d3d11_level, &d3d11_devctx);
	}
	if (FAILED(hr))
	{
		spdlog::info("[fancy2d] D3D11CreateDevice 调用失败");
		throw fcyWin32COMException("f2dRenderDevice11::f2dRenderDevice11", "D3D11CreateDevice failed.", hr);
	}
	spdlog::info("[fancy2d] 已创建基本 Direct3D 11 组件：Direct3D 11 设备");

	// 检查各种特性

	checkFeatureSupported();

	// 交换链

	if (!createSwapchain(nullptr))
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::createSwapchain failed.");
	}
	GetSupportedDisplayModeCount(true);

	spdlog::info("[fancy2d] 已创建图形组件（图形 API：DXGI + Direct3D 11）");

	m_pEngine->GetMainWindow()->SetGraphicListener(this);
}
f2dRenderDevice11::~f2dRenderDevice11()
{
	if (m_pEngine->GetMainWindow())
		m_pEngine->GetMainWindow()->SetGraphicListener(nullptr);

	beforeDestroyDevice(); // 一定要先退出独占全屏

	// 删除渲染器监听链
	for (auto& v : _setEventListeners)
	{
		// 报告可能的对象泄漏
		char tTextBuffer[256];
		sprintf_s(tTextBuffer, "Unrelease listener object at %p", v.listener);
#ifdef _DEBUG
		fcyDebug::Trace("[@f2dRenderDevice11::~f2dRenderDevice11] %s\n", tTextBuffer);
#endif
		m_pEngine->ThrowException(fcyException("f2dRenderDevice11::~f2dRenderDevice11", tTextBuffer));
	}
	_setEventListeners.clear();
}

void* f2dRenderDevice11::GetHandle() { return d3d11_device.Get(); }
fcStr f2dRenderDevice11::GetDeviceName() { return m_DevName.c_str(); }

// 设备丢失与恢复

int f2dRenderDevice11::sendDevLostMsg()
{
	int tRet = 0;

	// 发送丢失消息
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderDeviceLost();
		tRet += 1;
	}

	return tRet;
}
int f2dRenderDevice11::sendDevResetMsg()
{
	int tRet = 0;

	// 发送重置消息
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderDeviceReset();
		tRet += 1;
	}

	return tRet;
}
int f2dRenderDevice11::dispatchRenderSizeDependentResourcesCreate()
{
	int cnt = 0;
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesCreate();
		cnt += 1;
	}
	return cnt;
}
int f2dRenderDevice11::dispatchRenderSizeDependentResourcesDestroy()
{
	int cnt = 0;
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesDestroy();
		cnt += 1;
	}
	return cnt;
}

fResult f2dRenderDevice11::AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority)
{
	if (Listener == NULL)
		return FCYERR_INVAILDPARAM;

	EventListenerNode node;
	node.uuid = _iEventListenerUUID;
	node.priority = Priority;
	node.listener = Listener;
	auto v = _setEventListeners.emplace(node);
	_iEventListenerUUID += 1;

	return FCYERR_OK;
}
fResult f2dRenderDevice11::RemoveListener(f2dRenderDeviceEventListener* Listener)
{
	int ifind = 0;
	for (auto it = _setEventListeners.begin(); it != _setEventListeners.end();)
	{
		if (it->listener == Listener)
		{
			it = _setEventListeners.erase(it);
			ifind += 1;
		}
		else
		{
			it++;
		}
	}

	return ifind > 0 ? FCYERR_OK : FCYERR_OBJNOTEXSIT;
}
fResult f2dRenderDevice11::SyncDevice()
{
	// 需要重置设备
	if (m_bDevLost)
	{
		m_bDevLost = false;
		int tObjCount = sendDevResetMsg(); // 通知非托管组件恢复工作
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "device reset (%d object(s) reset)", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDevice11::SyncDevice", tBuffer));
		return FCYERR_OK;
	}
	// 要退出全屏
	if (swapchain_want_exit_fullscreen)
	{
		swapchain_want_exit_fullscreen = false;
		if (!swapchain_windowed && dxgi_swapchain)
		{
			BOOL bFSC = FALSE;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFSC, &dxgi_output);
			if (bFSC)
			{
				spdlog::info("[fancy2d] 尝试退出独占全屏");
				hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
				if (FAILED(hr))
				{
					spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
				}
			}
		}
	}
	else
	{
		// 试着重新进入全屏
		if (swapchain_want_enter_fullscreen)
		{
			swapchain_want_enter_fullscreen = false;
			if (!swapchain_windowed && dxgi_swapchain)
			{
				BOOL bFSC = FALSE;
				Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
				HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFSC, &dxgi_output);
				if (FAILED(hr) || !bFSC)
				{
					spdlog::info("[fancy2d] 尝试切换到独占全屏");
					hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
					if (FAILED(hr))
					{
						spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #TRUE 调用失败，无法进入独占全屏");
					}
				}
			}
		}
	}
	// 需要重新调整交换链大小
	if (swapchain_want_resize)
	{
		swapchain_want_resize = false;
		destroyRenderAttachments();
		if (dxgi_swapchain)
		{
			HRESULT hr = gHR = dxgi_swapchain->ResizeBuffers(swapchain_resize_data.BufferCount, swapchain_width, swapchain_height, swapchain_resize_data.Format, swapchain_resize_data.Flags);
			if (FAILED(hr))
			{
				return FCYERR_INTERNALERR;
			}
		}
		if (!createRenderAttachments())
		{
			return FCYERR_INTERNALERR;
		}
	}
	// 小 Hack，在这里绑定交换链的 RenderTarget
	setupRenderAttachments();
	return FCYERR_OK;
}
void f2dRenderDevice11::OnLostFocus()
{
	swapchain_want_exit_fullscreen = true;
}
void f2dRenderDevice11::OnGetFocus()
{
	swapchain_want_enter_fullscreen = true;
}
void f2dRenderDevice11::OnSize(fuInt ClientWidth, fuInt ClientHeight)
{
	swapchain_want_resize = true;
}

// 创建资源

fResult f2dRenderDevice11::CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2D11(this, pStream, HasMipmap, IsDynamic);
	}
	catch (const std::bad_alloc&)
	{
		return FCYERR_OUTOFMEM;
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2D11(this, pMemory, Size, HasMipmap, IsDynamic);
	}
	catch (const std::bad_alloc&)
	{
		return FCYERR_OUTOFMEM;
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2D11(this, Width, Height);
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dRenderTarget11(this, Width, Height, AutoResize);
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dDepthStencil11(this, Width, Height, AutoResize);
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

// 渲染附件

fResult f2dRenderDevice11::Clear(const fcyColor& BackBufferColor, fFloat ZValue)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (rtv)
		{
			FLOAT clear_color[4] = {
				(float)BackBufferColor.r / 255.0f,
				(float)BackBufferColor.g / 255.0f,
				(float)BackBufferColor.b / 255.0f,
				(float)BackBufferColor.a / 255.0f,
			};
			d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		}
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, ZValue, 0);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::Clear(const fcyColor& BackBufferColor, fFloat ZValue, fuInt StencilValue)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (rtv)
		{
			FLOAT clear_color[4] = {
				(float)BackBufferColor.r / 255.0f,
				(float)BackBufferColor.g / 255.0f,
				(float)BackBufferColor.b / 255.0f,
				(float)BackBufferColor.a / 255.0f,
			};
			d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		}
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, ZValue, (UINT8)StencilValue);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearColor(const fcyColor& BackBufferColor)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		if (rtv)
		{
			FLOAT clear_color[4] = {
				(float)BackBufferColor.r / 255.0f,
				(float)BackBufferColor.g / 255.0f,
				(float)BackBufferColor.b / 255.0f,
				(float)BackBufferColor.a / 255.0f,
			};
			d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearZBuffer(fFloat Value)
{
	if (d3d11_devctx)
	{
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, Value, 0);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearStencilBuffer(fuInt StencilValue)
{
	if (d3d11_devctx)
	{
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_STENCIL, 0.0f, (UINT8)StencilValue);
		}
	}
	return FCYERR_OK;
}

f2dTexture2D* f2dRenderDevice11::GetRenderTarget()
{
	return *m_RenderTarget;
}
fResult f2dRenderDevice11::SetRenderTarget(f2dTexture2D* pTex)
{
	if (pTex && !pTex->IsRenderTarget())
		return FCYERR_INVAILDPARAM;

	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	if (*m_RenderTarget == pTex)
		return FCYERR_OK;

	m_RenderTarget = pTex; // 注意 pTex 可能是 NULL 代表需要重置为交换链的 RenderTarget

	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		d3d11_devctx->OMSetRenderTargets(1, &rtv, dsv);
	}

	return FCYERR_OK;
}
f2dDepthStencilSurface* f2dRenderDevice11::GetDepthStencilSurface()
{
	return *m_DepthStencil;
}
fResult f2dRenderDevice11::SetDepthStencilSurface(f2dDepthStencilSurface* pSurface)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	if (*m_DepthStencil == pSurface)
		return FCYERR_OK;

	m_DepthStencil = pSurface; // 注意 pSurface 可能是 NULL 代表需要重置为默认的 DepthStencil

	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		d3d11_devctx->OMSetRenderTargets(1, &rtv, dsv);
	}

	return FCYERR_OK;
}

// 状态设置

f2dGraphics* f2dRenderDevice11::QueryCurGraphics() { return m_pCurGraphics; }
fResult f2dRenderDevice11::SubmitCurGraphics(f2dGraphics* pGraph, bool bDirty)
{
	if (pGraph == NULL)
	{
		m_pCurGraphics = NULL;
		return FCYERR_OK;
	}
	else if (m_pCurGraphics)
	{
		if (m_pCurGraphics->IsInRender())
			return FCYERR_ILLEGAL;
	}

	if (!bDirty && pGraph == m_pCurGraphics)
		return FCYERR_OK;

	m_pCurGraphics = pGraph;

	return FCYERR_OK;
}
fcyRect f2dRenderDevice11::GetScissorRect()
{
	if (!d3d11_devctx)
	{
		return fcyRect();
	}
	UINT vpc = 0;
	D3D11_RECT vpv[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
	d3d11_devctx->RSGetScissorRects(&vpc, vpv);
	return fcyRect(
		(fFloat)vpv[0].left,
		(fFloat)vpv[0].top,
		(fFloat)vpv[0].right,
		(fFloat)vpv[0].bottom
	);
}
fResult f2dRenderDevice11::SetScissorRect(const fcyRect& pRect)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
	{
		m_pCurGraphics->Flush();
	}
	if (!d3d11_devctx)
	{
		return FCYERR_INTERNALERR;
	}
	D3D11_RECT d3d11_rc = {
		.left = (LONG)pRect.a.x,
		.top = (LONG)pRect.a.y,
		.right = (LONG)pRect.b.x,
		.bottom = (LONG)pRect.b.y,
	};
	d3d11_devctx->RSSetScissorRects(1, &d3d11_rc);
	return FCYERR_OK;
}
fcyRect f2dRenderDevice11::GetViewport()
{
	if (!d3d11_devctx)
	{
		return fcyRect();
	}
	UINT vpc = 0;
	D3D11_VIEWPORT vpv[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
	d3d11_devctx->RSGetViewports(&vpc, vpv);
	return fcyRect(
		vpv[0].TopLeftX,
		vpv[0].TopLeftY,
		vpv[0].TopLeftX + vpv[0].Width,
		vpv[0].TopLeftY + vpv[0].Height
	);
}
fResult f2dRenderDevice11::SetViewport(fcyRect vp)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
	{
		m_pCurGraphics->Flush();
	}
	if (!d3d11_devctx)
	{
		return FCYERR_INTERNALERR;
	}
	D3D11_VIEWPORT d3d11_vp = {
		.TopLeftX = vp.a.x,
		.TopLeftY = vp.a.y,
		.Width = vp.GetWidth(),
		.Height = vp.GetHeight(),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
	d3d11_devctx->RSSetViewports(1, &d3d11_vp);
	return FCYERR_OK;
}

// 交换链

void f2dRenderDevice11::beforeDestroyDevice()
{
	if (d3d11_devctx)
	{
		d3d11_devctx->ClearState();
		d3d11_devctx->Flush();
	}
	if (dxgi_swapchain)
	{
		BOOL bFullscreen = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
		if (bFullscreen)
		{
			spdlog::info("[fancy2d] 尝试退出独占全屏");
			hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
			}
		}
	}
}
void f2dRenderDevice11::destroySwapchain()
{
	destroyRenderAttachments();
	if (dxgi_swapchain)
	{
		BOOL bFullscreen = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
		if (bFullscreen)
		{
			spdlog::info("[fancy2d] 尝试退出独占全屏");
			hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
			}
		}
	}
	dxgi_swapchain.Reset();
}
bool f2dRenderDevice11::createSwapchain(f2dDisplayMode* pmode)
{
	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	dxgi_factory.As(&dxgi_factory2);
	Microsoft::WRL::ComPtr<ID3D11Device1> d3d11_device1;
	d3d11_device.As(&d3d11_device1);
	
	spdlog::info("[fancy2d] 开始创建 SwapChain（交换链）组件");
	if (dxgi_factory2 && d3d11_device1)
	{
		// Windows 7 平台更新已安装，或者 Windows 8 及以上
		DXGI_SWAP_CHAIN_DESC1 desc1 = {
			.Width = swapchain_width,
			.Height = swapchain_height,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.Stereo = FALSE, // 永远用不上立体交换链
			.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1, .Quality = 0,},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.Scaling = DXGI_SCALING_STRETCH, // Windows 7 只支持这个
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD, // Windows 7 只支持这个
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE, // 永远用不上 A 通道
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC descf = {
			.RefreshRate = DXGI_RATIONAL{.Numerator = 0, .Denominator = 0,}, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Windowed = TRUE,
		};
		if (pmode)
		{
			desc1.Width = pmode->width;
			desc1.Height = pmode->height;
			descf.RefreshRate.Numerator = pmode->refresh_rate.numerator;
			descf.RefreshRate.Denominator = pmode->refresh_rate.denominator;
			descf.ScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)pmode->scanline_ordering;
			descf.Scaling = (DXGI_MODE_SCALING)pmode->scaling;
		}
		if (swapchain_windowed && swapchain_flip && dxgi_support_tearing)
		{
			// 只有在窗口模式下才允许开这个功能
			desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			swapchain_resize_data.AllowTearing = TRUE;
		}
		else
		{
			swapchain_resize_data.AllowTearing = FALSE;
		}
		swapchain_resize_data.BufferCount = desc1.BufferCount;
		swapchain_resize_data.Format = desc1.Format;
		swapchain_resize_data.Flags = desc1.Flags;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swapchain1;
		hr = gHR = dxgi_factory2->CreateSwapChainForHwnd(d3d11_device1.Get(), win32_window, &desc1, swapchain_windowed ? NULL : &descf, NULL, &dxgi_swapchain1);
		if (SUCCEEDED(hr))
		{
			hr = gHR = dxgi_swapchain1.As(&dxgi_swapchain);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGISwapChain1::QueryInterface -> #IDXGISwapChain 调用失败，创建 SwapChain（交换链）组件失败");
			}
		}
		else
		{
			spdlog::error("[fancy2d] IDXGIFactory2::CreateSwapChainForHwnd 调用失败，创建 SwapChain（交换链）组件失败");
		}
	}
	if (!dxgi_swapchain)
	{
		// 回落到 Windows 7 的方式创建
		DXGI_MODE_DESC mode = {
			.Width = swapchain_width,
			.Height = swapchain_height,
			.RefreshRate = DXGI_RATIONAL{.Numerator = 0, .Denominator = 0,}, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
		};
		if (pmode)
		{
			mode.Width = pmode->width;
			mode.Height = pmode->height;
			mode.RefreshRate.Numerator = pmode->refresh_rate.numerator;
			mode.RefreshRate.Denominator = pmode->refresh_rate.denominator;
			mode.ScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)pmode->scanline_ordering;
			mode.Scaling = (DXGI_MODE_SCALING)pmode->scaling;
		}
		DXGI_SWAP_CHAIN_DESC desc = {
			.BufferDesc = mode,
			.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1, .Quality = 0,},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.OutputWindow = win32_window,
			.Windowed = TRUE,
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD, // Windows 7 只支持这个
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		if (swapchain_windowed && swapchain_flip && dxgi_support_tearing)
		{
			// 只有在窗口模式下才允许开这个功能
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			swapchain_resize_data.AllowTearing = TRUE;
		}
		else
		{
			swapchain_resize_data.AllowTearing = FALSE;
		}
		swapchain_resize_data.BufferCount = desc.BufferCount;
		swapchain_resize_data.Format = mode.Format;
		swapchain_resize_data.Flags = desc.Flags;
		hr = gHR = dxgi_factory->CreateSwapChain(d3d11_device.Get(), &desc, &dxgi_swapchain);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIFactory1::CreateSwapChain 调用失败，创建 SwapChain（交换链）组件失败");
			return false;
		}
	}
	if (dxgi_swapchain)
	{
		spdlog::info("[fancy2d] 已创建 SwapChain（交换链）组件");
	}
	
	hr = gHR = dxgi_factory->MakeWindowAssociation(win32_window, DXGI_MWA_NO_ALT_ENTER); // 别他妈乱切换了
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IDXGIFactory1::MakeWindowAssociation 调用失败，无法关闭 DXGI 自带的 ALT+ENTER 切换全屏功能（该功能会导致画面显示异常）");
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device1;
	hr = gHR = d3d11_device.As(&dxgi_device1);
	if (SUCCEEDED(hr))
	{
		hr = gHR = dxgi_device1->SetMaximumFrameLatency(1);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIDevice1::SetMaximumFrameLatency -> #1 调用失败");
		}
	}

	if (!createRenderAttachments())
	{
		return false;
	}
	setupRenderAttachments();

	return true;
}
void f2dRenderDevice11::destroyRenderAttachments()
{
	if (d3d11_devctx)
	{
		d3d11_devctx->ClearState();
		d3d11_devctx->Flush();
	}
	d3d11_rendertarget.Reset();
	d3d11_depthstencil.Reset();
}
bool f2dRenderDevice11::createRenderAttachments()
{
	if (!dxgi_swapchain)
	{
		return false;
	}

	spdlog::info("[fancy2d] 开始创建 RenderAttachment（渲染附件）");

	HRESULT hr = 0;

	spdlog::info("[fancy2d] 开始创建 RenderTarget（渲染目标）");
	Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgi_surface;
	hr = gHR = dxgi_swapchain->GetBuffer(0, IID_ID3D11Texture2D, &dxgi_surface);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IDXGISwapChain::GetBuffer -> #0 调用失败，无法获得 BackBuffer（后备缓冲区）");
		return false;
	}
	hr = gHR = d3d11_device->CreateRenderTargetView(dxgi_surface.Get(), NULL, &d3d11_rendertarget);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] ID3D11Device::CreateRenderTargetView 调用失败");
		return false;
	}
	spdlog::info("[fancy2d] 已创建 RenderTarget（渲染目标）");

	spdlog::info("[fancy2d] 开始创建 DepthStencilBuffer（深度&模板缓冲区）");
	D3D11_TEXTURE2D_DESC tex2ddef = {
		.Width = swapchain_width,
		.Height = swapchain_height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1, .Quality = 0,},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
	hr = gHR = d3d11_device->CreateTexture2D(&tex2ddef, NULL, &d3d11_texture2d);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] ID3D11Device::CreateTexture2D 调用失败");
		return false;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdef = {
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = D3D11_TEX2D_DSV{.MipSlice = 0,},
	};
	hr = gHR = d3d11_device->CreateDepthStencilView(d3d11_texture2d.Get(), &dsvdef, &d3d11_depthstencil);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] ID3D11Device::CreateDepthStencilView 调用失败");
		return false;
	}
	spdlog::info("[fancy2d] 已创建 DepthStencilBuffer（深度&模板缓冲区）");

	spdlog::info("[fancy2d] 已创建 RenderAttachment（渲染附件）");

	return true;
}
void f2dRenderDevice11::setupRenderAttachments()
{
	if (d3d11_devctx && d3d11_rendertarget && d3d11_depthstencil)
	{
		ID3D11RenderTargetView* rtv = d3d11_rendertarget.Get();
		d3d11_devctx->OMSetRenderTargets(1, &rtv, d3d11_depthstencil.Get());
	}
}

fuInt f2dRenderDevice11::GetSupportedDisplayModeCount(fBool refresh)
{
	if (display_modes.empty() || refresh)
	{
		display_modes.clear();
		HRESULT hr = 0;

		spdlog::info("[fancy2d] 开始枚举支持的显示模式");

		if (!dxgi_swapchain)
		{
			spdlog::error("[fancy2d] 无法获得 Output（显示输出）设备");
			return 0;
		}
		
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGISwapChain::GetContainingOutput 调用失败，无法获得 Output（显示输出）设备");
			return 0;
		}

		UINT mode_count = 0;
		hr = gHR = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, NULL);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIOutput::GetDisplayModeList 调用失败");
			return 0;
		}
		std::vector<DXGI_MODE_DESC> modes(mode_count);
		hr = gHR = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, modes.data());
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIOutput::GetDisplayModeList 调用失败");
			return 0;
		}

		display_modes.reserve(mode_count);
		for (UINT i = 0; i < mode_count; i += 1)
		{
			if (
				((double)modes[i].RefreshRate.Numerator / (double)modes[i].RefreshRate.Denominator) >= 58.5
				&& (modes[i].Width >= 640 || modes[i].Height >= 360)
				&& (modes[i].ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED || modes[i].ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE)
				&& modes[i].Scaling == DXGI_MODE_SCALING_UNSPECIFIED)
			{
				
				display_modes.emplace_back(f2dDisplayMode{
					.width = modes[i].Width,
					.height = modes[i].Height,
					.refresh_rate = f2dRational{
						.numerator = modes[i].RefreshRate.Numerator,
						.denominator = modes[i].RefreshRate.Denominator,
					},
					.format = (fuInt)modes[i].Format,
					.scanline_ordering = (fuInt)modes[i].ScanlineOrdering,
					.scaling = (fuInt)modes[i].Scaling,
				});
			}
		}
		if (!display_modes.empty())
		{
			spdlog::info("[fancy2d] 共找到 {} 个支持的显示模式：", display_modes.size());
			for (size_t i = 0; i < display_modes.size(); i += 1)
			{
				spdlog::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz"
					, i
					, display_modes[i].width, display_modes[i].height
					, (double)display_modes[i].refresh_rate.numerator / (double)display_modes[i].refresh_rate.denominator
				);
			}
		}
		else
		{
			for (UINT i = 0; i < mode_count; i += 1)
			{
				display_modes.emplace_back(f2dDisplayMode{
						.width = modes[i].Width,
						.height = modes[i].Height,
						.refresh_rate = f2dRational{
							.numerator = modes[i].RefreshRate.Numerator,
							.denominator = modes[i].RefreshRate.Denominator,
						},
						.format = (fuInt)modes[i].Format,
						.scanline_ordering = (fuInt)modes[i].ScanlineOrdering,
						.scaling = (fuInt)modes[i].Scaling,
					});
			}
			if (!display_modes.empty())
			{
				spdlog::warn("[fancy2d] 找不到支持的显示模式，开始查找兼容的显示模式");
				spdlog::info("[fancy2d] 共找到 {} 个兼容的显示模式：", display_modes.size());
				for (size_t i = 0; i < display_modes.size(); i += 1)
				{
					spdlog::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz"
						, i
						, display_modes[i].width, display_modes[i].height
						, (double)display_modes[i].refresh_rate.numerator / (double)display_modes[i].refresh_rate.denominator
					);
				}
			}
			else
			{
				spdlog::error("[fancy2d] 枚举支持的显示模式失败，没有可用的显示模式");
				display_modes.clear();
				return 0;
			}
		}
	}
	return (fuInt)display_modes.size();
}
f2dDisplayMode f2dRenderDevice11::GetSupportedDisplayMode(fuInt Index)
{
	assert(Index < display_modes.size());
	return display_modes[Index];
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel)
{
	if (Width < 1 || Height < 1)
		return FCYERR_INVAILDPARAM;
	swapchain_width = Width;
	swapchain_height = Height;
	swapchain_windowed = true;
	swapchain_vsync = VSync;
	swapchain_flip = FlipModel;
	dispatchRenderSizeDependentResourcesDestroy();
	destroySwapchain();
	if (!createSwapchain(nullptr))
	{
		return FCYERR_INTERNALERR;
	}
	dispatchRenderSizeDependentResourcesCreate();
	return FCYERR_OK;
}
fResult f2dRenderDevice11::SetDisplayMode(f2dDisplayMode mode, fBool VSync)
{
	if (mode.width < 1 || mode.height < 1)
		return FCYERR_INVAILDPARAM;
	swapchain_width = mode.width;
	swapchain_height = mode.height;
	swapchain_windowed = false;
	swapchain_vsync = VSync;
	swapchain_flip = false; // 独占全屏永远不能开这个功能
	dispatchRenderSizeDependentResourcesDestroy();
	destroySwapchain();
	if (!createSwapchain(&mode))
	{
		return FCYERR_INTERNALERR;
	}
	dispatchRenderSizeDependentResourcesCreate();
	// 进入全屏
	spdlog::info("[fancy2d] 尝试切换到独占全屏");
	HRESULT hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #TRUE 调用失败，无法进入独占全屏");
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}
fuInt f2dRenderDevice11::GetBufferWidth() { return swapchain_width; }
fuInt f2dRenderDevice11::GetBufferHeight() { return swapchain_height; }
fBool f2dRenderDevice11::IsWindowed() { return swapchain_windowed; }
fResult f2dRenderDevice11::Present()
{
	if (!dxgi_swapchain)
	{
		return FCYERR_INTERNALERR;
	}

	setupRenderAttachments(); // 重新绑定回原来的
	m_RenderTarget = nullptr;
	m_DepthStencil = nullptr;

	UINT interval = swapchain_vsync ? 1 : 0;
	UINT flags = (swapchain_resize_data.AllowTearing && (!swapchain_vsync)) ? DXGI_PRESENT_ALLOW_TEARING : 0; // 这个功能只有窗口下才能开
	HRESULT hr = gHR = dxgi_swapchain->Present(interval, flags);
	if (hr != S_OK && hr != DXGI_STATUS_OCCLUDED && hr != DXGI_STATUS_MODE_CHANGED && hr != DXGI_STATUS_MODE_CHANGE_IN_PROGRESS)
	{
		spdlog::critical("[fancy2d] IDXGISwapChain::Present 调用失败，注意：设备已丢失，接下来的渲染将无法正常进行");
		spdlog::info("[fancy2d] 可能导致设备丢失的情况：\n"
			"    1、其他应用程序独占图形设备\n"
			"    2、图形设备驱动程序因驱动更新而重置\n"
			"    3、图形设备驱动程序崩溃\n"
			"    4、图形设备因运行异常，或电脑休眠/睡眠，而停止工作\n"
			"    5、图形设备已被移除*1\n"
			"    6、其他意外情况*2\n"
			"        *1 注意，某些电脑的显卡是热插拔的，比如某些平板、笔记本二合一电脑，独立显卡装在键盘、触控板和额外电池一侧上，如果把该侧移除变成平板形态，独显就也一起没了\n"
			"        *2 比如 Windows 系统玄学 Bug\n"
		);
		// 设备丢失，广播设备丢失事件
		m_bDevLost = true; // 标记为设备丢失状态
		int tObjCount = sendDevLostMsg();
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "device lost (%d object(s) lost)", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDevice11::Present", tBuffer));
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}

// 纹理读写

fResult f2dRenderDevice11::SaveScreen(f2dStream* pStream) { return FCYERR_NOTSUPPORT; }
fResult f2dRenderDevice11::SaveTexture(f2dStream* pStream, f2dTexture2D* pTex) { return FCYERR_NOTSUPPORT; }
fResult f2dRenderDevice11::SaveScreen(fcStrW path)
{
	if (!path)
		return FCYERR_INVAILDPARAM;
	if (!d3d11_devctx || !d3d11_rendertarget)
		return FCYERR_ILLEGAL;

	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource;
	d3d11_rendertarget->GetResource(&d3d11_resource);
	hr = gHR = DirectX::SaveWICTextureToFile(d3d11_devctx.Get(), d3d11_resource.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if (FAILED(hr))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDevice11::SaveScreen", "DirectX::SaveWICTextureToFile failed", hr));
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::SaveTexture(fcStrW path, f2dTexture2D* pTex)
{
	if (!path || !pTex)
		return FCYERR_INVAILDPARAM;
	if (!d3d11_devctx)
		return FCYERR_ILLEGAL;

	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_resource;
	if (pTex->IsRenderTarget())
	{
		f2dTexture2D11* pRTex = dynamic_cast<f2dTexture2D11*>(pTex);
		d3d11_resource = pRTex->GetResource();
	}
	else
	{
		f2dRenderTarget11* pRTex = dynamic_cast<f2dRenderTarget11*>(pTex);
		d3d11_resource = pRTex->GetResource();
	}
	hr = gHR = DirectX::SaveWICTextureToFile(d3d11_devctx.Get(), d3d11_resource.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if (FAILED(hr))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDevice11::SaveScreen", "DirectX::SaveWICTextureToFile failed", hr));
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

// 废弃的方法集合，大部分是固定管线遗毒，部分是完全没做好的功能，或者不适应新的图形API

fResult f2dRenderDevice11::SubmitWorldMat(const fcyMatrix4& Mat)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitLookatMat(const fcyMatrix4& Mat)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitProjMat(const fcyMatrix4& Mat)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitBlendState(const f2dBlendState& State)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitVD(IDirect3DVertexDeclaration9* pVD)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitTextureBlendOP_Color(D3DTEXTUREOP ColorOP)
{
	return FCYERR_NOTIMPL;
}
F2DAALEVEL f2dRenderDevice11::GetAALevel()
{
	return F2DAALEVEL_NONE;
}
fBool f2dRenderDevice11::CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed)
{
	return false;
}
fBool f2dRenderDevice11::IsZBufferEnabled()
{
	return false;
}
fResult f2dRenderDevice11::SetZBufferEnable(fBool v)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::UpdateScreenToWindow(fcyColor KeyColor, fByte Alpha)
{
	// 应该考虑用 DirectComposition 代替
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetTextureFilter(F2DTEXFILTERTYPE filter)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut)
{
	// 设计的很固定管线，所以抱歉了
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut)
{
	// 就支持个破屏幕后处理，也配叫 Graphics3D ？你配吗？配几把？
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut)
{
	// 时代眼泪 Effect 框架
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut)
{
	// 模型功能也没有完成
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel)
{
	if (Windowed)
	{
		return SetDisplayMode(Width, Height, VSync, FlipModel);
	}
	else
	{
		if (d3d11_device && dxgi_swapchain)
		{
			HRESULT hr = 0;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
			if (SUCCEEDED(hr))
			{
				DXGI_MODE_DESC target_mode = {
					.Width = Width,
					.Height = Height,
					.RefreshRate = DXGI_RATIONAL{.Numerator = 60,.Denominator = 1},
					.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
					.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
					.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
				};
				DXGI_MODE_DESC mode = {};
				hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &mode, d3d11_device.Get());
				if (SUCCEEDED(hr))
				{
					f2dDisplayMode f2dmode = {
						.width = mode.Width,
						.height = mode.Height,
						.refresh_rate = f2dRational{.numerator = mode.RefreshRate.Numerator, .denominator = mode.RefreshRate.Denominator},
						.format = (fuInt)mode.Format,
						.scanline_ordering = (fuInt)mode.ScanlineOrdering,
						.scaling = (fuInt)mode.Scaling,
					};
					return SetDisplayMode(f2dmode, VSync);
				}
			}
		}
		return FCYERR_INVAILDPARAM;
	}
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRateA, fuInt RefreshRateB, fBool Windowed, fBool VSync, fBool FlipModel)
{
	if (Windowed)
	{
		return SetDisplayMode(Width, Height, VSync, FlipModel);
	}
	else
	{
		if (d3d11_device && dxgi_swapchain)
		{
			HRESULT hr = 0;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
			if (SUCCEEDED(hr))
			{
				DXGI_MODE_DESC target_mode = {
					.Width = Width,
					.Height = Height,
					.RefreshRate = DXGI_RATIONAL{.Numerator = RefreshRateA,.Denominator = RefreshRateB},
					.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
					.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
					.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
				};
				DXGI_MODE_DESC mode = {};
				hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &mode, d3d11_device.Get());
				if (SUCCEEDED(hr))
				{
					f2dDisplayMode f2dmode = {
						.width = mode.Width,
						.height = mode.Height,
						.refresh_rate = f2dRational{.numerator = mode.RefreshRate.Numerator, .denominator = mode.RefreshRate.Denominator},
						.format = (fuInt)mode.Format,
						.scanline_ordering = (fuInt)mode.ScanlineOrdering,
						.scaling = (fuInt)mode.Scaling,
					};
					return SetDisplayMode(f2dmode, VSync);
				}
			}
		}
		return FCYERR_INVAILDPARAM;
	}
}
