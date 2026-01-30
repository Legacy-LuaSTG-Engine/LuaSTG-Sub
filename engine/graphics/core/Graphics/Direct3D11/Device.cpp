#include "core/Graphics/Direct3D11/Device.hpp"
#include "core/Configuration.hpp"
#include "core/Logger.hpp"
#include "d3d11/GraphicsDeviceManager.hpp"
#include "d3d11/ToStringHelper.hpp"
#include "d3d11/DeviceHelper.hpp"
#include "windows/WindowsVersion.hpp"
#include "windows/AdapterPolicy.hpp"
#include "windows/Direct3D11.hpp"
#include "utf8.hpp"

namespace {
	using std::string_view_literals::operator ""sv;

	std::string multi_plane_overlay_flags_to_string(UINT const flags) {
		std::string buffer;
		if (flags & DXGI_OVERLAY_SUPPORT_FLAG_DIRECT) {
			buffer.append("Direct");
		}
		if (flags & DXGI_OVERLAY_SUPPORT_FLAG_SCALING) {
			if (!buffer.empty()) buffer.append("、");
			buffer.append("Scaling");
		}
		if (buffer.empty()) {
			buffer.append("None");
		}
		return buffer;
	};
	std::string hardware_composition_flags_to_string(UINT const flags) {
		std::string buffer;
		if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN) {
			buffer.append("Fullscreen");
		}
		if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED) {
			if (!buffer.empty()) buffer.append("、");
			buffer.append("Windowed");
		}
		if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED) {
			if (!buffer.empty()) buffer.append("、");
			buffer.append("CursorStretched");
		}
		if (buffer.empty()) {
			buffer.append("None");
		}
		return buffer;
	};
	std::string_view rotation_to_string(DXGI_MODE_ROTATION const rot) {
		switch (rot) {
		default:
		case DXGI_MODE_ROTATION_UNSPECIFIED: return "Unspecified";
		case DXGI_MODE_ROTATION_IDENTITY: return "Identity";
		case DXGI_MODE_ROTATION_ROTATE90: return "Rotate90";
		case DXGI_MODE_ROTATION_ROTATE180: return "Rotate180";
		case DXGI_MODE_ROTATION_ROTATE270: return "Rotate270";
		}
	};
}

// Device
namespace core::Graphics::Direct3D11 {
	Device::Device(std::string_view const& preferred_gpu)
		: preferred_adapter_name(preferred_gpu) {
		// 创建图形组件

		Logger::info("[core] [GraphicsDevice] initializing...");

		testAdapterPolicy();
		if (!createDXGI())
			throw std::runtime_error("create basic DXGI components failed");
		if (!createD3D11())
			throw std::runtime_error("create basic D3D11 components failed");
		if (!createWIC())
			throw std::runtime_error("create basic WIC components failed");
#ifdef LUASTG_ENABLE_DIRECT2D
		if (!createD2D1())
			throw std::runtime_error("create basic D2D1 components failed");
#endif
		if (!createDWrite())
			throw std::runtime_error("create basic DWrite components failed");

		Logger::info("[core] [GraphicsDevice] initialization complete");
	}
	Device::~Device() {
		// 清理对象
		destroyDWrite(); // 长生存期
#ifdef LUASTG_ENABLE_DIRECT2D
		destroyD2D1();
#endif
		destroyWIC(); // 长生存期
		destroyD3D11();
		destroyDXGI();
		assert(m_eventobj.size() == 0);
		assert(m_eventobj_late.size() == 0);
	}

