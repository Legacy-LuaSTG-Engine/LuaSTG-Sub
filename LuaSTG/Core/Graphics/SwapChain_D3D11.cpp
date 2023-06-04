#include "Core/Graphics/SwapChain_D3D11.hpp"
#include "Core/Graphics/Format_D3D11.hpp"
#include "Core/i18n.hpp"
#include "Platform/WindowsVersion.hpp"
#include "Platform/CommandLineArguments.hpp"
#include "Platform/DesktopWindowManager.hpp"
#include "Platform/Direct3D11.hpp"
#include "Platform/DXGI.hpp"
#include "Platform/RuntimeLoader/D3DKMT.hpp"
#include "utf8.hpp"

#include "ScreenGrab11.h"

//#define _log(x) OutputDebugStringA(x "\n")
#define _log(x)

#define HRNew HRESULT hr = S_OK;
#define HRGet hr = gHR
#define HRCheckCallReport(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); }
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); assert(false); return false; }
#define HRCheckCallNoAssertReturnBool(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); return false; }

#define NTNew NTSTATUS nt{};
#define NTGet nt
#define NTCheckCallReport(x) if (nt != STATUS_SUCCESS) { i18n_core_system_call_report_error(x); }
#define NTCheckCallReturnBool(x) if (nt != STATUS_SUCCESS) { i18n_core_system_call_report_error(x); assert(false); return false; }
#define NTCheckCallNoAssertReturnBool(x) if (nt != STATUS_SUCCESS) { i18n_core_system_call_report_error(x); return false; }

namespace Core::Graphics
{
	constexpr DXGI_FORMAT const COLOR_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;
	constexpr DXGI_FORMAT const DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

	constexpr uint32_t const BACKGROUND_W = 512; // 512 * 16 = 8192 一般显示器大概也超不过这个分辨率？
	constexpr uint32_t const BACKGROUND_H = 512; // 16x 是硬件支持的最大放大级别 0.25x 是最小缩小级别，128 ~ 8192

	inline std::string_view to_string(DXGI_MODE_SCANLINE_ORDER v)
	{
		switch (v)
		{
		default:
		case DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED:
			assert(false); return "未知扫描方式";
		case DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE:
			return "逐行扫描";
		case DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST:
			return "隔行扫描（先奇数行）";
		case DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST:
			return "隔行扫描（先偶数行）";
		}
	}
	inline std::string_view to_string(DXGI_MODE_SCALING v)
	{
		switch (v)
		{
		default:
		case DXGI_MODE_SCALING_UNSPECIFIED:
			return "自动缩放";
		case DXGI_MODE_SCALING_CENTERED:
			return "居中显示";
		case DXGI_MODE_SCALING_STRETCHED:
			return "拉伸全屏";
		}
	}

