#include "Core/Graphics/SwapChain_D3D11.hpp"
#include "Core/Graphics/Format_D3D11.hpp"
#include "Core/i18n.hpp"
#include "utility/encoding.hpp"

#include "ScreenGrab11.h"

namespace Core::Graphics
{
	inline bool compare_DXGI_MODE_DESC_main(DXGI_MODE_DESC const& a, DXGI_MODE_DESC const& b)
	{
		return a.Width == b.Width
			&& a.Height == b.Height
			&& a.RefreshRate.Numerator == b.RefreshRate.Numerator
			&& a.RefreshRate.Denominator == b.RefreshRate.Denominator
			&& a.Format == b.Format
			;
	}
	inline bool is_DXGI_MODE_equal(DXGI_MODE_DESC const& a, DXGI_MODE_DESC const& b)
	{
		return a.Width == b.Width
			&& a.Height == b.Height
			&& a.RefreshRate.Numerator == b.RefreshRate.Numerator
			&& a.RefreshRate.Denominator == b.RefreshRate.Denominator
			&& a.Format == b.Format
			&& a.ScanlineOrdering == b.ScanlineOrdering
			&& a.Scaling == b.Scaling
			;
	}
	inline bool is_DXGI_MODE_better(DXGI_MODE_DESC const& a, DXGI_MODE_DESC const& b)
	{
		if (a.ScanlineOrdering != b.ScanlineOrdering && a.Scaling != b.Scaling)
		{
			if (b.ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE && b.Scaling == DXGI_MODE_SCALING_UNSPECIFIED)
				return true; // 逐行扫描 + 自动缩放 更好
		}
		else if (a.ScanlineOrdering != b.ScanlineOrdering)
		{
			if (b.ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE)
				return true; // 逐行扫描 更好
		}
		else if (a.Scaling != b.Scaling)
		{
			if (b.Scaling == DXGI_MODE_SCALING_UNSPECIFIED)
				return true; // 自动缩放 更好
		}
		return false; // 没有更好的点
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
		if (m_init) // 曾经设置过
		{
			if (m_swapchain_last_windowed)
				setWindowMode(m_swapchain_last_mode.width, m_swapchain_last_mode.height, m_swapchain_last_flip);
			else
				setExclusiveFullscreenMode(m_swapchain_last_mode);
		}
	}
	void SwapChain_D3D11::onDeviceDestroy()
	{
		destroySwapChain();
	}

	void SwapChain_D3D11::onWindowCreate()
	{
		//onDeviceCreate();
	}
	void SwapChain_D3D11::onWindowDestroy()
	{
		//onDeviceDestroy();
	}

	void SwapChain_D3D11::onWindowActive()
	{
		m_window_active_changed.store(0x1);
	}
	void SwapChain_D3D11::onWindowInactive()
	{
		m_window_active_changed.store(0x2);
	}