	bool Device::selectAdapter() {
		Logger::info("[core] [GraphicsDevice] enumerate physical devices"sv);

		win32::com_ptr<IDXGIAdapter1> adapter;
		std::string adapter_name;
		core::GraphicsDeviceManagerDXGI::refreshAndFindAdapter(preferred_adapter_name, adapter.put(), &adapter_name);
		core::GraphicsDeviceManager::log();

		if (adapter) {
			if (!win32::check_hresult_as_boolean(
				adapter->GetParent(IID_PPV_ARGS(dxgi_factory.put())),
				"IDXGIObject::GetParent (IDXGIFactory2)"sv
			)) {
				return false;
			}
			dxgi_adapter = adapter;
			dxgi_adapter_name = adapter_name;
		}

		dxgi_adapter_name_list.clear();
		for (uint32_t i = 0; i < core::GraphicsDeviceManager::getPhysicalDeviceCount(); i += 1) {
			dxgi_adapter_name_list.emplace_back(core::GraphicsDeviceManager::getPhysicalDeviceName(i));
		}

		if (!dxgi_adapter) {
			Logger::error("[core] [GraphicsDevice] no physical device available");
			return false;
		}

		//if (!link_to_output) {
		//	Logger::warn(
		//		"[core] [GraphicsDevice] {} doesn't appear to connect to any display output, which could result in:\n"
		//		"    Buffer is copied over PCI-E when in exclusive fullscreen\n"
		//		"    Desktop Window Manager takes over desktop composition when in exclusive fullscreen\n"
		//		"    Degraded performance and increased frame latency"
		//	);
		//}

		Logger::info("[core] [GraphicsDevice] current physical device: {}", dxgi_adapter_name);
		return true;
	}
	bool Device::createDXGI() {
		if (!selectAdapter()) {
			if (!core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
				return false;
			}
		}
		return true;
	}
	void Device::destroyDXGI() {
		dxgi_factory.reset();
		dxgi_adapter.reset();

		dxgi_adapter_name.clear();
		dxgi_adapter_name_list.clear();
	}
	bool Device::createD3D11() {
		HRESULT hr = S_OK;

		// 创建

		if (dxgi_adapter) {
			d3d11::createDevice(
				dxgi_adapter.get(),
				d3d11_device.put(), d3d11_devctx.put(), &d3d_feature_level
			);
		}
		else if (core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
			D3D_DRIVER_TYPE driver_type{};
			if (d3d11::createSoftwareDevice(
				d3d11_device.put(), d3d11_devctx.put(), &d3d_feature_level,
				&driver_type
			)) {
				Logger::info("[core] [GraphicsDevice] software device type: {}", toStringView(driver_type));
			}
		}

		if (!d3d11_device) {
			Logger::error("[core] [GraphicsDevice] no device available");
			return false;
		}

		if (!dxgi_adapter) {
			hr = gHR = Platform::Direct3D11::GetDeviceAdater(d3d11_device.get(), dxgi_adapter.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: Platform::Direct3D11::GetDeviceAdater");
				return false;
			}
		}

		// 特性检查

		d3d11::logDeviceFeatureSupportDetails(d3d11_device.get());
		d3d11::logDeviceFormatSupportDetails(d3d11_device.get());

		hr = gHR = d3d11_device->QueryInterface(d3d11_device1.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: ID3D11Device::QueryInterface -> ID3D11Device1");
			// 不是严重错误
		}
		hr = gHR = d3d11_devctx->QueryInterface(d3d11_devctx1.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: ID3D11DeviceContext::QueryInterface -> ID3D11DeviceContext1");
			// 不是严重错误
		}

		struct D3DX11_FEATURE_DATA_FORMAT_SUPPORT {
			DXGI_FORMAT InFormat;
			UINT OutFormatSupport;
			UINT OutFormatSupport2;
		};
		auto check_format_support = [&](DXGI_FORMAT const format, std::string_view const& name) ->D3DX11_FEATURE_DATA_FORMAT_SUPPORT {
			std::string name1("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_FORMAT_SUPPORT ("); name1.append(name); name1.append(")");
			std::string name2("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_FORMAT_SUPPORT2 ("); name2.append(name); name2.append(")");

			D3D11_FEATURE_DATA_FORMAT_SUPPORT data1 = { .InFormat = format };
			HRESULT const hr1 = gHR = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data1, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT));
			if (FAILED(hr1)) Logger::error("Windows API failed: {}", name1);