	inline bool makeLetterboxing(Vector2U rect, Vector2U inner_rect, DXGI_MATRIX_3X2_F& mat)
	{
		if (rect == inner_rect)
		{
			return false; // 不需要
		}
		else
		{
			double const hscale = (double)rect.x / (double)inner_rect.x;
			double const vscale = (double)rect.y / (double)inner_rect.y;
			double const scale = std::min(hscale, vscale);
			double const width = scale * (double)inner_rect.x;
			double const height = scale * (double)inner_rect.y;
			double const x = ((double)rect.x - width) * 0.5;
			double const y = ((double)rect.y - height) * 0.5;
			mat = DXGI_MATRIX_3X2_F{
				FLOAT(scale), 0.0f,
				0.0f, FLOAT(scale),
				FLOAT(x), FLOAT(y),
			};
			return true;
		}
	}
	inline DXGI_SWAP_CHAIN_DESC1 getDefaultSwapChainInfo7()
	{
		return DXGI_SWAP_CHAIN_DESC1{
			.Width = 0,
			.Height = 0,
			.Format = COLOR_BUFFER_FORMAT,
			.Stereo = FALSE,
			.SampleDesc = DXGI_SAMPLE_DESC{
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 1,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE,
			.Flags = 0,
		};
	}
	inline DXGI_SWAP_CHAIN_DESC1 getDefaultSwapChainInfo10()
	{
		return DXGI_SWAP_CHAIN_DESC1{
			.Width = 0,
			.Height = 0,
			.Format = COLOR_BUFFER_FORMAT,
			.Stereo = FALSE,
			.SampleDesc = DXGI_SAMPLE_DESC{
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.Scaling = DXGI_SCALING_NONE,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE,
			.Flags = 0,
		};
	}
	static bool findBestDisplayMode(IDXGISwapChain1* dxgi_swapchain, Vector2U canvas_size, DXGI_MODE_DESC1& mode)
	{
		HRNew;

		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		HRGet = dxgi_swapchain->GetContainingOutput(&dxgi_output);
		HRCheckCallNoAssertReturnBool("IDXGISwapChain1::GetContainingOutput");

		Microsoft::WRL::ComPtr<IDXGIOutput1> dxgi_output_1;
		HRGet = dxgi_output.As(&dxgi_output_1);
		HRCheckCallReturnBool("IDXGIOutput::QueryInterface -> IDXGIOutput1");

		DXGI_OUTPUT_DESC dxgi_output_info = {};
		HRGet = dxgi_output_1->GetDesc(&dxgi_output_info);
		HRCheckCallReturnBool("IDXGIOutput1::GetDesc");
		assert(dxgi_output_info.AttachedToDesktop);
		assert(dxgi_output_info.Monitor);
		assert(dxgi_output_info.DesktopCoordinates.right > dxgi_output_info.DesktopCoordinates.left);
		assert(dxgi_output_info.DesktopCoordinates.bottom > dxgi_output_info.DesktopCoordinates.top);
		UINT const monitor_w = static_cast<UINT>(dxgi_output_info.DesktopCoordinates.right - dxgi_output_info.DesktopCoordinates.left);
		UINT const monitor_h = static_cast<UINT>(dxgi_output_info.DesktopCoordinates.bottom - dxgi_output_info.DesktopCoordinates.top);

		UINT dxgi_mode_count = 0;
		HRGet = dxgi_output_1->GetDisplayModeList1(COLOR_BUFFER_FORMAT, 0, &dxgi_mode_count, NULL);
		HRCheckCallReturnBool("IDXGIOutput1::GetDisplayModeList1 -> DXGI_FORMAT_B8G8R8A8_UNORM");
		assert(dxgi_mode_count > 0);

		std::vector<DXGI_MODE_DESC1> mode_list(dxgi_mode_count);
		HRGet = dxgi_output_1->GetDisplayModeList1(COLOR_BUFFER_FORMAT, 0, &dxgi_mode_count, mode_list.data());
		HRCheckCallReturnBool("IDXGIOutput1::GetDisplayModeList1 -> DXGI_FORMAT_B8G8R8A8_UNORM");

		// 公共函数

		auto scoreRefreshRate = [](DXGI_MODE_DESC1 const& v) -> int
		{
			int final_score = 0;

			double const refresh_rate_f = (double)v.RefreshRate.Numerator / (double)v.RefreshRate.Denominator;
			assert(refresh_rate_f >= 1.0); // 理论上不存在刷新率等于或者小于 0 的情况
			double const multiple_f = refresh_rate_f / 60.0; // 相对 60Hz 的倍数
			assert(multiple_f > 0.0);

			double const multiple = std::ceil(multiple_f);
			assert(multiple > 0.0);
			double const range_low = (60.0 * multiple) - multiple;
			double const range_high = (60.0 * multiple) + multiple;
			assert(range_low > 0.0);
			assert(range_high > 0.0);

			if (refresh_rate_f > range_low && refresh_rate_f < range_high)
			{
				// 是 60Hz 或者 60Hz 的倍数的刷新率
				int multiple_i = std::clamp((int)multiple, 1, 100); // 限制范围防止 int 乘上去后溢出
				final_score = 10000000 * multiple_i;
			}
			else if (refresh_rate_f > 118.0)
			{
				// 只要是 120Hz 或以上的刷新率，就已经可以满足游戏需求，无论是 144Hz 还是 165Hz 这种奇异刷新率
				final_score = 5000000;
			}
			else if (refresh_rate_f > 59.0)
			{
				// 还能接受的刷新率，比如 75Hz 和 90Hz
				final_score = 1000000;
			}
			else
			{
				// 以上要求都达不到，我只能说，你这个显示器就是个几把
				final_score = 0;
			}

			int const refresh_rate_v = std::clamp((int)(refresh_rate_f * 100.0), 0, 999999); // 理论上不会有超过一万刷新率的屏幕？
			final_score += refresh_rate_v; // 后面补上刷新率的 100 倍整数方便一次性比较

			return final_score;
		};

		auto compareRefreshRate = [=](DXGI_MODE_DESC1 const& a, DXGI_MODE_DESC1 const& b) -> bool
		{
			return scoreRefreshRate(a) > scoreRefreshRate(b);
		};

		auto checkRequiredRefreshRate = [](DXGI_MODE_DESC1 const& v) -> bool
		{
			return (double)v.RefreshRate.Numerator / (double)v.RefreshRate.Denominator >= 59.0;
		};

		auto checkRequiredCanvasSize = [&](DXGI_MODE_DESC1 const& v) -> bool
		{
			return v.Width >= canvas_size.x && v.Height >= canvas_size.y;
		};

		auto checkRequiredAspectRatio = [&](DXGI_MODE_DESC1 const& v) -> bool
		{
			if (monitor_w > monitor_h)
			{
				UINT const width = v.Height * monitor_w / monitor_h;
				assert(width > 0);
				if (width == 0) return false;
				else if (v.Width == width) return true;
				else if (width > v.Width && (width - v.Width) <= 2) return true;
				else if (v.Width > width && (v.Width - width) <= 2) return true;
				else return false;
			}
			else
			{
				UINT const height = v.Width * monitor_h / monitor_w;
				assert(height > 0);
				if (height == 0) return false;
				else if (v.Height == height) return true;
				else if (height > v.Height && (height - v.Height) <= 2) return true;
				else if (v.Height > height && (v.Height - height) <= 2) return true;
				else return false;
			}
		};

		// 默认的桌面分辨率

		std::vector<DXGI_MODE_DESC1> default_mode_list;
		for (auto const& v : mode_list)
		{
			if (v.Width == monitor_w && v.Height == monitor_h)
			{
				default_mode_list.emplace_back(v);
			}
		}

		assert(!default_mode_list.empty());
		if (default_mode_list.empty()) return false;

		std::sort(default_mode_list.begin(), default_mode_list.end(), compareRefreshRate);
		DXGI_MODE_DESC1 default_mode = default_mode_list.at(0);
		default_mode_list.clear();

		// 剔除隔行扫描（理论情况下不会出现）

		for (auto it = mode_list.begin(); it != mode_list.end();)
		{
			if (!(it->ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED || it->ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE))
				it = mode_list.erase(it);
			else
				it++;
		}

		// 剔除刷新率过低的

		for (auto it = mode_list.begin(); it != mode_list.end();)
		{
			if (!checkRequiredRefreshRate(*it))
			{
				it = mode_list.erase(it);
			}
			else
			{
				it++;
			}
		}

		// 剔除分辨率比画布分辨率低的

		for (auto it = mode_list.begin(); it != mode_list.end();)
		{
			if (!checkRequiredCanvasSize(*it))
			{
				it = mode_list.erase(it);
			}
			else
			{
				it++;
			}
		}

		// 剔除横纵比不和显示器匹配的

		for (auto it = mode_list.begin(); it != mode_list.end();)
		{
			if (!checkRequiredAspectRatio(*it))
			{
				it = mode_list.erase(it);
			}
			else
			{
				it++;
			}
		}

		// 根据面积比和刷新率进行排序，优先刷新率，然后是面积比

		auto compareByRefreshRateAndSize = [&](DXGI_MODE_DESC1 const& a, DXGI_MODE_DESC1 const& b) -> bool
		{
			double const canvas_sz = (double)canvas_size.x * (double)canvas_size.y;
			double const sz_a = canvas_sz / ((double)a.Width * (double)a.Height);
			double const sz_b = canvas_sz / ((double)b.Width * (double)b.Height);
			if (sz_a == sz_b)
			{
				return compareRefreshRate(a, b);
			}
			else
			{
				return sz_b > sz_b;
			}
		};

		std::sort(mode_list.begin(), mode_list.end(), compareByRefreshRateAndSize);

		// 打印结果

		i18n_log_info_fmt("[core].SwapChain_D3D11.found_N_DisplayMode_fmt", mode_list.size());
		for (size_t i = 0; i < mode_list.size(); i += 1)
		{
			spdlog::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz {} {}"
				, i
				, mode_list[i].Width, mode_list[i].Height
				, (double)mode_list[i].RefreshRate.Numerator / (double)mode_list[i].RefreshRate.Denominator
				, to_string(mode_list[i].Scaling)
				, to_string(mode_list[i].ScanlineOrdering)
			);
		}

		// 最终，挑选出面积比最大且刷新率最高的

		if (!mode_list.empty())
		{
			mode = mode_list.at(0);
		}
		else
		{
			mode = default_mode;
		}

		return true;
	}
	static bool checkHardwareCompositionSupport(ID3D11Device* device)
	{
		assert(device);

		HRNew;

		// 不是 Windows 10 则跳过

		if (!IsWindows10OrGreater())
		{
			return false;
		}

		// 不支持画面撕裂则跳过

		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory;
		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device = device;

		HRGet = Platform::Direct3D11::GetDeviceFactory(d3d11_device.Get(), &dxgi_factory);
		HRCheckCallReturnBool("ID3D11Device::GetParent -> IDXGIFactory2");

		if (!Platform::DXGI::CheckFeatureSupportPresentAllowTearing(dxgi_factory.Get()))
		{
			return false;
		}

		// 用户禁用了 MPO 则跳过

		if (Platform::DesktopWindowManager::IsOverlayTestModeExists())
		{
			return false;
		}

		// 检查显示输出拓扑

		bool v_primary_support = true;
		bool v_support = true;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> v_adapter;
		for (UINT i_adapter = 0; SUCCEEDED(dxgi_factory->EnumAdapters1(i_adapter, &v_adapter)); i_adapter += 1)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput> v_output;
			for (UINT i_output = 0; SUCCEEDED(v_adapter->EnumOutputs(i_output, &v_output)); i_output += 1)
			{
				BOOL is_primary = FALSE;
				BOOL overlays = FALSE;
				UINT overlay_support = 0;
				UINT hardware_composition_support = 0;

				DXGI_OUTPUT_DESC v_output_info = {};
				HRGet = v_output->GetDesc(&v_output_info);
				HRCheckCallReport("IDXGIOutput::GetDesc");
				if (SUCCEEDED(hr))
				{
					MONITORINFOEXW v_monitor_info = {};
					v_monitor_info.cbSize = sizeof(v_monitor_info);
					if (!GetMonitorInfoW(v_output_info.Monitor, &v_monitor_info))
					{
						hr = HRESULT_FROM_WIN32(GetLastError());
						HRCheckCallReport("GetMonitorInfoW");
					}
					else
					{
						if (v_monitor_info.dwFlags & MONITORINFOF_PRIMARY)
						{
							is_primary = TRUE;
						}
					}
				}

				Microsoft::WRL::ComPtr<IDXGIOutput2> v_output2;
				HRGet = v_output.As(&v_output2);
				HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput2");
				if (v_output2)
				{
					overlays = v_output2->SupportsOverlays();
				}

				Microsoft::WRL::ComPtr<IDXGIOutput3> v_output3;
				HRGet = v_output.As(&v_output3);
				HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput3");
				if (v_output3)
				{
					HRGet = v_output3->CheckOverlaySupport(DXGI_FORMAT_B8G8R8A8_UNORM, d3d11_device.Get(), &overlay_support);
					HRCheckCallReport("IDXGIOutput3::CheckOverlaySupport -> DXGI_FORMAT_B8G8R8A8_UNORM");
				}

				Microsoft::WRL::ComPtr<IDXGIOutput6> v_output6;
				HRGet = v_output.As(&v_output6);
				HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput6");
				if (v_output6)
				{
					HRGet = v_output6->CheckHardwareCompositionSupport(&hardware_composition_support);
					HRCheckCallReport("IDXGIOutput6::CheckHardwareCompositionSupport");
				}

				bool const condition1 = (overlays);
				bool const condition2 = (overlay_support & DXGI_OVERLAY_SUPPORT_FLAG_DIRECT) && (overlay_support & DXGI_OVERLAY_SUPPORT_FLAG_SCALING);
				bool const condition3 = (hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN);
				if (is_primary)
				{
					if (!((condition1 || condition2) && condition3))
					{
						v_primary_support = false;
						v_support = false;
					}
				}
				else
				{
					// 多显示输出系统上，似乎只有主显示输出会报告多平面叠加支持
					if (!((condition1 || condition2 || v_primary_support) && condition3))
					{
						v_support = false;
					}
				}
			}
		}

		return v_support;
	}
	static bool checkModernSwapChainModelAvailable(ID3D11Device* device)
	{
		// 是否需要统一开启现代交换链模型
		// 我们划定一个红线，红线以下永远不开启，红线以上永远开启
		// 这样可以简化逻辑的处理

		assert(device);
		HRNew;
		NTNew;

		// 预检系统版本 Windows 10 1809
		// * DXGI_SWAP_EFFECT_FLIP_DISCARD 从 Windows 10 开始支持
		// * 在 Windows 10 1709 (16299) Fall Creators Update 中
		//   修复了 Frame Latency Waitable Object 和 SetMaximumFrameLatency 实际上至少有 2 帧的问题
		// * Windows 10 1809 (17763) 是目前微软还在主流支持的最早版本
		// * DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT 从 Windows 8.1 开始支持，我们跳过它

		if (!Platform::WindowsVersion::Is10Build17763()) {
			return false;
		}

		// 检查 PresentAllowTearing
		// * DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING 从 Windows 10 开始支持
		// * DXGI_PRESENT_ALLOW_TEARING 从 Windows 10 开始支持
		// * 此外还有别的要求，比如WDDM支持、MPO支持、显卡驱动支持等
		// * 注意，就算报告支持，实际运行的时候可能仍然不允许撕裂
		// * 系统安装了 KB3156421 更新，也就是 Windows 10 在 2016 年 5 月 10 日的更新
		//   https://zhuanlan.zhihu.com/p/20892856?refer=highwaytographics

		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory;
		HRGet = Platform::Direct3D11::GetDeviceFactory(device, &dxgi_factory);
		HRCheckCallReturnBool("Platform::Direct3D11::GetDeviceFactory");
		if (!Platform::DXGI::CheckFeatureSupportPresentAllowTearing(dxgi_factory.Get()))
		{
			return false;
		}

		// 检查 DirectFlip

		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
		HRGet = Platform::Direct3D11::GetDeviceAdater(device, &dxgi_adapter);
		HRCheckCallReturnBool("Platform::Direct3D11::GetDeviceAdater");
		DXGI_ADAPTER_DESC1 dxgi_adapter_info{};
		HRGet = dxgi_adapter->GetDesc1(&dxgi_adapter_info);
		HRCheckCallReturnBool("IDXGIAdapter1::GetDesc1");

		Platform::RuntimeLoader::D3DKMT d3dkmt;

		D3DKMT_OPENADAPTERFROMLUID open_adapter_from_luid{};
		open_adapter_from_luid.AdapterLuid = dxgi_adapter_info.AdapterLuid;
		NTGet = d3dkmt.OpenAdapterFromLuid(&open_adapter_from_luid);
		NTCheckCallReturnBool("D3DKMTOpenAdapterFromLuid");

		auto auto_close_adapter = wil::scope_exit([&open_adapter_from_luid, &d3dkmt]
			{
				D3DKMT_CLOSEADAPTER close_adapter{};
				close_adapter.hAdapter = open_adapter_from_luid.hAdapter;
				NTNew;
				NTGet = d3dkmt.CloseAdapter(&close_adapter);
				NTCheckCallReport("D3DKMTCloseAdapter");
			});
		
		D3DKMT_CREATEDEVICE create_device{};
		create_device.hAdapter = open_adapter_from_luid.hAdapter;
		NTGet = d3dkmt.CreateDevice(&create_device);
		NTCheckCallReturnBool("D3DKMTCreateDevice");
		
		auto auto_close_device = wil::scope_exit([&create_device, &d3dkmt]
			{
				D3DKMT_DESTROYDEVICE destroy_device{};
				destroy_device.hDevice = create_device.hDevice;
				NTNew;
				NTGet = d3dkmt.DestroyDevice(&destroy_device);
				NTCheckCallReport("D3DKMTDestroyDevice");
			});

		auto query_adapter_info = [&open_adapter_from_luid, &d3dkmt]<typename T>(KMTQUERYADAPTERINFOTYPE type, T, std::string_view type_name) -> T {
			T data{};
			D3DKMT_QUERYADAPTERINFO query{};
			query.hAdapter = open_adapter_from_luid.hAdapter;
			query.Type = type;
			query.pPrivateDriverData = &data;
			query.PrivateDriverDataSize = sizeof(data);
			NTNew;
			NTGet = d3dkmt.QueryAdapterInfo(&query);
			NTCheckCallReport(std::string("D3DKMTQueryAdapterInfo -> ").append(type_name));
			return data;
		};

		// wddm 1.2
		auto const direct_flip_caps = query_adapter_info(KMTQAITYPE_DIRECTFLIP_SUPPORT, D3DKMT_DIRECTFLIP_SUPPORT{}, "D3DKMT_DIRECTFLIP_SUPPORT");

		return direct_flip_caps.Supported;
	}
	inline bool isModernSwapChainModel(DXGI_SWAP_CHAIN_DESC1 const& info)
	{
		return info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
			|| info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD
			;
	}
	inline bool isModernSwapChainModel(IDXGISwapChain1* dxgi_swapchain)
	{
		HRNew;

		DXGI_SWAP_CHAIN_DESC1 info = {};
		HRGet = dxgi_swapchain->GetDesc1(&info);
		HRCheckCallReturnBool("IDXGISwapChain1::GetDesc1");

		return isModernSwapChainModel(info);
	}