	void SwapChain_D3D11::destroySwapChain()
	{
		destroyRenderAttachment();
		if (dxgi_swapchain)
		{
			BOOL bFullscreen = FALSE;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
			if (bFullscreen)
			{
				spdlog::info("[core] 尝试退出独占全屏");
				hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
				if (FAILED(hr))
				{
					spdlog::error("[core] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
				}
			}
		}
		dxgi_swapchain_event.Close();
		dxgi_swapchain.Reset();
	}
	bool SwapChain_D3D11::createSwapChain(bool windowed, bool flip, DisplayMode const& mode, bool no_attachment)
	{
		HRESULT hr = 0;

		spdlog::info("[core] 开始创建 SwapChain");

		if (!m_window->GetWindow())
		{
			spdlog::error("[core] 无法创建 SwapChain，窗口为空");
			assert(false); return false;
		}
		auto* dxgi_factory = m_device->GetDXGIFactory1();
		if (!dxgi_factory)
		{
			spdlog::error("[core] 无法创建 SwapChain，DXGI 组件为空");
			assert(false); return false;
		}
		auto* d3d11_device = m_device->GetD3D11Device();
		if (!d3d11_device)
		{
			spdlog::error("[core] 无法创建 SwapChain，图形设备为空");
			assert(false); return false;
		}
		
		// Windows 7 平台更新已安装，或者 Windows 8 及以上
		
		if (auto* dxgi_factory2 = m_device->GetDXGIFactory2())
		{
			DXGI_SWAP_CHAIN_DESC1 desc1 = {
				.Width = mode.width,
				.Height = mode.height,
				.Format = m_swapchain_format,
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
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC descf = {};
			if (!windowed)
			{
				desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 允许修改显示器显示模式
				descf.RefreshRate.Numerator = mode.refresh_rate.numerator;
				descf.RefreshRate.Denominator = mode.refresh_rate.denominator;
				descf.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
				descf.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				descf.Windowed = TRUE; // 创建交换链后暂时保持在窗口化状态
			}
			if (windowed && flip)
			{
				// 仅限窗口模式
				if (m_device->IsTearingSupport()) // Windows 10 + 系统支持
				{
					desc1.BufferCount = 2;
					desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
					desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
					desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				}
				else if (m_device->IsFlipDiscardSupport()) // Windows 10
				{
					desc1.BufferCount = 2;
					desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
					desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				}
				else if (m_device->IsFrameLatencySupport()) // Windows 8.1
				{
					desc1.BufferCount = 2;
					desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
					desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				}
				else if (m_device->IsFlipSequentialSupport()) // Windows 8
				{
					desc1.BufferCount = 2;
					desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				}
			}
			m_swapchain_buffer_count = desc1.BufferCount;
			m_swapchain_flags = desc1.Flags;
			Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swapchain1;
			hr = gHR = dxgi_factory2->CreateSwapChainForHwnd(d3d11_device, m_window->GetWindow(), &desc1, windowed ? NULL : &descf, NULL, &dxgi_swapchain1);
			if (SUCCEEDED(hr))
			{
				hr = gHR = dxgi_swapchain1.As(&dxgi_swapchain);
				if (FAILED(hr))
				{
					i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain1::QueryInterface -> IDXGISwapChain");
					assert(false); return false;
				}
			}
			else
			{
				i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIFactory2::CreateSwapChainForHwnd");
			}
		}

		// 回落到 Windows 7 的方式创建

		if (!dxgi_swapchain)
		{
			DXGI_MODE_DESC modedef = {
				.Width = mode.width,
				.Height = mode.height,
				.RefreshRate = DXGI_RATIONAL{
					.Numerator = mode.refresh_rate.numerator,
					.Denominator = mode.refresh_rate.denominator,
				},
				.Format = m_swapchain_format,
				.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
				.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
			};
			DXGI_SWAP_CHAIN_DESC desc = {
				.BufferDesc = modedef,
				.SampleDesc = DXGI_SAMPLE_DESC{
					.Count = 1,
					.Quality = 0,
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = 1,
				.OutputWindow = m_window->GetWindow(),
				.Windowed = TRUE, // 创建交换链后暂时保持在窗口化状态
				.SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
				.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
			};
			m_swapchain_buffer_count = desc.BufferCount;
			m_swapchain_flags = desc.Flags;
			hr = gHR = dxgi_factory->CreateSwapChain(d3d11_device, &desc, &dxgi_swapchain);
			if (FAILED(hr))
			{
				i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIFactory1::CreateSwapChain");
				assert(false); return false;
			}
		}

		// 关闭傻逼快捷键

		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory_sw; // 这里他妈有坑，新创建的 DXGI 工厂和交换链内部的的不是同一个
		hr = gHR = dxgi_swapchain->GetParent(IID_PPV_ARGS(&dxgi_factory_sw));
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::GetParent -> IDXGIFactory1");
			assert(false); return false;
		}
		hr = gHR = dxgi_factory_sw->MakeWindowAssociation(m_window->GetWindow(), DXGI_MWA_NO_ALT_ENTER); // 别他妈乱切换了
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIFactory1::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");
			assert(false); return false;
		}

		// 设置最大帧延迟为 1

		Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device1;
		hr = gHR = d3d11_device->QueryInterface(IID_PPV_ARGS(&dxgi_device1));
		if (SUCCEEDED(hr))
		{
			hr = gHR = dxgi_device1->SetMaximumFrameLatency(1);
			if (FAILED(hr))
			{
				i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIDevice1::SetMaximumFrameLatency -> 1");
				assert(false); return false;
			}
		}

		Microsoft::WRL::ComPtr<IDXGISwapChain2> dxgi_swapchain2;
		hr = gHR = dxgi_swapchain.As(&dxgi_swapchain2);
		if (SUCCEEDED(hr) && (m_swapchain_flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT))
		{
			hr = gHR = dxgi_swapchain2->SetMaximumFrameLatency(1);
			if (FAILED(hr))
			{
				i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain2::SetMaximumFrameLatency -> 1");
				assert(false); return false;
			}
			dxgi_swapchain_event.Attach(dxgi_swapchain2->GetFrameLatencyWaitableObject());
			if (!dxgi_swapchain_event.IsValid())
			{
				i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain2::GetFrameLatencyWaitableObject");
				assert(false); return false;
			}
		}

		spdlog::info("[core] 已创建 SwapChain");

		// 渲染附件

		if (!no_attachment)
		{
			if (!createRenderAttachment())
				return false;
			applyRenderAttachment();
		}
		
		return true;
	}
	void SwapChain_D3D11::destroyRenderAttachment()
	{
		if (auto* ctx = m_device->GetD3D11DeviceContext())
		{
			ctx->ClearState();
			ctx->Flush();
		}
		d3d11_rtv.Reset();
		d3d11_dsv.Reset();
	}
	bool SwapChain_D3D11::createRenderAttachment()
	{
		HRESULT hr = 0;

		spdlog::info("[core] 开始创建 RenderAttachment");
		if (!dxgi_swapchain)
		{
			spdlog::info("[core] 无法创建 RenderAttachment，交换链为空");
			return false;
		}
		auto* d3d11_device = m_device->GetD3D11Device();
		assert(d3d11_device);
		if (!d3d11_device)
		{
			spdlog::info("[core] 无法创建 RenderAttachment，图形设备为空");
			return false;
		}

		// 渲染目标

		Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgi_surface;
		hr = gHR = dxgi_swapchain->GetBuffer(0, IID_ID3D11Texture2D, &dxgi_surface);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::GetBuffer -> 0");
			return false;
		}
		// TODO: 线性颜色空间自动转 sRGB
		//D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {
		//	.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		//	.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		//	.Texture2D = {
		//		.MipSlice = 0,
		//	},
		//};
		hr = gHR = d3d11_device->CreateRenderTargetView(dxgi_surface.Get(), NULL, &d3d11_rtv);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "ID3D11Device::CreateRenderTargetView");
			return false;
		}

		// 深度、模板缓冲区

		D3D11_TEXTURE2D_DESC bkbuf_info = {};
		dxgi_surface->GetDesc(&bkbuf_info);
		D3D11_TEXTURE2D_DESC dsbuf_info = {
			.Width = bkbuf_info.Width,
			.Height = bkbuf_info.Height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = m_ds_format,
			.SampleDesc = DXGI_SAMPLE_DESC{
				.Count = 1,
				.Quality = 0,
			},
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
		};
		Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
		hr = gHR = d3d11_device->CreateTexture2D(&dsbuf_info, NULL, &d3d11_texture2d);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "ID3D11Device::CreateTexture2D");
			return false;
		}
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_info = {
			.Format = dsbuf_info.Format,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
			.Flags = 0,
			.Texture2D = D3D11_TEX2D_DSV{
				.MipSlice = 0,
			},
		};
		hr = gHR = d3d11_device->CreateDepthStencilView(d3d11_texture2d.Get(), &dsv_info, &d3d11_dsv);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "ID3D11Device::CreateDepthStencilView");
			return false;
		}
		
		spdlog::info("[core] 已创建 RenderAttachment");

		return true;
	}
	void SwapChain_D3D11::applyRenderAttachment()
	{
		if (auto* ctx = m_device->GetD3D11DeviceContext())
		{
			ID3D11RenderTargetView* rtvs[1] = { d3d11_rtv.Get() };
			ctx->OMSetRenderTargets(1, rtvs, d3d11_dsv.Get());
		}
	}
	void SwapChain_D3D11::clearRenderAttachment()
	{
		if (auto* ctx = m_device->GetD3D11DeviceContext())
		{
			if (d3d11_rtv)
			{
				FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				ctx->ClearRenderTargetView(d3d11_rtv.Get(), clear_color);
			}
			if (d3d11_dsv)
			{
				ctx->ClearDepthStencilView(d3d11_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
			}
		}
	}

	bool SwapChain_D3D11::refreshDisplayMode()
	{
		m_displaymode.clear();

		HRESULT hr = S_OK;

		spdlog::info("[core] 开始枚举支持的显示模式");

		// 获取关联的显示输出

		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::GetContainingOutput");
			return false;
		}

		// 获取所有显示模式

		UINT mode_count = 0;
		hr = gHR = dxgi_output->GetDisplayModeList(m_swapchain_format, 0, &mode_count, NULL);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIOutput::GetDisplayModeList -> N");
			return false;
		}

		std::vector<DXGI_MODE_DESC> modes(mode_count);
		hr = gHR = dxgi_output->GetDisplayModeList(m_swapchain_format, 0, &mode_count, modes.data());
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIOutput::GetDisplayModeList");
			return false;
		}

		// 剔除过低的分辨率
		bool remove_small_size = false;
		for (auto& v : modes)
		{
			if (v.Width >= 640 && v.Height >= 360)
			{
				remove_small_size = true;
				break;
			}
		}
		if (remove_small_size)
		{
			for (auto it = modes.begin(); it != modes.end();)
			{
				if (!(it->Width >= 640 && it->Height >= 360))
					it = modes.erase(it);
				else
					it++;
			}
		}
		
		// 剔除过低的刷新率
		bool remove_low_refresh_rate = false;
		for (auto& v : modes)
		{
			if (((double)v.RefreshRate.Numerator / (double)v.RefreshRate.Denominator) >= 58.5)
			{
				remove_low_refresh_rate = true;
				break;
			}
		}
		if (remove_low_refresh_rate)
		{
			for (auto it = modes.begin(); it != modes.end();)
			{
				if (((double)it->RefreshRate.Numerator / (double)it->RefreshRate.Denominator) < 58.5)
					it = modes.erase(it);
				else
					it++;
			}
		}

		// 剔除隔行扫描
		bool remove_scanline_x_field = false;
		for (auto& v : modes)
		{
			if (v.ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED || v.ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE)
			{
				remove_scanline_x_field = true;
				break;
			}
		}
		if (remove_scanline_x_field)
		{
			for (auto it = modes.begin(); it != modes.end();)
			{
				if (!(it->ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED || it->ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE))
					it = modes.erase(it);
				else
					it++;
			}
		}

		// 合并扫描、缩放模式，优先选择逐行扫描和自动缩放模式
		if constexpr (true)
		{
			for (size_t i = 0; i < modes.size(); i += 1)
			{
				for (auto it = (modes.begin() + i + 1); it != modes.end();)
				{
					if (compare_DXGI_MODE_DESC_main(modes[i], *it))
					{
						if (is_DXGI_MODE_better(modes[i], *it))
							modes[i] = *it;
						it = modes.erase(it);
					}
					else
					{
						it++;
					}
				}
			}
		}
		
		// 生成结果
		m_displaymode.reserve(modes.size());
		for (auto& v : modes)
		{
			m_displaymode.emplace_back(DisplayMode{
				.width = v.Width,
				.height = v.Height,
				.refresh_rate = Rational(v.RefreshRate.Numerator, v.RefreshRate.Denominator),
				.format = convert_DXGI_FORMAT_to_Format(v.Format),
			});
		}

		// 打印结果
		if (!m_displaymode.empty())
		{
			spdlog::info("[core] 共找到 {} 个支持的显示模式：", m_displaymode.size());
			for (size_t i = 0; i < m_displaymode.size(); i += 1)
			{
				spdlog::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz"
					, i
					, m_displaymode[i].width, m_displaymode[i].height
					, (double)m_displaymode[i].refresh_rate.numerator / (double)m_displaymode[i].refresh_rate.denominator
				);
			}
		}
		else
		{
			spdlog::error("[core] 找不到可用的显示模式");
			return false;
		}

		return true;
	}
	uint32_t SwapChain_D3D11::getDisplayModeCount()
	{
		return (uint32_t)m_displaymode.size();
	}
	DisplayMode SwapChain_D3D11::getDisplayMode(uint32_t index)
	{
		assert(!m_displaymode.empty() && index < m_displaymode.size());
		return m_displaymode[index];
	}
	bool SwapChain_D3D11::findBestMatchDisplayMode(DisplayMode& mode)
	{
		if (mode.width < 1 || mode.height < 1)
		{
			spdlog::error("[core] 无法匹配大小为 ({}x{}) 的显示模式", mode.width, mode.height);
			return false;
		}

		auto* d3d11_device = m_device->GetD3D11Device();
		if (!d3d11_device)
		{
			spdlog::error("[core] 无法匹配显示模式，图形设备为空");
			assert(false); return false;
		}
		if (!dxgi_swapchain)
		{
			spdlog::error("[core] 无法匹配显示模式，交换链为空");
			assert(false); return false;
		}

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::GetContainingOutput");
			spdlog::error("[core] 独占全屏不可用");
			return false;
		}

		// 初次匹配
		DXGI_MODE_DESC target_mode = {
			.Width = mode.width,
			.Height = mode.height,
			.RefreshRate = DXGI_RATIONAL{
				.Numerator = (mode.refresh_rate.numerator != 0) ? mode.refresh_rate.numerator : 60,
				.Denominator = (mode.refresh_rate.denominator != 0) ? mode.refresh_rate.denominator : 1,
			},
			.Format = m_swapchain_format,
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
		};
		DXGI_MODE_DESC dxgi_mode = {};
		hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &dxgi_mode, d3d11_device);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIOutput::FindClosestMatchingMode");
			return false;
		}

		// 检查刷新率，如果太低的话就继续往上匹配
		UINT const numerator_candidate[] = { 120, 180, 240, 300, 360, 420, 480 };
		for (auto const& v : numerator_candidate)
		{
			if (((double)dxgi_mode.RefreshRate.Numerator / (double)dxgi_mode.RefreshRate.Denominator) > 59.5f)
			{
				break;
			}
			target_mode.RefreshRate = DXGI_RATIONAL{ .Numerator = v,.Denominator = 1 };
			hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &dxgi_mode, d3d11_device);
			if (FAILED(hr))
			{
				i18n_log_error_fmt("[core].system_call_failed_f", "IDXGIOutput::FindClosestMatchingMode");
				return false;
			}
		}

		mode.refresh_rate.numerator = dxgi_mode.RefreshRate.Numerator;
		mode.refresh_rate.denominator = dxgi_mode.RefreshRate.Denominator;

		return true;
	}

	bool SwapChain_D3D11::setWindowMode(uint32_t width, uint32_t height, bool flip_model)
	{
		// TODO: 也许，是时候该自动开启 flip 交换链模型了？
		//if (m_device->IsFrameLatencySupport() && m_device->IsFlipDiscardSupport() && m_device->IsTearingSupport())
		//{
		//	flip_model = true;
		//}
		if (width < 1 || height < 1)
		{
			spdlog::error("[core] 无法创建 SwapChain，大小不能为 ({}x{})", width, height);
			return false;
		}
		dispatchEvent(EventType::SwapChainDestroy);
		destroySwapChain();
		DisplayMode mode = {
			.width = width,
			.height = height,
			.refresh_rate = Rational(),
			.format = Format::B8G8R8A8_UNORM,
		};
		if (!createSwapChain(true, flip_model, mode, false)) // 让它创建渲染附件
		{
			return false;
		}
		m_swapchain_last_mode = mode;
		m_swapchain_last_windowed = TRUE;
		m_swapchain_last_flip = flip_model;
		m_init = TRUE;
		//m_window->setCursorToRightBottom();
		dispatchEvent(EventType::SwapChainCreate);
		return true;
	}
	bool SwapChain_D3D11::setSize(uint32_t width, uint32_t height)
	{
		//if (!m_swapchain_last_windowed)
		//{
		//	spdlog::error("[core] 无法直接更改独占全屏 SwapChain 的大小");
		//	return false;
		//}
		if (width < 1 || height < 1)
		{
			spdlog::error("[core] 无法更改 SwapChain 的大小，大小不能为 ({}x{})", width, height);
			return false;
		}
		if (width == uint32_t(-1)) width = m_swapchain_last_mode.width;
		if (height == uint32_t(-1)) height = m_swapchain_last_mode.height;
		if (!dxgi_swapchain)
		{
			spdlog::error("[core] 无法更改 SwapChain 的大小，交换链为空");
			assert(false); return false;
		}
		dispatchEvent(EventType::SwapChainDestroy);
		destroyRenderAttachment();

		HRESULT hr = S_OK;
		hr = gHR = dxgi_swapchain->ResizeBuffers(m_swapchain_buffer_count, width, height, m_swapchain_format, m_swapchain_flags);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::ResizeBuffers");
			assert(false); return false;
		}

		if (!createRenderAttachment())
		{
			return false;
		}
		applyRenderAttachment();
		m_swapchain_last_mode.width = width;
		m_swapchain_last_mode.height = height;
		//m_window->setCursorToRightBottom();
		dispatchEvent(EventType::SwapChainCreate);
		return true;
	}
	bool SwapChain_D3D11::setExclusiveFullscreenMode(DisplayMode const& mode)
	{
		if (mode.width < 1 || mode.height < 1)
		{
			spdlog::error("[core] 无法创建 SwapChain，大小不能为 ({}x{})", mode.width, mode.height);
			return false;
		}
		dispatchEvent(EventType::SwapChainDestroy);
		destroySwapChain();
		if (!createSwapChain(false, false, mode, true)) // 稍后创建渲染附件
		{
			return false;
		}
		// 进入全屏
		spdlog::info("[core] 尝试切换到独占全屏");
		HRESULT hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::SetFullscreenState -> TRUE");
			return false;
		}
		// 需要重设交换链大小（特别是 Flip 交换链模型）
		hr = gHR = dxgi_swapchain->ResizeBuffers(m_swapchain_buffer_count, mode.width, mode.height, m_swapchain_format, m_swapchain_flags);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::ResizeBuffers");
			assert(false); return false;
		}
		// 创建渲染附件
		if (!createRenderAttachment())
		{
			return false;
		}
		applyRenderAttachment();
		m_swapchain_last_mode = mode;
		m_swapchain_last_windowed = FALSE;
		m_swapchain_last_flip = FALSE;
		m_init = TRUE;
		//m_window->setCursorToRightBottom();
		dispatchEvent(EventType::SwapChainCreate);
		return true;
	}
	
	void SwapChain_D3D11::syncWindowActive()
	{
		int window_active_changed = m_window_active_changed.exchange(0);
		if (window_active_changed & 0x1)
		{
			if (!m_swapchain_last_windowed && dxgi_swapchain)
			{
				BOOL bFSC = FALSE;
				Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
				HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFSC, &dxgi_output);
				if (SUCCEEDED(hr))
				{
					if (!bFSC)
					{
						spdlog::info("[core] 尝试切换到独占全屏");
						hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
						if (FAILED(hr))
						{
							i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::SetFullscreenState -> TRUE");
						}
						else
						{
							setSize(uint32_t(-1), uint32_t(-1));
						}
					}
				}
				else
				{
					i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::GetFullscreenState");
				}
			}
		}
		else if (window_active_changed & 0x2)
		{
			if (!m_swapchain_last_windowed && dxgi_swapchain)
			{
				BOOL bFSC = FALSE;
				Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
				HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFSC, &dxgi_output);
				if (SUCCEEDED(hr))
				{
					if (bFSC)
					{
						spdlog::info("[core] 尝试退出独占全屏");
						hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
						if (FAILED(hr))
						{
							i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::SetFullscreenState -> FALSE");
						}
						else
						{
							setSize(uint32_t(-1), uint32_t(-1));
						}
					}
				}
				else
				{
					i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::GetFullscreenState");
				}
			}
			m_window->setLayer(WindowLayer::Normal);
		}
	}
	void SwapChain_D3D11::waitFrameLatency()
	{
		if (dxgi_swapchain_event.IsValid() && (m_swapchain_flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT))
		{
			DWORD const result = WaitForSingleObjectEx(dxgi_swapchain_event.Get(), 1000, TRUE);
			if (!(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT))
			{
				gHRLastError;
				i18n_log_error_fmt("[core].system_call_failed_f", "WaitForSingleObjectEx");
			}
		}
	}
	void SwapChain_D3D11::setVSync(bool enable)
	{
		m_swapchain_vsync = enable;
	}
	bool SwapChain_D3D11::present()
	{
		HRESULT hr = S_OK;

		// 恢复默认

		applyRenderAttachment();

		// 呈现

		UINT const interval = m_swapchain_vsync ? 1 : 0;
		UINT const flags = (!m_swapchain_vsync && (m_swapchain_flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)) ? DXGI_PRESENT_ALLOW_TEARING : 0;
		hr = gHR = dxgi_swapchain->Present(interval, flags);

		// 取消绑定并丢弃内容

		ID3D11RenderTargetView* rtvs[1] = { NULL };
		m_device->GetD3D11DeviceContext()->OMSetRenderTargets(1, rtvs, NULL);
		if (auto* ctx1 = m_device->GetD3D11DeviceContext1())
		{
			ctx1->DiscardView(d3d11_rtv.Get());
			ctx1->DiscardView(d3d11_dsv.Get());
		}

		// 检查结果

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// 设备丢失
			return m_device->handleDeviceLost();
		}
		else if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDXGISwapChain::Present");
			return false;
		}

		// 检查 DXGI 工厂有效性

		if (!m_device->validateDXGIFactory())
			return false;

		return true;
	}

	bool SwapChain_D3D11::saveSnapshotToFile(StringView path)
	{
		std::wstring wpath(utility::encoding::to_wide(path));
		HRESULT hr = S_OK;
		Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource;
		d3d11_rtv->GetResource(&d3d11_resource);
		hr = gHR = DirectX::SaveWICTextureToFile(
			m_device->GetD3D11DeviceContext(),
			d3d11_resource.Get(),
			GUID_ContainerFormatJpeg,
			wpath.c_str(),
			&GUID_WICPixelFormat24bppBGR);
		return SUCCEEDED(hr);
	}

	SwapChain_D3D11::SwapChain_D3D11(Window_Win32* p_window, Device_D3D11* p_device)
		: m_window(p_window)
		, m_device(p_device)
	{
		m_window->addEventListener(this);
		m_device->addEventListener(this);
	}
	SwapChain_D3D11::~SwapChain_D3D11()
	{
		m_window->removeEventListener(this);
		m_device->removeEventListener(this);
		destroySwapChain();
		assert(m_eventobj.size() == 0);
		assert(m_eventobj_late.size() == 0);
	}

	bool SwapChain_D3D11::create(Window_Win32* p_window, Device_D3D11* p_device, SwapChain_D3D11** pp_swapchain)
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