			D3D11_FEATURE_DATA_FORMAT_SUPPORT2 data2 = { .InFormat = format };
			HRESULT const hr2 = gHR = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &data2, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT2));
			if (FAILED(hr2)) Logger::error("Windows API failed: {}", name2);

			return D3DX11_FEATURE_DATA_FORMAT_SUPPORT{ .InFormat = format, .OutFormatSupport = data1.OutFormatSupport, .OutFormatSupport2 = data2.OutFormatSupport2 };
		};
		auto check_format_support_1 = [&](DXGI_FORMAT const format, std::string_view const& name) ->D3DX11_FEATURE_DATA_FORMAT_SUPPORT {
			std::string name1("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_FORMAT_SUPPORT ("); name1.append(name); name1.append(")");

			D3D11_FEATURE_DATA_FORMAT_SUPPORT data1 = { .InFormat = format };
			HRESULT const hr1 = gHR = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data1, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT));
			if (FAILED(hr1)) Logger::error("Windows API failed: {}", name1);

			D3D11_FEATURE_DATA_FORMAT_SUPPORT2 data2 = { .InFormat = format };
			d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &data2, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT2));

			return D3DX11_FEATURE_DATA_FORMAT_SUPPORT{ .InFormat = format, .OutFormatSupport = data1.OutFormatSupport, .OutFormatSupport2 = data2.OutFormatSupport2 };
		};

	#define _CHECK_FORMAT_SUPPORT(_NAME, _FORMAT) \
		D3DX11_FEATURE_DATA_FORMAT_SUPPORT d3d11_feature_format_##_NAME = check_format_support(_FORMAT, #_FORMAT);
	#define _CHECK_FORMAT_SUPPORT_1(_NAME, _FORMAT) \
		D3DX11_FEATURE_DATA_FORMAT_SUPPORT d3d11_feature_format_##_NAME = check_format_support_1(_FORMAT, #_FORMAT);

		_CHECK_FORMAT_SUPPORT(rgba32, DXGI_FORMAT_R8G8B8A8_UNORM);
		_CHECK_FORMAT_SUPPORT(rgba32_srgb, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		_CHECK_FORMAT_SUPPORT(bgra32, DXGI_FORMAT_B8G8R8A8_UNORM);
		_CHECK_FORMAT_SUPPORT(bgra32_srgb, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
		_CHECK_FORMAT_SUPPORT_1(d24_s8, DXGI_FORMAT_D24_UNORM_S8_UINT);

	#undef _CHECK_FORMAT_SUPPORT

		if (
			(d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D)
			&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER)
			&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP)
			&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET)
			&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_BLENDABLE)
			&& (d3d11_feature_format_bgra32.OutFormatSupport & D3D11_FORMAT_SUPPORT_DISPLAY)
			) {
			// 确实支持
		}
		else {
			Logger::warn("[core] [GraphicsDevice] This device does not fully support the B8G8R8A8 format");
		}

		testMultiPlaneOverlay();

		tracy_context = tracy_d3d11_context_create(d3d11_device.get(), d3d11_devctx.get());

		return true;
	}
	void Device::destroyD3D11() {
		tracy_d3d11_context_destroy(tracy_context);
		tracy_context = nullptr;

		d3d_feature_level = D3D_FEATURE_LEVEL_10_0;

		d3d11_device.reset();
		d3d11_device1.reset();
		d3d11_devctx.reset();
		d3d11_devctx1.reset();
	}
	bool Device::createWIC() {
		HRESULT hr = S_OK;

		hr = gHR = CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(wic_factory2.put()));
		if (SUCCEEDED(hr)) {
			hr = gHR = wic_factory2->QueryInterface(wic_factory.put());
			assert(SUCCEEDED(hr));
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IWICImagingFactory2::QueryInterface -> IWICImagingFactory");
				return false;
			}
		}
		else {
			Logger::error("Windows API failed: CoCreateInstance -> IWICImagingFactory2");
			// 没有那么严重，来再一次
			hr = gHR = CoCreateInstance(CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(wic_factory.put()));
			assert(SUCCEEDED(hr));
			if (FAILED(hr)) {
				Logger::error("Windows API failed: CoCreateInstance -> IWICImagingFactory");
				return false;
			}
		}

		return true;
	}
	void Device::destroyWIC() {
		wic_factory.reset();
		wic_factory2.reset();
	}
#ifdef LUASTG_ENABLE_DIRECT2D
	bool Device::createD2D1() {
		HRESULT hr = S_OK;

		hr = gHR = d2d1_loader.CreateFactory(
			D2D1_FACTORY_TYPE_MULTI_THREADED,
			IID_PPV_ARGS(d2d1_factory.put()));
		if (FAILED(hr)) {
			Logger::error("Windows API failed: D2D1CreateFactory -> ID2D1Factory1");
			assert(false); return false;
		}

		win32::com_ptr<IDXGIDevice> dxgi_device;
		hr = gHR = d3d11_device->QueryInterface(dxgi_device.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: ID3D11Device::QueryInterface -> IDXGIDevice");
			assert(false); return false;
		}

		hr = gHR = d2d1_factory->CreateDevice(dxgi_device.get(), d2d1_device.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: ID2D1Factory1::CreateDevice");
			assert(false); return false;
		}

		hr = gHR = d2d1_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d1_devctx.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: ID2D1Device::CreateDeviceContext");
			assert(false); return false;
		}

		return true;
	}
	void Device::destroyD2D1() {
		d2d1_factory.reset();
		d2d1_device.reset();
		d2d1_devctx.reset();
	}