	void SwapChain_D3D11::dispatchEvent(EventType t)
	{
		// 回调
		m_is_dispatch_event = true;
		switch (t)
		{
		case EventType::SwapChainCreate:
			for (auto& v : m_eventobj)
			{
				if (v) v->onSwapChainCreate();
			}
			break;
		case EventType::SwapChainDestroy:
			for (auto& v : m_eventobj)
			{
				if (v) v->onSwapChainDestroy();
			}
			break;
		}
		m_is_dispatch_event = false;
		// 处理那些延迟的对象
		removeEventListener(nullptr);
		for (auto& v : m_eventobj_late)
		{
			m_eventobj.emplace_back(v);
		}
		m_eventobj_late.clear();
	}
	void SwapChain_D3D11::addEventListener(ISwapChainEventListener* e)
	{
		removeEventListener(e);
		if (m_is_dispatch_event)
		{
			m_eventobj_late.emplace_back(e);
		}
		else
		{
			m_eventobj.emplace_back(e);
		}
	}
	void SwapChain_D3D11::removeEventListener(ISwapChainEventListener* e)
	{
		if (m_is_dispatch_event)
		{
			for (auto& v : m_eventobj)
			{
				if (v == e)
				{
					v = nullptr; // 不破坏遍历过程
				}
			}
		}
		else
		{
			for (auto it = m_eventobj.begin(); it != m_eventobj.end();)
			{
				if (*it == e)
					it = m_eventobj.erase(it);
				else
					it++;
			}
		}
	}

	void SwapChain_D3D11::onDeviceCreate()
	{
		m_modern_swap_chain_available = checkModernSwapChainModelAvailable(m_device->GetD3D11Device());
		m_scaling_renderer.AttachDevice(m_device->GetD3D11Device());
		if (m_init) // 曾经设置过
		{
			setWindowMode(m_canvas_size);
			if (m_swap_chain_fullscreen_mode)
			{
				// 重新进入独占全屏
				onWindowFullscreenStateChange(true);
			}
		}
	}
	void SwapChain_D3D11::onDeviceDestroy()
	{
		destroySwapChain();
		m_scaling_renderer.DetachDevice();
		m_modern_swap_chain_available = false;
	}
	void SwapChain_D3D11::onWindowCreate()
	{
		// 目前窗口的重新创建只会被交换链触发，所以这里留空
	}
	void SwapChain_D3D11::onWindowDestroy()
	{
		// 目前窗口的重新创建只会被交换链触发，所以这里留空
	}
	void SwapChain_D3D11::onWindowActive()
	{
		if (!dxgi_swapchain)
			return;
		_log("onWindowActive");
		m_swapchain_want_present_reset = TRUE;
		enterExclusiveFullscreenTemporarily();
	}
	void SwapChain_D3D11::onWindowInactive()
	{
		if (!dxgi_swapchain)
			return;
		_log("onWindowInactive");
		leaveExclusiveFullscreenTemporarily();
	}
	void SwapChain_D3D11::onWindowSize(Core::Vector2U size)
	{
		if (size.x == 0 || size.y == 0)
			return; // 忽略窗口最小化
		if (!dxgi_swapchain)
			return; // 此时交换链还未初始化
		if (m_is_composition_mode)
		{
			handleDirectCompositionWindowSize(size);
		}
		else
		{
			handleSwapChainWindowSize(size);
		}
	}
	void SwapChain_D3D11::onWindowFullscreenStateChange(bool state)
	{
		if (!dxgi_swapchain)
			return;
		if (state)
			enterExclusiveFullscreen();
		else
			leaveExclusiveFullscreen();
	}

	bool SwapChain_D3D11::createSwapChain(bool fullscreen, DXGI_MODE_DESC1 const& mode, bool no_attachment)
	{
		_log("createSwapChain");

		i18n_log_info("[core].SwapChain_D3D11.start_creating_swapchain");

		// 必须成功的操作

		if (!m_window->GetWindow())
		{
			i18n_log_error("[core].SwapChain_D3D11.create_swapchain_failed_null_window");
			assert(false); return false;
		}
		if (!m_device->GetD3D11Device())
		{
			i18n_log_error("[core].SwapChain_D3D11.create_swapchain_failed_null_device");
			assert(false); return false;
		}

		HRNew;

		// 填写交换链描述

		bool const flip_available = m_modern_swap_chain_available;

		m_swap_chain_info = getDefaultSwapChainInfo7();
		m_swap_chain_fullscreen_info = {};
		
		// 切换为 FLIP 交换链模型，独占全屏也支持
		if (flip_available)
		{
			m_swap_chain_info.BufferCount = 2;
			m_swap_chain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}

		if (!fullscreen)
		{
			// 获取窗口尺寸
			RECT rc = {};
			if (!GetClientRect(m_window->GetWindow(), &rc))
			{
				HRGet = HRESULT_FROM_WIN32(GetLastError());
				HRCheckCallReturnBool("GetClientRect");
			}
			if (rc.right <= rc.left || rc.bottom <= rc.top)
			{
				i18n_log_error_fmt("[core].SwapChain_D3D11.create_swapchain_failed_invalid_size_fmt",
					rc.right - rc.left,
					rc.bottom - rc.top);
				assert(false); return false;
			}
			// 使用窗口尺寸
			m_swap_chain_info.Width = static_cast<UINT>(rc.right - rc.left);
			m_swap_chain_info.Height = static_cast<UINT>(rc.bottom - rc.top);
			// 进一步配置 FLIP 交换链模型
			if (flip_available)
			{
				m_swap_chain_info.BufferCount = 3; // 三个缓冲区能带来更平稳的性能
				m_swap_chain_info.Scaling = DXGI_SCALING_NONE; // 禁用 DWM 对交换链的缩放
				m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; // 低延迟呈现技术
				m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // 立即刷新和可变刷新率
			}
		}
		else
		{
			// 使用显示模式尺寸
			m_swap_chain_info.Width = mode.Width;
			m_swap_chain_info.Height = mode.Height;
			// 允许切换显示模式
			m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			// 配置全屏模式
			m_swap_chain_fullscreen_info.RefreshRate = mode.RefreshRate;
			m_swap_chain_fullscreen_info.ScanlineOrdering = mode.ScanlineOrdering;
			m_swap_chain_fullscreen_info.Scaling = mode.Scaling;
			m_swap_chain_fullscreen_info.Windowed = TRUE; // 稍后再切换到独占全屏
		}

		// 创建交换链

		HRGet = m_device->GetDXGIFactory2()->CreateSwapChainForHwnd(
			m_device->GetD3D11Device(),
			m_window->GetWindow(),
			&m_swap_chain_info,
			fullscreen ? &m_swap_chain_fullscreen_info : NULL,
			NULL,
			&dxgi_swapchain);
		HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForHwnd");
		
		m_swap_chain_fullscreen_mode = fullscreen;

		// 关闭傻逼快捷键，别他妈乱切换了
		// 注意这里他妈的有坑，新创建的 DXGI 工厂和交换链内部的的不是同一个

		HRGet = Platform::DXGI::MakeSwapChainWindowAssociation(
			dxgi_swapchain.Get(), DXGI_MWA_NO_ALT_ENTER);
		HRCheckCallReturnBool("IDXGIFactory1::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");
		
		// 设置设备最大帧延迟为 1

		HRGet = Platform::DXGI::SetDeviceMaximumFrameLatency(
			dxgi_swapchain.Get(), 1);
		HRCheckCallReturnBool("IDXGIDevice1::SetMaximumFrameLatency -> 1");
		
		// 如果启用了低延迟呈现技术，则设置交换链最大帧延迟为 1
		
		if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
		{
			HANDLE event_handle{};
			HRGet = Platform::DXGI::SetSwapChainMaximumFrameLatency(
				dxgi_swapchain.Get(), 1, &event_handle);
			HRCheckCallReturnBool("IDXGISwapChain2::SetMaximumFrameLatency -> 1");
			dxgi_swapchain_event.Attach(event_handle);
		}

		//i18n_log_info("[core].SwapChain_D3D11.created_swapchain");

		auto refresh_rate_string = fmt::format("{:.2f}Hz", (double)mode.RefreshRate.Numerator / (double)mode.RefreshRate.Denominator);
		if (!fullscreen) refresh_rate_string = i18n("DXGI.DisplayMode.RefreshRate.Desktop");
		std::string_view swapchain_model = i18n("DXGI.SwapChain.SwapEffect.Discard");
		if (m_swap_chain_info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL) swapchain_model = i18n("DXGI.SwapChain.SwapEffect.FlipSequential");
		if (m_swap_chain_info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) swapchain_model = i18n("DXGI.SwapChain.SwapEffect.FlipDiscard");
		auto enable_or_disable = [](bool v) -> std::string_view { return v ? i18n("Enable") : i18n("Disable"); };
		i18n_log_info_fmt("[core].SwapChain_D3D11.created_swapchain_info_fmt"
			, mode.Width, mode.Height, refresh_rate_string
			, enable_or_disable(fullscreen)
			, swapchain_model
			, enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
			, enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
		);

		// 渲染附件

		if (!no_attachment)
		{
			if (!createRenderAttachment()) return false;
		}

		// 标记

		m_swapchain_want_present_reset = TRUE;

		return true;
	}
	void SwapChain_D3D11::destroySwapChain()
	{
		_log("destroySwapChain");

		//waitFrameLatency(INFINITE, true);
		destroyDirectCompositionResources();
		destroyRenderAttachment();
		if (dxgi_swapchain)
		{
			// 退出独占全屏
			HRNew;
			BOOL bFullscreen = FALSE;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			HRGet = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
			HRCheckCallReport("IDXGISwapChain::GetFullscreenState");
			if (SUCCEEDED(hr) && bFullscreen)
			{
				i18n_log_info("[core].SwapChain_D3D11.leave_exclusive_fullscreen");
				HRGet = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
				HRCheckCallReport("IDXGISwapChain::SetFullscreenState -> FALSE");
			}
		}
		dxgi_swapchain_event.Close();
		dxgi_swapchain.Reset();
	}
	void SwapChain_D3D11::waitFrameLatency(uint32_t timeout, bool reset)
	{
		//_log("waitFrameLatency");

		if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
		{
			if (reset && dxgi_swapchain)
			{
				// 微软不知道写了什么狗屎bug，有时候dwm临时接管桌面合成后会导致上屏延迟多一倍
				// 重新设置最大帧延迟并创建延迟等待对象似乎能解决该问题
				dxgi_swapchain_event.Close();
				Microsoft::WRL::ComPtr<IDXGISwapChain2> dxgi_swapchain2;
				winrt::check_hresult(dxgi_swapchain.As(&dxgi_swapchain2));
				winrt::check_hresult(dxgi_swapchain2->SetMaximumFrameLatency(1));
				dxgi_swapchain_event.Attach(dxgi_swapchain2->GetFrameLatencyWaitableObject());
			}
			if (dxgi_swapchain_event.IsValid())
			{
				DWORD const result = WaitForSingleObject(dxgi_swapchain_event.Get(), timeout);
				if (!(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT))
				{
					gHRLastError;
					i18n_core_system_call_report_error("WaitForSingleObject");
				}
			}
		}
	}
	bool SwapChain_D3D11::enterExclusiveFullscreenTemporarily()
	{
		if (!m_swap_chain_fullscreen_mode)
		{
			return true;
		}
		if (!dxgi_swapchain)
		{
			assert(false); return false;
		}

		HRNew;

		BOOL get_state = FALSE;
		HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
		HRCheckCallReturnBool("IDXGISwapChain::GetFullscreenState");

		if (get_state)
		{
			return true;
		}

		_log("IDXGISwapChain::SetFullscreenState -> TRUE\n");
		i18n_log_info("[core].SwapChain_D3D11.enter_exclusive_fullscreen");
		HRGet = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
		HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> TRUE");

		return true;
	}
	bool SwapChain_D3D11::leaveExclusiveFullscreenTemporarily()
	{
		if (!m_swap_chain_fullscreen_mode)
		{
			return true;
		}
		if (!dxgi_swapchain)
		{
			assert(false); return false;
		}

		HRNew;

		BOOL get_state = FALSE;
		HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
		HRCheckCallReturnBool("IDXGISwapChain::GetFullscreenState");

		if (!get_state)
		{
			return true;
		}

		_log("IDXGISwapChain::SetFullscreenState -> FALSE\n");
		i18n_log_info("[core].SwapChain_D3D11.leave_exclusive_fullscreen");
		HRGet = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
		HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> FALSE");

		m_window->setLayer(WindowLayer::Normal); // 强制取消窗口置顶

		return true;
	}
	bool SwapChain_D3D11::enterExclusiveFullscreen()
	{
		bool const disable_efs = Platform::CommandLineArguments::Get().IsOptionExist("--disable-exclusive-fullscreen");
		if (disable_efs)
		{
			return false;
		}

		assert(dxgi_swapchain);
		if (!dxgi_swapchain) return false;

		DXGI_MODE_DESC1 display_mode = {};
		if (!findBestDisplayMode(dxgi_swapchain.Get(), m_canvas_size, display_mode)) return false;

		dispatchEvent(EventType::SwapChainDestroy);
		destroySwapChain();

		if (!m_window->getRedirectBitmapEnable())
		{
			m_window->setRedirectBitmapEnable(true);
			if (!m_window->recreateWindow()) return false;
		}

		m_window->setSize({ display_mode.Width, display_mode.Height });

		if (!createSwapChain(true, display_mode, true)) // 稍后创建渲染附件
		{
			return false;
		}

		HRNew;

		// 进入全屏
		i18n_log_info("[core].SwapChain_D3D11.enter_exclusive_fullscreen");
		HRGet = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
		HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> TRUE");

		// 需要重设交换链大小（特别是 Flip 交换链模型）
		HRGet = dxgi_swapchain->ResizeBuffers(0, display_mode.Width, display_mode.Height, DXGI_FORMAT_UNKNOWN, m_swap_chain_info.Flags);
		HRCheckCallReturnBool("IDXGISwapChain::ResizeBuffers");

		// 创建渲染附件
		if (!createRenderAttachment())
		{
			return false;
		}
		if (!updateLetterBoxingRendererTransform()) return false;

		// 记录状态
		m_init = TRUE;

		// 广播
		dispatchEvent(EventType::SwapChainCreate);
		
		return true;
	}
	bool SwapChain_D3D11::leaveExclusiveFullscreen()
	{
		HRNew;

		BOOL get_state = FALSE;
		HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
		HRCheckCallReturnBool("IDXGISwapChain::GetFullscreenState");

		if (get_state)
		{
			i18n_log_info("[core].SwapChain_D3D11.leave_exclusive_fullscreen");
			HRGet = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
			HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> FALSE");
		}
		
		if (m_swap_chain_fullscreen_mode)
		{
			m_swap_chain_fullscreen_mode = FALSE; // 手动离开全屏模式

			dispatchEvent(EventType::SwapChainDestroy);
			destroySwapChain();

			if (!m_window->getRedirectBitmapEnable())
			{
				m_window->setRedirectBitmapEnable(true);
				if (!m_window->recreateWindow()) return false;
			}

			if (!createSwapChain(false, {}, false))
			{
				return false;
			}

			if (!updateLetterBoxingRendererTransform()) return false;

			// 记录状态
			m_init = TRUE;

			// 广播
			dispatchEvent(EventType::SwapChainCreate);

			return true;
		}
		
		return true;
	}