#endif
	bool Device::createDWrite() {
		HRESULT hr = S_OK;

		hr = gHR = dwrite_loader.CreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_PPV_ARGS(dwrite_factory.put()));
		if (FAILED(hr)) {
			Logger::error("Windows API failed: DWriteCreateFactory -> DWRITE_FACTORY_TYPE_SHARED");
			return false;
		}

		return true;
	}
	void Device::destroyDWrite() {
		dwrite_factory.reset();
	}
	bool Device::doDestroyAndCreate() {
		dispatchEvent(EventType::DeviceDestroy);

		//destroyDWrite(); // 长生存期
#ifdef LUASTG_ENABLE_DIRECT2D
		destroyD2D1();
#endif
		//destroyWIC(); // 长生存期
		destroyD3D11();
		destroyDXGI();

		testAdapterPolicy();

		if (!createDXGI()) return false;
		if (!createD3D11()) return false;
		//if (!createWIC()) return false; // 长生存期
#ifdef LUASTG_ENABLE_DIRECT2D
		if (!createD2D1()) return false;
#endif
		//if (!createDWrite()) return false; // 长生存期

		dispatchEvent(EventType::DeviceCreate);

		return true;
	}
	bool Device::testAdapterPolicy() {
		if (preferred_adapter_name.empty()) {
			return true; // default GPU
		}

		auto testStage = [](const std::string& name, const bool nv, const bool amd) -> bool {
			Platform::AdapterPolicy::setNVIDIA(nv);
			Platform::AdapterPolicy::setAMD(amd);
			if (!core::GraphicsDeviceManager::refresh()) {
				return false;
			}
			if (core::GraphicsDeviceManager::getPhysicalDeviceCount() == 0) {
				return false;
			}
			return core::GraphicsDeviceManager::getPhysicalDeviceName(0) == name;
		};

		// Stage 1 - Disable and test
		if (testStage(preferred_adapter_name, false, false)) return true;
		// Stage 2 - Enable and test
		if (testStage(preferred_adapter_name, true, true)) return true;
		// Stage 3 - NVIDIA and test
		if (testStage(preferred_adapter_name, true, false)) return true;
		// Stage 4 - AMD and test
		if (testStage(preferred_adapter_name, false, true)) return true;
		// Stage 5 - Disable and failed
		Platform::AdapterPolicy::setAll(false);

		return false;
	}
	bool Device::testMultiPlaneOverlay() {
		HRESULT hr = S_OK;

		win32::com_ptr<IDXGIAdapter1> adapter_;
		for (UINT i = 0; SUCCEEDED(dxgi_factory->EnumAdapters1(i, adapter_.put())); i += 1) {
			win32::com_ptr<IDXGIOutput> output_;
			for (UINT j = 0; SUCCEEDED(adapter_->EnumOutputs(j, output_.put())); j += 1) {
				DXGI_OUTPUT_DESC desc = {};
				hr = gHR = output_->GetDesc(&desc);
				if (FAILED(hr)) {
					Logger::error("Windows API failed: IDXGIOutput3::CheckOverlaySupport -> DXGI_FORMAT_B8G8R8A8_UNORM");
					assert(false); return false;
				}

				BOOL overlay_ = FALSE;
				win32::com_ptr<IDXGIOutput2> output2_;
				if (win32::check_hresult_as_boolean(output_->QueryInterface(output2_.put()))) {
					overlay_ = output2_->SupportsOverlays();
				}

				UINT overlay_flags = 0;
				win32::com_ptr<IDXGIOutput3> output3_;
				if (win32::check_hresult_as_boolean(output_->QueryInterface(output3_.put()))) {
					hr = gHR = output3_->CheckOverlaySupport(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						d3d11_device.get(),
						&overlay_flags);
					if (FAILED(hr)) {
						Logger::error("Windows API failed: IDXGIOutput3::CheckOverlaySupport -> DXGI_FORMAT_B8G8R8A8_UNORM");
					}
				}

				UINT composition_flags = 0;
				win32::com_ptr<IDXGIOutput6> output6_;
				if (win32::check_hresult_as_boolean(output_->QueryInterface(output6_.put()))) {
					hr = gHR = output6_->CheckHardwareCompositionSupport(&composition_flags);
					if (FAILED(hr)) {
						Logger::error("Windows API failed: IDXGIOutput6::CheckHardwareCompositionSupport");
					}
				}

				Logger::info(
					"[core] [GraphicsDevice] physical device {} -- display output {}:\n"
					"    Attached To Desktop: {}\n"
					"    Desktop Coordinates: ({}, {}) ({} x {})\n"
					"    Rotation: {}\n"
					"    Multi-Plane Overlay: {}\n"
					"    Multi-Plane Overlay Feature: {}\n"
					"    Hardware Composition: {}"
					, i, j
					, desc.AttachedToDesktop ? "True" : "False"
					, desc.DesktopCoordinates.left
					, desc.DesktopCoordinates.top
					, desc.DesktopCoordinates.right - desc.DesktopCoordinates.left
					, desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top
					, rotation_to_string(desc.Rotation)
					, overlay_ ? "support" : "not support"
					, multi_plane_overlay_flags_to_string(overlay_flags)
					, hardware_composition_flags_to_string(composition_flags)
				);
			}
		}

		return true;
	}

	bool Device::handleDeviceLost() {
		if (d3d11_device) {
			gHR = d3d11_device->GetDeviceRemovedReason();
		}
		return doDestroyAndCreate();
	}

	void Device::dispatchEvent(EventType t) {
		// 回调
		m_is_dispatch_event = true;
		switch (t) {
		case EventType::DeviceCreate:
			for (auto& v : m_eventobj) {
				if (v) v->onDeviceCreate();
			}
			break;
		case EventType::DeviceDestroy:
			for (auto& v : m_eventobj) {
				if (v) v->onDeviceDestroy();
			}
			break;
		}
		m_is_dispatch_event = false;
		// 处理那些延迟的对象
		removeEventListener(nullptr);
		for (auto& v : m_eventobj_late) {
			m_eventobj.emplace_back(v);
		}
		m_eventobj_late.clear();
	}

	void Device::addEventListener(IDeviceEventListener* e) {
		removeEventListener(e);
		if (m_is_dispatch_event) {
			m_eventobj_late.emplace_back(e);
		}
		else {
			m_eventobj.emplace_back(e);
		}
	}
	void Device::removeEventListener(IDeviceEventListener* e) {
		if (m_is_dispatch_event) {
			for (auto& v : m_eventobj) {
				if (v == e) {
					v = nullptr; // 不破坏遍历过程
				}
			}
		}
		else {
			for (auto it = m_eventobj.begin(); it != m_eventobj.end();) {
				if (*it == e)
					it = m_eventobj.erase(it);
				else
					it++;
			}
		}
	}

	DeviceMemoryUsageStatistics Device::getMemoryUsageStatistics() {
		DeviceMemoryUsageStatistics data = {};
		win32::com_ptr<IDXGIAdapter3> adapter;
		if (win32::check_hresult_as_boolean(dxgi_adapter->QueryInterface(adapter.put()))) {
			DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
			if (win32::check_hresult_as_boolean(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info))) {
				data.local.budget = info.Budget;
				data.local.current_usage = info.CurrentUsage;
				data.local.available_for_reservation = info.AvailableForReservation;
				data.local.current_reservation = info.CurrentReservation;
			}
			if (win32::check_hresult_as_boolean(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info))) {
				data.non_local.budget = info.Budget;
				data.non_local.current_usage = info.CurrentUsage;
				data.non_local.available_for_reservation = info.AvailableForReservation;
				data.non_local.current_reservation = info.CurrentReservation;
			}
		}
		return data;
	}

	bool Device::recreate() {
		return doDestroyAndCreate();
	}

	bool Device::create(StringView preferred_gpu, Device** p_device) {
		try {
			*p_device = new Device(preferred_gpu);
			return true;
		}
		catch (...) {
			*p_device = nullptr;
			return false;
		}
	}
}
namespace core::Graphics {
	bool IDevice::create(StringView preferred_gpu, IDevice** p_device) {
		try {
			*p_device = new Direct3D11::Device(preferred_gpu);
			return true;
		}
		catch (...) {
			*p_device = nullptr;
			return false;
		}
	}
}