	bool SwapChain_D3D11::createDirectCompositionResources()
	{
		_log("createDirectCompositionResources");

		// 我们限制 DirectComposition 仅在 Windows 10+ 使用

		HRNew;

		// 必须成功的操作

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
		HRGet = m_device->GetD3D11Device()->QueryInterface(IID_PPV_ARGS(&dxgi_device));
		HRCheckCallReturnBool("ID3D11Device::QueryInterface -> IDXGIDevice");
		
		// 创建基本组件

		HRGet = dcomp_loader.CreateDevice(dxgi_device.Get(), IID_PPV_ARGS(&dcomp_desktop_device));
		HRCheckCallReturnBool("DCompositionCreateDevice");
		
	#ifdef _DEBUG
		Microsoft::WRL::ComPtr<IDCompositionDeviceDebug> dcomp_device_debug;
		HRGet = dcomp_desktop_device.As(&dcomp_device_debug);
		HRCheckCallReport("IDCompositionDesktopDevice::QueryInterface -> IDCompositionDeviceDebug");
		if (SUCCEEDED(hr))
		{
			HRGet = dcomp_device_debug->EnableDebugCounters();
			HRCheckCallReport("IDCompositionDeviceDebug::EnableDebugCounters");
		}
	#endif

		HRGet = dcomp_desktop_device->CreateTargetForHwnd(m_window->GetWindow(), TRUE, &dcomp_target);
		HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateTargetForHwnd");
		
		HRGet = dcomp_desktop_device->CreateVisual(&dcomp_visual_root);
		HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");
		
		HRGet = dcomp_desktop_device->CreateVisual(&dcomp_visual_background);
		HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

		HRGet = dcomp_desktop_device->CreateVisual(&dcomp_visual_swap_chain);
		HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

		// 初始化背景

		HRGet = dcomp_desktop_device->CreateSurface(
			BACKGROUND_W, BACKGROUND_H,
			COLOR_BUFFER_FORMAT,
			DXGI_ALPHA_MODE_IGNORE,
			&dcomp_surface_background);
		HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateSurface");

		if (SUCCEEDED(hr))
		{
			Microsoft::WRL::ComPtr<IDXGISurface> dxgi_surface;
			POINT offset = { 0, 0 };
			HRGet = dcomp_surface_background->BeginDraw(NULL, IID_PPV_ARGS(&dxgi_surface), &offset);
			HRCheckCallReturnBool("IDCompositionSurface::BeginDraw");
			
			Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_res;
			HRGet = dxgi_surface.As(&d3d11_res);
			HRCheckCallReturnBool("IDXGISurface::QueryInterface -> ID3D11Resource");
			
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_dcomp_bg_rtv;
			HRGet = m_device->GetD3D11Device()->CreateRenderTargetView(d3d11_res.Get(), NULL, &d3d11_dcomp_bg_rtv);
			HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");
			
			FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_device->GetD3D11DeviceContext()->ClearRenderTargetView(d3d11_dcomp_bg_rtv.Get(), clear_color);
			m_device->GetD3D11DeviceContext()->Flush();

			HRGet = dcomp_surface_background->EndDraw();
			HRCheckCallReturnBool("IDCompositionSurface::EndDraw");
		}

		HRGet = dcomp_visual_background->SetContent(dcomp_surface_background.Get());
		HRCheckCallReturnBool("IDCompositionVisual2::SetContent");
		
		// 把交换链塞进可视物

		HRGet = dcomp_visual_swap_chain->SetContent(dxgi_swapchain.Get());
		HRCheckCallReturnBool("IDCompositionVisual2::SetContent");
		
		HRGet = dcomp_visual_swap_chain->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR);
		HRCheckCallReturnBool("IDCompositionVisual2::SetBitmapInterpolationMode -> DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR");

		// 构建视觉树

		HRGet = dcomp_visual_root->AddVisual(dcomp_visual_background.Get(), TRUE, NULL);
		HRCheckCallReturnBool("IDCompositionVisual2::AddVisual");

		HRGet = dcomp_visual_root->AddVisual(dcomp_visual_swap_chain.Get(), FALSE, NULL);
		HRCheckCallReturnBool("IDCompositionVisual2::AddVisual");

		HRGet = dcomp_target->SetRoot(dcomp_visual_root.Get());
		HRCheckCallReturnBool("IDCompositionTarget::SetRoot");
		
		// 设置变换并提交

		if (!updateDirectCompositionTransform()) return false;

		return true;
	}
	void SwapChain_D3D11::destroyDirectCompositionResources()
	{
		_log("destroyDirectCompositionResources");

		// 我也不知道为什么要这样清理，但是不这么做的话会出现内存不足，后续创建设备和资源的操作会失败

		HRNew;

		if (dcomp_target)
		{
			HRGet = dcomp_target->SetRoot(NULL);
			HRCheckCallReport("IDCompositionTarget::SetRoot -> NULL");
		}
		if (dcomp_visual_root)
		{
			HRGet = dcomp_visual_root->RemoveAllVisuals();
			HRCheckCallReport("IDCompositionVisual2::RemoveAllVisuals");
		}
		if (dcomp_visual_background)
		{
			HRGet = dcomp_visual_background->SetContent(NULL);
			HRCheckCallReport("IDCompositionVisual2::SetContent -> NULL");
		}
		if (dcomp_visual_swap_chain)
		{
			HRGet = dcomp_visual_swap_chain->SetContent(NULL);
			HRCheckCallReport("IDCompositionVisual2::SetContent -> NULL");
		}

		dcomp_target.Reset();
		dcomp_visual_root.Reset();
		dcomp_visual_background.Reset();
		dcomp_visual_swap_chain.Reset();
		dcomp_surface_background.Reset();

		if (dcomp_desktop_device)
		{
			HRGet = dcomp_desktop_device->Commit();
			HRCheckCallReport("IDCompositionDesktopDevice::Commit");
			HRGet = dcomp_desktop_device->WaitForCommitCompletion();
			HRCheckCallReport("IDCompositionDesktopDevice::WaitForCommitCompletion");
		}
		
		dcomp_desktop_device.Reset();
	}
	bool SwapChain_D3D11::updateDirectCompositionTransform()
	{
		_log("updateDirectCompositionTransform");

		HRNew;

		// 必须成功的操作

		DXGI_SWAP_CHAIN_DESC1 desc1 = {};
		HRGet = dxgi_swapchain->GetDesc1(&desc1);
		HRCheckCallReturnBool("IDXGISwapChain1::GetDesc1");
		
		RECT rc = {};
		if (!GetClientRect(m_window->GetWindow(), &rc))
		{
			HRGet = HRESULT_FROM_WIN32(GetLastError());
			HRCheckCallReturnBool("GetClientRect");
		}
		auto const window_size_u = Vector2U(
			(uint32_t)(rc.right - rc.left),
			(uint32_t)(rc.bottom - rc.top));

		// 让背景铺满整个画面

		auto const background_mat = D2D1::Matrix3x2F::Scale(
			(float)window_size_u.x / (float)BACKGROUND_W,
			(float)window_size_u.y / (float)BACKGROUND_H
		);
		HRGet = dcomp_visual_background->SetTransform(background_mat);
		HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
		
		// 设置交换链内容内接放大

		DXGI_MATRIX_3X2_F mat{};
		if (makeLetterboxing(window_size_u, Vector2U(desc1.Width, desc1.Height), mat))
		{
			D2D_MATRIX_3X2_F const mat_d2d = {
				mat._11, mat._12,
				mat._21, mat._22,
				mat._31, mat._32,
			};
			HRGet = dcomp_visual_swap_chain->SetTransform(mat_d2d);
			HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
		}
		else
		{
			auto const mat_d2d = D2D1::Matrix3x2F::Identity();
			HRGet = dcomp_visual_swap_chain->SetTransform(mat_d2d);
			HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
		}

		// 提交

		if (!commitDirectComposition()) return false;

		return true;
	}
	bool SwapChain_D3D11::commitDirectComposition()
	{
		_log("commitDirectComposition");

		HRNew;

		HRGet = dcomp_desktop_device->Commit();
		HRCheckCallReturnBool("IDCompositionDesktopDevice::Commit");
		
		HRGet = dcomp_desktop_device->WaitForCommitCompletion();
		HRCheckCallReturnBool("IDCompositionDesktopDevice::WaitForCommitCompletion");
		
		return true;
	}
	bool SwapChain_D3D11::createCompositionSwapChain(Vector2U size, bool latency_event)
	{
		_log("createCompositionSwapChain");

		// 我们限制 DirectComposition 仅在 Windows 10+ 使用

		HRNew;

		i18n_log_info("[core].SwapChain_D3D11.start_creating_swapchain");

		// 检查组件

		if (!m_window->GetWindow())
		{
			i18n_log_error("[core].SwapChain_D3D11.create_swapchain_failed_null_window");
			assert(false); return false;
		}
		if (!m_device->GetDXGIFactory2()) // 要求平台更新
		{
			i18n_log_error("[core].SwapChain_D3D11.create_swapchain_failed_null_DXGI");
			assert(false); return false;
		}
		if (!m_device->GetD3D11Device())
		{
			i18n_log_error("[core].SwapChain_D3D11.create_swapchain_failed_null_device");
			assert(false); return false;
		}

		// 填充交换链描述

		m_swap_chain_info = getDefaultSwapChainInfo10();
		m_swap_chain_info.Width = size.x;
		m_swap_chain_info.Height = size.y;
		m_swap_chain_info.Scaling = DXGI_SCALING_STRETCH; // DirectComposition 要求这个缩放模式

		if (m_device->IsTearingSupport()) // Windows 10+ 且要求系统支持
		{
			m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // 允许撕裂
		}
		if (latency_event)
		{
			m_swap_chain_info.BufferCount = 3; // 三重缓冲
			m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; // 低延迟渲染
		}

		// 创建交换链

		HRGet = m_device->GetDXGIFactory2()->CreateSwapChainForComposition(
			m_device->GetD3D11Device(),
			&m_swap_chain_info, NULL,
			&dxgi_swapchain);
		HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForComposition");
		
		m_swap_chain_fullscreen_mode = FALSE;

		// 设置最大帧延迟为 1

		HRGet = Platform::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.Get(), 1);
		HRCheckCallReturnBool("IDXGIDevice1::SetMaximumFrameLatency -> 1");
		
		if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
		{
			HANDLE event_handle{};
			HRGet = Platform::DXGI::SetSwapChainMaximumFrameLatency(
				dxgi_swapchain.Get(), 1, &event_handle);
			HRCheckCallReturnBool("IDXGISwapChain2::SetMaximumFrameLatency -> 1");
			dxgi_swapchain_event.Attach(event_handle);
		}

		// 打印信息

		auto refresh_rate_string = i18n("DXGI.DisplayMode.RefreshRate.Desktop");
		std::string_view swapchain_model = i18n("DXGI.SwapChain.SwapEffect.FlipSequential");
		if (m_swap_chain_info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) swapchain_model = i18n("DXGI.SwapChain.SwapEffect.FlipDiscard");
		auto enable_or_disable = [](bool v) -> std::string_view { return v ? i18n("Enable") : i18n("Disable"); };
		i18n_log_info_fmt("[core].SwapChain_D3D11.created_swapchain_info_fmt"
			, size.x, size.y, refresh_rate_string
			, i18n("Disable") // 没有独占全屏
			, swapchain_model
			, enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
			, enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
		);

		// 渲染附件

		if (!createRenderAttachment()) return false;

		// 创建合成器

		if (!createDirectCompositionResources()) return false;

		// 标记

		m_swapchain_want_present_reset = TRUE;

		return true;
	}

	bool SwapChain_D3D11::createSwapChainRenderTarget()
	{
		_log("createSwapChainRenderTarget");

		if (!dxgi_swapchain)
		{
			assert(false); return false;
		}

		if (!m_device->GetD3D11Device())
		{
			assert(false); return false;
		}

		HRNew;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgi_surface;
		HRGet = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface));
		HRCheckCallReturnBool("IDXGISwapChain::GetBuffer -> 0");
		
		// TODO: 线性颜色空间
		HRGet = m_device->GetD3D11Device()->CreateRenderTargetView(dxgi_surface.Get(), NULL, &m_swap_chain_d3d11_rtv);
		HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

		return true;
	}
	void SwapChain_D3D11::destroySwapChainRenderTarget()
	{
		_log("destroySwapChainRenderTarget");

		if (m_device->GetD3D11DeviceContext())
		{
			m_device->GetD3D11DeviceContext()->ClearState();
			m_device->GetD3D11DeviceContext()->Flush();
		}
		m_swap_chain_d3d11_rtv.Reset();
	}
	bool SwapChain_D3D11::createCanvasColorBuffer()
	{
		_log("createCanvasColorBuffer");

		if (m_canvas_size.x == 0 || m_canvas_size.y == 0)
		{
			assert(false); return false;
		}

		if (!m_device->GetD3D11Device())
		{
			assert(false); return false;
		}

		HRNew;

		// Color Buffer

		D3D11_TEXTURE2D_DESC cb_info = {};
		cb_info.Width = m_canvas_size.x;
		cb_info.Height = m_canvas_size.y;
		cb_info.MipLevels = 1;
		cb_info.ArraySize = 1;
		cb_info.Format = COLOR_BUFFER_FORMAT;
		cb_info.SampleDesc.Count = 1;
		cb_info.Usage = D3D11_USAGE_DEFAULT;
		cb_info.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> cb_texture;
		HRGet = m_device->GetD3D11Device()->CreateTexture2D(&cb_info, NULL, &cb_texture);
		HRCheckCallReturnBool("ID3D11Device::CreateTexture2D");

		// Shader Resource

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_info = {};
		srv_info.Format = cb_info.Format; // TODO: 线性颜色空间
		srv_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_info.Texture2D.MipLevels = cb_info.MipLevels;

		HRGet = m_device->GetD3D11Device()->CreateShaderResourceView(cb_texture.Get(), &srv_info, &m_canvas_d3d11_srv);
		HRCheckCallReturnBool("ID3D11Device::CreateShaderResourceView");

		// Render Target

		D3D11_RENDER_TARGET_VIEW_DESC rtv_info = {};
		rtv_info.Format = cb_info.Format; // TODO: 线性颜色空间
		rtv_info.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		HRGet = m_device->GetD3D11Device()->CreateRenderTargetView(cb_texture.Get(), &rtv_info, &m_canvas_d3d11_rtv);
		HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

		return true;
	}
	void SwapChain_D3D11::destroyCanvasColorBuffer()
	{
		_log("destroyCanvasColorBuffer");

		m_canvas_d3d11_srv.Reset();
		m_canvas_d3d11_rtv.Reset();
	}
	bool SwapChain_D3D11::createCanvasDepthStencilBuffer()
	{
		_log("createCanvasDepthStencilBuffer");

		if (m_canvas_size.x == 0 || m_canvas_size.y == 0)
		{
			assert(false); return false;
		}

		if (!m_device->GetD3D11Device())
		{
			assert(false); return false;
		}

		HRNew;

		// Depth Stencil Buffer

		D3D11_TEXTURE2D_DESC ds_info = {};
		ds_info.Width = m_canvas_size.x;
		ds_info.Height = m_canvas_size.y;
		ds_info.MipLevels = 1;
		ds_info.ArraySize = 1;
		ds_info.Format = DEPTH_BUFFER_FORMAT;
		ds_info.SampleDesc.Count = 1;
		ds_info.Usage = D3D11_USAGE_DEFAULT;
		ds_info.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> ds_texture;
		HRGet = m_device->GetD3D11Device()->CreateTexture2D(&ds_info, NULL, &ds_texture);
		HRCheckCallReturnBool("ID3D11Device::CreateTexture2D");

		// Depth Stencil View

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_info = {};
		dsv_info.Format = ds_info.Format;
		dsv_info.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		HRGet = m_device->GetD3D11Device()->CreateDepthStencilView(ds_texture.Get(), &dsv_info, &m_canvas_d3d11_dsv);
		HRCheckCallReturnBool("ID3D11Device::CreateDepthStencilView");

		return true;
	}
	void SwapChain_D3D11::destroyCanvasDepthStencilBuffer()
	{
		_log("destroyCanvasDepthStencilBuffer");

		m_canvas_d3d11_dsv.Reset();
	}
	bool SwapChain_D3D11::createRenderAttachment()
	{
		_log("createRenderAttachment");

		if (!createSwapChainRenderTarget()) return false;
		if (m_is_composition_mode)
		{
			// 此时画布颜色缓冲区就是交换链的后台缓冲区
			m_canvas_d3d11_srv.Reset(); // 不使用
			m_canvas_d3d11_rtv = m_swap_chain_d3d11_rtv;
		}
		else
		{
			if (!createCanvasColorBuffer()) return false;
		}
		if (!createCanvasDepthStencilBuffer()) return false;
		return true;
	}
	void SwapChain_D3D11::destroyRenderAttachment()
	{
		_log("destroyRenderAttachment");

		destroyCanvasDepthStencilBuffer();
		destroyCanvasColorBuffer();
		destroySwapChainRenderTarget();
	}
	void SwapChain_D3D11::applyRenderAttachment()
	{
		//_log("applyRenderAttachment");

		if (auto* ctx = m_device->GetD3D11DeviceContext())
		{
			ID3D11RenderTargetView* rtvs[1] = { m_canvas_d3d11_rtv.Get() };
			ctx->OMSetRenderTargets(1, rtvs, m_canvas_d3d11_dsv.Get());
		}
	}
	void SwapChain_D3D11::clearRenderAttachment()
	{
		//_log("clearRenderAttachment");

		if (auto* ctx = m_device->GetD3D11DeviceContext())
		{
			if (m_canvas_d3d11_rtv)
			{
				FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				ctx->ClearRenderTargetView(m_canvas_d3d11_rtv.Get(), clear_color);
			}
			if (m_canvas_d3d11_dsv)
			{
				ctx->ClearDepthStencilView(m_canvas_d3d11_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
			}
			ctx->Flush(); // 让命令立即提交到 GPU
		}
	}

	bool SwapChain_D3D11::updateLetterBoxingRendererTransform()
	{
		_log("updateLetterBoxingRendererTransform");
		assert(m_canvas_d3d11_srv);
		assert(m_swap_chain_d3d11_rtv);

		return m_scaling_renderer.UpdateTransform(
			m_canvas_d3d11_srv.Get(),
			m_swap_chain_d3d11_rtv.Get()
		);
	}
	bool SwapChain_D3D11::presentLetterBoxingRenderer()
	{
		_log("presentLetterBoxingRenderer");
		assert(m_canvas_d3d11_srv);
		assert(m_swap_chain_d3d11_rtv);

		return m_scaling_renderer.Draw(
			m_canvas_d3d11_srv.Get(),
			m_swap_chain_d3d11_rtv.Get(),
			true
		);
	}

	bool SwapChain_D3D11::handleDirectCompositionWindowSize(Vector2U size)
	{
		_log("handleDirectCompositionWindowSize");

		if (size.x == 0 || size.y == 0)
		{
			assert(false); return false;
		}

		if (!dxgi_swapchain)
		{
			assert(false); return false;
		}
		
		// 此时交换链和画布一致，不应该修改交换链本身，而是修改合成变换

		if (!updateDirectCompositionTransform()) return false;

		return true;
	}
	bool SwapChain_D3D11::handleSwapChainWindowSize(Vector2U size)
	{
		_log("handleSwapChainWindowSize");

		if (size.x == 0 || size.y == 0)
		{
			assert(false); return false;
		}

		if (!dxgi_swapchain)
		{
			assert(false); return false;
		}

		// 此时交换链和画布分离，应该重新调整交换链尺寸

		HRNew;

		destroySwapChainRenderTarget();

		HRGet = dxgi_swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, m_swap_chain_info.Flags);
		HRCheckCallReturnBool("IDXGISwapChain::ResizeBuffers");

		if (!createSwapChainRenderTarget()) return false;

		if (!updateLetterBoxingRendererTransform()) return false;

		return true;
	}

	bool SwapChain_D3D11::setWindowMode(Vector2U size)
	{
		_log("setWindowMode");

		bool const flip_available = m_modern_swap_chain_available;
		bool const disable_composition = Platform::CommandLineArguments::Get().IsOptionExist("--disable-direct-composition");

		if (!disable_composition && flip_available && checkHardwareCompositionSupport(m_device->GetD3D11Device()))
		{
			m_is_composition_mode = true;
			return setCompositionWindowMode(size);
		}
		else
		{
			m_is_composition_mode = false;
		}

		if (size.x < 1 || size.y < 1)
		{
			i18n_log_error_fmt("[core].SwapChain_D3D11.create_swapchain_failed_invalid_size_fmt", size.x, size.y);
			assert(false); return false;
		}
		dispatchEvent(EventType::SwapChainDestroy);
		destroySwapChain();

		if (!m_window->getRedirectBitmapEnable())
		{
			m_window->setRedirectBitmapEnable(true);
			if (!m_window->recreateWindow()) return false;
		}

		m_canvas_size = size;
		if (!createSwapChain(false, {}, false)) // 让它创建渲染附件
		{
			return false;
		}
		m_init = TRUE;
		dispatchEvent(EventType::SwapChainCreate);

		if (!updateLetterBoxingRendererTransform()) return false;

		return true;
	}
	bool SwapChain_D3D11::setCompositionWindowMode(Vector2U size)
	{
		_log("setCompositionWindowMode");

		// 销毁旧交换链

		dispatchEvent(EventType::SwapChainDestroy);
		destroySwapChain();

		// 如果有重定向表面，则去除

		if (m_window->getRedirectBitmapEnable())
		{
			m_window->setRedirectBitmapEnable(false);
			if (!m_window->recreateWindow())
				return false;
		}

		// 创建交换链

		bool latency_event = true;
		m_canvas_size = size;
		if (!createCompositionSwapChain(size, latency_event))
		{
			return false;
		}

		// 更新数据

		m_init = TRUE;

		// 通知各个组件交换链已重新创建

		dispatchEvent(EventType::SwapChainCreate);

		return true;
	}

	bool SwapChain_D3D11::setCanvasSize(Vector2U size)
	{
		_log("setCanvasSize");

		if (size.x == 0 || size.y == 0)
		{
			i18n_log_error_fmt("[core].SwapChain_D3D11.resize_canvas_failed_invalid_size_fmt",
				size.x, size.y);
			assert(false); return false;
		}

		m_canvas_size = size;

		// TODO: 如果尺寸没变，是不是可以直接返回？

		if (!dxgi_swapchain)
		{
			return true; // 当交换链还未初始化时，仅保存画布尺寸
		}

		dispatchEvent(EventType::SwapChainDestroy);

		if (m_is_composition_mode)
		{
			// 对于合成交换链，由于交换链和画布是一致的，所以要调整交换链尺寸

			destroyRenderAttachment();

			HRNew;
			HRGet = dxgi_swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, m_swap_chain_info.Flags);
			HRCheckCallReturnBool("IDXGISwapChain::ResizeBuffers");

			if (!createRenderAttachment()) return false;

			if (!updateDirectCompositionTransform()) return false;
		}
		else
		{
			// 对于普通交换链，由于画布是独立的，只需重新创建画布

			destroyCanvasDepthStencilBuffer();
			destroyCanvasColorBuffer();

			if (!createCanvasColorBuffer()) return false;
			if (!createCanvasDepthStencilBuffer()) return false;

			if (!updateLetterBoxingRendererTransform()) return false;
		}

		dispatchEvent(EventType::SwapChainCreate);

		if (!m_is_composition_mode && m_swap_chain_fullscreen_mode)
		{
			// TODO: LuaSTG 那边会先调用 setCanvasSize 再调用 setWindowMode 触发两次交换链创建
			// 重新选择合适的独占全屏模式
			leaveExclusiveFullscreen();
			enterExclusiveFullscreen();
		}

		return true;
	}

	void SwapChain_D3D11::waitFrameLatency()
	{
		if (m_swapchain_want_present_reset)
		{
			m_swapchain_want_present_reset = FALSE;
			waitFrameLatency(1000, true);
		}
		else
		{
			waitFrameLatency(1000, false);
		}
	}
	void SwapChain_D3D11::setVSync(bool enable)
	{
		m_swap_chain_vsync = enable;
	}
	bool SwapChain_D3D11::present()
	{
		HRESULT hr = S_OK;

		// 手动合成画面的情况下，通过内接缩放渲染器来缩放显示

		if (!m_is_composition_mode)
		{
			if (!presentLetterBoxingRenderer()) return false;
		}
		m_device->GetD3D11DeviceContext()->Flush(); // 立即提交命令到 GPU
		
		// 呈现

		// TODO: 这里应该使用FLIP交换链的立即丢弃机制吗
		// 对于 FLIP 交换链来说，呈现间隔为 0 代表的是丢弃旧帧，而不是取消垂直同步，所以可以利用其来实现较低延迟的三重缓冲机制
		UINT interval = 0;
		if (m_swap_chain_info.SwapEffect != DXGI_SWAP_EFFECT_FLIP_DISCARD && m_swap_chain_info.SwapEffect != DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL && m_swap_chain_vsync)
		{
			interval = 1;
		}
		UINT flags = 0;
		if (interval == 0 && !m_swap_chain_vsync && (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING))
		{
			flags |= DXGI_PRESENT_ALLOW_TEARING;
		}
		hr = gHR = dxgi_swapchain->Present(interval, flags);

		// 清空渲染状态并丢弃内容

		m_device->GetD3D11DeviceContext()->ClearState();
		m_device->GetD3D11DeviceContext1()->DiscardView(m_swap_chain_d3d11_rtv.Get());
		
		// 检查结果

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// 设备丢失
			return m_device->handleDeviceLost();
		}
		else if (FAILED(hr))
		{
			i18n_core_system_call_report_error("IDXGISwapChain::Present");
			return false;
		}

		// 检查 DXGI 工厂有效性

		if (!m_device->validateDXGIFactory())
			return false;

		return true;
	}

	bool SwapChain_D3D11::saveSnapshotToFile(StringView path)
	{
		std::wstring wpath(utf8::to_wstring(path));

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource;
		m_canvas_d3d11_rtv->GetResource(&d3d11_resource);

		hr = gHR = DirectX::SaveWICTextureToFile(
			m_device->GetD3D11DeviceContext(),
			d3d11_resource.Get(),
			GUID_ContainerFormatJpeg,
			wpath.c_str(),
			&GUID_WICPixelFormat24bppBGR);
		if (FAILED(hr))
		{
			i18n_core_system_call_report_error("DirectX::SaveWICTextureToFile");
			return false;
		}

		return true;
	}

	SwapChain_D3D11::SwapChain_D3D11(Window_Win32* p_window, Device_D3D11* p_device)
		: m_window(p_window)
		, m_device(p_device)
	{
		assert(p_window);
		assert(p_device);
		m_modern_swap_chain_available = checkModernSwapChainModelAvailable(m_device->GetD3D11Device());
		m_scaling_renderer.AttachDevice(m_device->GetD3D11Device());
		m_window->addEventListener(this);
		m_device->addEventListener(this);
	}
	SwapChain_D3D11::~SwapChain_D3D11()
	{
		m_window->removeEventListener(this);
		m_device->removeEventListener(this);
		destroySwapChain();
		m_scaling_renderer.DetachDevice();
		m_modern_swap_chain_available = false;
		assert(m_eventobj.size() == 0);
		assert(m_eventobj_late.size() == 0);
	}

	bool SwapChain_D3D11::create(Window_Win32* p_window, Device_D3D11* p_device, SwapChain_D3D11** pp_swapchain)
	{
		try
		{
			*pp_swapchain = new SwapChain_D3D11(p_window, p_device);
			return true;
		}
		catch (...)
		{
			*pp_swapchain = nullptr;
			return false;
		}
	}

	bool ISwapChain::create(IWindow* p_window, IDevice* p_device, ISwapChain** pp_swapchain)
	{
		try
		{
			*pp_swapchain = new SwapChain_D3D11(dynamic_cast<Window_Win32*>(p_window), dynamic_cast<Device_D3D11*>(p_device));
			return true;
		}
		catch (...)
		{
			*pp_swapchain = nullptr;
			return false;
		}
	}
}
