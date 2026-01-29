#include "core/Graphics/Direct3D11/Device.hpp"
#include "core/Configuration.hpp"
#include "core/Logger.hpp"
#include "windows/WindowsVersion.hpp"
#include "windows/AdapterPolicy.hpp"
#include "windows/Direct3D11.hpp"
#include "utf8.hpp"

namespace {
	std::string bytes_count_to_string(DWORDLONG size) {
		int count = 0;
		char buffer[64] = {};
		if (size < 1024llu) // B
		{
			count = std::snprintf(buffer, 64, "%u B", (unsigned int)size);
		}
		else if (size < (1024llu * 1024llu)) // KB
		{
			count = std::snprintf(buffer, 64, "%.2f KiB", (double)size / 1024.0);
		}
		else if (size < (1024llu * 1024llu * 1024llu)) // MB
		{
			count = std::snprintf(buffer, 64, "%.2f MiB", (double)size / (1024.0 * 1024.0));
		}
		else // GB
		{
			count = std::snprintf(buffer, 64, "%.2f GiB", (double)size / (1024.0 * 1024.0 * 1024.0));
		}
		return std::string(buffer, count);
	}
	std::string_view adapter_flags_to_string(UINT const flags) {
		if ((flags & DXGI_ADAPTER_FLAG_REMOTE)) {
			if (flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				return "Remote Software";
			}
			else {
				return "Remote Hardware";
			}
		}
		else {
			if (flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				return "Software";
			}
			else {
				return "Hardware";
			}
		}
	}
	std::string_view d3d_feature_level_to_string(D3D_FEATURE_LEVEL level) {
		switch (level) {
		case D3D_FEATURE_LEVEL_12_2: return "12.2";
		case D3D_FEATURE_LEVEL_12_1: return "12.1";
		case D3D_FEATURE_LEVEL_12_0: return "12.0";
		case D3D_FEATURE_LEVEL_11_1: return "11.1";
		case D3D_FEATURE_LEVEL_11_0: return "11.0";
		case D3D_FEATURE_LEVEL_10_1: return "10.1";
		case D3D_FEATURE_LEVEL_10_0: return "10.0";
		case D3D_FEATURE_LEVEL_9_3: return "9.3";
		case D3D_FEATURE_LEVEL_9_2: return "9.2";
		case D3D_FEATURE_LEVEL_9_1: return "9.1";
		default: return "Unknown";
		}
	}
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
	std::string_view threading_feature_to_string(D3D11_FEATURE_DATA_THREADING const v) {
		if (v.DriverConcurrentCreates) {
			if (v.DriverCommandLists) {
				return "DriverConcurrentCreates, DriverCommandLists";
			}
			else {
				return "DriverConcurrentCreates";
			}
		}
		else {
			if (v.DriverCommandLists) {
				return "DriverCommandLists";
			}
			else {
				return "None";
			}
		}
	};
	std::string_view d3d_feature_level_to_maximum_texture2d_size_string(D3D_FEATURE_LEVEL const level) {
		switch (level) {
		case D3D_FEATURE_LEVEL_12_2:
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "16384x16384";
		case D3D_FEATURE_LEVEL_10_1:
		case D3D_FEATURE_LEVEL_10_0:
			return "8192x8192";
		case D3D_FEATURE_LEVEL_9_3:
			return "4096x4096";
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_1:
		default:
			return "2048x2048";
		}
	}
	std::string_view renderer_architecture_to_string(BOOL const TileBasedDeferredRenderer) {
		if (TileBasedDeferredRenderer)
			return "Tile Based Deferred Renderer (TBDR)";
		else
			return "Immediate Mode Rendering (IMR)";
	}
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

	bool Device::createDXGIFactory() {
		HRESULT hr = S_OK;

		// 创建 1.2 的组件，强制要求平台更新
		hr = gHR = dxgi_loader.CreateFactory(IID_PPV_ARGS(dxgi_factory.put()));
		if (FAILED(hr)) {
			Logger::error("Windows API failed: CreateDXGIFactory2 -> IDXGIFactory2");
			assert(false); return false;
		}

		return true;
	}
	void Device::destroyDXGIFactory() {
		dxgi_factory.reset();
	}
	bool Device::selectAdapter() {
		HRESULT hr = S_OK;

		// 枚举所有图形设备

		Logger::info("[core] [GraphicsDevice] enumerate physical devices");

		struct AdapterCandidate {
			win32::com_ptr<IDXGIAdapter1> adapter;
			std::string adapter_name;
			DXGI_ADAPTER_DESC1 adapter_info;
			D3D_FEATURE_LEVEL feature_level;
			BOOL link_to_output;
		};
		std::vector<AdapterCandidate> adapter_candidate;

		win32::com_ptr<IDXGIAdapter1> dxgi_adapter_temp;
		for (UINT idx = 0; SUCCEEDED(dxgi_factory->EnumAdapters1(idx, dxgi_adapter_temp.put())); idx += 1) {
			// 检查此设备是否支持 Direct3D 11 并获取特性级别
			bool supported_d3d11 = false;
			D3D_FEATURE_LEVEL level_info = D3D_FEATURE_LEVEL_10_0;
			hr = gHR = d3d11_loader.CreateDeviceFromAdapter(
				dxgi_adapter_temp.get(),
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				D3D_FEATURE_LEVEL_10_0,
				NULL,
				&level_info,
				NULL);
			if (SUCCEEDED(hr)) {
				supported_d3d11 = true;
			}

			// 获取图形设备信息
			std::string dev_name = "<NULL>";
			DXGI_ADAPTER_DESC1 desc_ = {};
			if (win32::check_hresult_as_boolean(dxgi_adapter_temp->GetDesc1(&desc_))) {
				bool soft_dev_type = (desc_.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) || (desc_.Flags & DXGI_ADAPTER_FLAG_REMOTE);
				dev_name = utf8::to_string(desc_.Description);
				Logger::info(
					"[core] [GraphicsDevice] physical device {}:\n"
					"    Description: {}\n"
					"    Direct3D Feature Level: {}\n"
					"    Type: {}\n"
					"    Dedicated Video Memory: {}\n"
					"    Dedicated System Memory: {}\n"
					"    Shared System Memory: {}\n"
					"    Vendor ID: 0x{:08X}\n"
					"    Device ID: 0x{:08X}\n"
					"    SubSystem ID: 0x{:08X}\n"
					"    Revision: 0x{:08X}\n"
					"    Adapter LUID：{:08X}-{:08X}"
					, idx
					, dev_name
					, d3d_feature_level_to_string(level_info)
					, adapter_flags_to_string(desc_.Flags)
					, bytes_count_to_string(desc_.DedicatedVideoMemory)
					, bytes_count_to_string(desc_.DedicatedSystemMemory)
					, bytes_count_to_string(desc_.SharedSystemMemory)
					, desc_.VendorId
					, desc_.DeviceId
					, desc_.SubSysId
					, desc_.Revision
					, static_cast<DWORD>(desc_.AdapterLuid.HighPart), desc_.AdapterLuid.LowPart
				);
				if (soft_dev_type) {
					supported_d3d11 = false; // 排除软件或远程设备
				}
			}
			else {
				Logger::error("Windows API failed: IDXGIAdapter1::GetDesc1");
				Logger::error("[core] [Graphics Device] physical device {}: <Unable to read information>", idx);
				supported_d3d11 = false; // 排除未知错误
			}

			// 枚举显示输出
			bool has_linked_output = false;
			win32::com_ptr<IDXGIOutput> dxgi_output_temp;
			for (UINT odx = 0; SUCCEEDED(dxgi_adapter_temp->EnumOutputs(odx, dxgi_output_temp.put())); odx += 1) {
				win32::com_ptr<IDXGIOutput6> dxgi_output_temp6;
				hr = gHR = dxgi_output_temp->QueryInterface(dxgi_output_temp6.put());
				if (FAILED(hr)) {
					Logger::error("Windows API failed: IDXGIOutput::QueryInterface -> IDXGIOutput6");
					// 不是严重错误
				}

				DXGI_OUTPUT_DESC1 o_desc = {};
				bool read_o_desc = false;
				UINT comp_sp_flags = 0;

				if (dxgi_output_temp6) {
					if (!win32::check_hresult_as_boolean(dxgi_output_temp6->CheckHardwareCompositionSupport(&comp_sp_flags))) {
						comp_sp_flags = 0;
						Logger::error("Windows API failed: IDXGIOutput6::CheckHardwareCompositionSupport");
					}
					if (win32::check_hresult_as_boolean(dxgi_output_temp6->GetDesc1(&o_desc))) {
						read_o_desc = true;
					}
					else {
						Logger::error("Windows API failed: IDXGIOutput6::GetDesc1");
					}
				}
				if (!read_o_desc) {
					DXGI_OUTPUT_DESC desc_0_ = {};
					if (win32::check_hresult_as_boolean(dxgi_output_temp->GetDesc(&desc_0_))) {
						std::memcpy(o_desc.DeviceName, desc_0_.DeviceName, sizeof(o_desc.DeviceName));
						o_desc.DesktopCoordinates = desc_0_.DesktopCoordinates;
						o_desc.AttachedToDesktop = desc_0_.AttachedToDesktop;
						o_desc.Rotation = desc_0_.Rotation;
						o_desc.Monitor = desc_0_.Monitor;
						read_o_desc = true;
					}
					else {
						Logger::error("Windows API failed: IDXGIOutput::GetDesc");
					}
				}

				if (read_o_desc) {
					Logger::info(
						"[core] [GraphicsDevice] physical device {} -- display output {}:\n"
						"    Attached To Desktop: {}\n"
						"    Desktop Coordinates: ({}, {}) ({} x {})\n"
						"    Rotation: {}\n"
						"    Hardware Composition: {}"
						, idx, odx
						, o_desc.AttachedToDesktop ? "True" : "False"
						, o_desc.DesktopCoordinates.left
						, o_desc.DesktopCoordinates.top
						, o_desc.DesktopCoordinates.right - o_desc.DesktopCoordinates.left
						, o_desc.DesktopCoordinates.bottom - o_desc.DesktopCoordinates.top
						, rotation_to_string(o_desc.Rotation)
						, hardware_composition_flags_to_string(comp_sp_flags)
					);
					has_linked_output = true;
				}
				else {
					Logger::error("[core] [GraphicsDevice] physical device {} -- display output {}: <Unable to read information>", idx, odx);
				}
			}
			dxgi_output_temp.reset();

			// 加入候选列表
			if (supported_d3d11) {
				adapter_candidate.emplace_back(AdapterCandidate{
					.adapter = dxgi_adapter_temp,
					.adapter_name = std::move(dev_name),
					.adapter_info = desc_,
					.feature_level = level_info,
					.link_to_output = has_linked_output,
											   });
			}
		}
		dxgi_adapter_temp.reset();

		// 选择图形设备

		BOOL link_to_output = false;
		for (auto& v : adapter_candidate) {
			if (v.adapter_name == preferred_adapter_name) {
				dxgi_adapter = v.adapter;
				dxgi_adapter_name = v.adapter_name;
				link_to_output = v.link_to_output;
				break;
			}
		}
		if (!dxgi_adapter && !adapter_candidate.empty()) {
			auto& v = adapter_candidate[0];
			dxgi_adapter = v.adapter;
			dxgi_adapter_name = v.adapter_name;
			link_to_output = v.link_to_output;
		}
		dxgi_adapter_name_list.clear();
		for (auto& v : adapter_candidate) {
			dxgi_adapter_name_list.emplace_back(std::move(v.adapter_name));
		}
		adapter_candidate.clear();

		// 获取图形设备

		if (dxgi_adapter) {
			Logger::info("[core] [GraphicsDevice] current physical device: {}", dxgi_adapter_name);
			if (!link_to_output) {
				Logger::warn(
					"[core] [GraphicsDevice] {} doesn't appear to connect to any display output, which could result in:\n"
					"    Buffer is copied over PCI-E when in exclusive fullscreen\n"
					"    Desktop Window Manager takes over desktop composition when in exclusive fullscreen\n"
					"    Degraded performance and increased frame latency"
				);
			}
		}
		else {
			Logger::error("[core] [GraphicsDevice] no device available");
			return false;
		}

		return true;
	}
	bool Device::createDXGI() {
		HRESULT hr = S_OK;

		// 创建工厂

		if (!createDXGIFactory()) return false;

		// 检测特性支持情况

		win32::com_ptr<IDXGIFactory3> dxgi_factory3;
		hr = gHR = dxgi_factory->QueryInterface(dxgi_factory3.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: IDXGIFactory1::QueryInterface -> IDXGIFactory3");
			// 不是严重错误
		}

		win32::com_ptr<IDXGIFactory4> dxgi_factory4;
		hr = gHR = dxgi_factory->QueryInterface(dxgi_factory4.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: IDXGIFactory1::QueryInterface -> IDXGIFactory4");
			// 不是严重错误
		}

		win32::com_ptr<IDXGIFactory5> dxgi_factory5;
		hr = gHR = dxgi_factory->QueryInterface(dxgi_factory5.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: IDXGIFactory1::QueryInterface -> IDXGIFactory5");
			// 不是严重错误
		}

		win32::com_ptr<IDXGIFactory6> dxgi_factory6;
		hr = gHR = dxgi_factory->QueryInterface(dxgi_factory6.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: IDXGIFactory1::QueryInterface -> IDXGIFactory6");
			// 不是严重错误
		}

		win32::com_ptr<IDXGIFactory7> dxgi_factory7;
		hr = gHR = dxgi_factory->QueryInterface(dxgi_factory7.put());
		if (FAILED(hr)) {
			Logger::error("Windows API failed: IDXGIFactory1::QueryInterface -> IDXGIFactory7");
			// 不是严重错误
		}

		if (dxgi_factory5) {
			BOOL value = FALSE;
			hr = gHR = dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &value, sizeof(value));
			if (SUCCEEDED(hr)) {
				dxgi_support_tearing = value;
			}
			else {
				Logger::error("Windows API failed: IDXGIFactory5::CheckFeatureSupport -> DXGI_FEATURE_PRESENT_ALLOW_TEARING");
				// 不是严重错误
			}
		}

		// 打印特性支持情况
		Logger::info(
			"[core] [GraphicsDevice] DXGI components feature support:\n"
			"    Present Allow Tearing: {}"
			, dxgi_support_tearing ? "support" : "not support (requires Windows 10 or above, and hardware support)"
		);

		// 获取适配器

		if (!selectAdapter()) {
			if (!core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
				return false;
			}
		}

		// 检查适配器支持

		if (dxgi_adapter) {
			win32::com_ptr<IDXGIAdapter2> dxgi_adapter2;
			hr = gHR = dxgi_adapter->QueryInterface(dxgi_adapter2.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIAdapter1::QueryInterface -> IDXGIAdapter2");
				// 不是严重错误
			}

			win32::com_ptr<IDXGIAdapter3> dxgi_adapter3;
			hr = gHR = dxgi_adapter->QueryInterface(dxgi_adapter3.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIAdapter1::QueryInterface -> IDXGIAdapter3");
				// 不是严重错误
			}

			win32::com_ptr<IDXGIAdapter2> dxgi_adapter4;
			hr = gHR = dxgi_adapter->QueryInterface(dxgi_adapter4.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIAdapter1::QueryInterface -> IDXGIAdapter4");
				// 不是严重错误
			}
		}

		return true;
	}
	void Device::destroyDXGI() {
		destroyDXGIFactory();
		dxgi_adapter.reset();

		dxgi_adapter_name.clear();
		dxgi_adapter_name_list.clear();

		dxgi_support_tearing = FALSE;
	}
	bool Device::createD3D11() {
		HRESULT hr = S_OK;

		// 创建

		if (dxgi_adapter) {
			hr = gHR = d3d11_loader.CreateDeviceFromAdapter(
				dxgi_adapter.get(),
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				D3D_FEATURE_LEVEL_10_0,
				d3d11_device.put(),
				&d3d_feature_level,
				d3d11_devctx.put());
		}
		else if (core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
			D3D_DRIVER_TYPE d3d_driver_type = D3D_DRIVER_TYPE_UNKNOWN;
			hr = gHR = d3d11_loader.CreateDeviceFromSoftAdapter(
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				D3D_FEATURE_LEVEL_10_0,
				d3d11_device.put(),
				&d3d_feature_level,
				d3d11_devctx.put(),
				&d3d_driver_type);
			if (SUCCEEDED(hr)) {
				switch (d3d_driver_type) {
				case D3D_DRIVER_TYPE_REFERENCE:
					Logger::info("[core] [GraphicsDevice] device type: reference");
					break;
				case D3D_DRIVER_TYPE_SOFTWARE:
					Logger::info("[core] [GraphicsDevice] device type: software");
					break;
				case D3D_DRIVER_TYPE_WARP:
					Logger::info("[core] [GraphicsDevice] device type: Windows Advanced Rasterization Platform (WARP)");
					break;
				}
			}
		}
		if (!d3d11_device) {
			Logger::error("Windows API failed: D3D11CreateDevice");
			return false;
		}
		M_D3D_SET_DEBUG_NAME(d3d11_device.get(), "Device_D3D11::d3d11_device");
		M_D3D_SET_DEBUG_NAME(d3d11_devctx.get(), "Device_D3D11::d3d11_devctx");
		if (!dxgi_adapter) {
			hr = gHR = Platform::Direct3D11::GetDeviceAdater(d3d11_device.get(), dxgi_adapter.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: Platform::Direct3D11::GetDeviceAdater");
				return false;
			}
		}

		// 特性检查

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

		D3D11_FEATURE_DATA_THREADING d3d11_feature_mt = {};
		HRESULT hr_mt = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &d3d11_feature_mt, sizeof(d3d11_feature_mt));
		if (FAILED(hr_mt)) {
			Logger::error("Windows API failed: ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_THREADING");
			// 不是严重错误
		}

		D3D11_FEATURE_DATA_ARCHITECTURE_INFO d3d11_feature_arch = {};
		HRESULT hr_arch = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_ARCHITECTURE_INFO, &d3d11_feature_arch, sizeof(d3d11_feature_arch));
		if (FAILED(hr_arch)) {
			Logger::error("Windows API failed: ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_ARCHITECTURE_INFO");
			// 不是严重错误
		}

		D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11_feature_o2 = {};
		HRESULT hr_o2 = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &d3d11_feature_o2, sizeof(d3d11_feature_o2));
		if (FAILED(hr_o2)) {
			Logger::error("Windows API failed: ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_D3D11_OPTIONS2");
			// 不是严重错误
		}

	#define _FORMAT_INFO_STRING_FMT3 \
		"        用于顶点缓冲区：{}\n"\
		"        创建二维纹理：{}\n"\
		"        创建立方体纹理：{}\n"\
		"        着色器采样：{}\n"\
		"        创建多级渐进纹理：{}\n"\
		"        自动生成多级渐进纹理：{}\n"\
		"        绑定为渲染目标：{}\n"\
		"        像素颜色混合操作：{}\n"\
		"        绑定为深度、模板缓冲区：{}\n"\
		"        被 CPU 锁定、读取：{}\n"\
		"        解析多重采样：{}\n"\
		"        用于显示输出：{}\n"\
		"        创建多重采样渲染目标：{}\n"\
		"        像素颜色逻辑混合操作：{}\n"\
		"        资源可分块：{}\n"\
		"        资源可共享：{}\n"\
		"        多平面叠加：{}\n"

	#define _FORMAT_MAKE_SUPPORT "support" : "not support"

	#define _FORMAT_INFO_STRING_ARG3(_NAME) \
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER        ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D               ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURECUBE             ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE           ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP                     ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN             ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET           ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_BLENDABLE               ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL           ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_CPU_LOCKABLE            ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE     ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_DISPLAY                 ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_TILED                 ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_SHAREABLE             ) ? _FORMAT_MAKE_SUPPORT\
		, (d3d11_feature_format_##_NAME.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_MULTIPLANE_OVERLAY    ) ? _FORMAT_MAKE_SUPPORT

		Logger::info("[core] Direct3D 11 设备功能支持：\n"
					 "    Direct3D 功能级别：{}\n"
					 "    R8G8B8A8 格式：\n"
					 _FORMAT_INFO_STRING_FMT3
					 "    R8G8B8A8 sRGB 格式：\n"
					 _FORMAT_INFO_STRING_FMT3
					 "    B8G8R8A8 格式：\n"
					 _FORMAT_INFO_STRING_FMT3
					 "    B8G8R8A8 sRGB 格式：\n"
					 _FORMAT_INFO_STRING_FMT3
					 "    D24 S8 格式：\n"
					 _FORMAT_INFO_STRING_FMT3
					 "    最大二维纹理尺寸：{}\n"
					 "    多线程架构：{}\n"
					 "    渲染架构：{}\n"
					 "    统一内存架构（UMA）：{}"
					 , d3d_feature_level_to_string(d3d_feature_level)
					 _FORMAT_INFO_STRING_ARG3(rgba32)
					 _FORMAT_INFO_STRING_ARG3(rgba32_srgb)
					 _FORMAT_INFO_STRING_ARG3(bgra32)
					 _FORMAT_INFO_STRING_ARG3(bgra32_srgb)
					 _FORMAT_INFO_STRING_ARG3(d24_s8)
					 , d3d_feature_level_to_maximum_texture2d_size_string(d3d_feature_level)
					 , threading_feature_to_string(d3d11_feature_mt)
					 , renderer_architecture_to_string(d3d11_feature_arch.TileBasedDeferredRenderer)
					 , d3d11_feature_o2.UnifiedMemoryArchitecture ? _FORMAT_MAKE_SUPPORT
		);

	#undef _FORMAT_INFO_STRING_ARG3
	#undef _FORMAT_MAKE_SUPPORT
	#undef _FORMAT_INFO_STRING_FMT3

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
		if (preferred_adapter_name.empty()) return true; // default GPU

		auto testAdapterName = [&]() -> bool {
			HRESULT hr = S_OK;
			win32::com_ptr<IDXGIAdapter1> dxgi_adapter;
			hr = gHR = dxgi_factory->EnumAdapters1(0, dxgi_adapter.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIFactory1::EnumAdapters1 -> #0");
				assert(false); return false;
			}
			DXGI_ADAPTER_DESC1 desc = {};
			hr = gHR = dxgi_adapter->GetDesc1(&desc);
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIAdapter1::GetDesc1");
				assert(false); return false;
			}
			std::string gpu_name(utf8::to_string(desc.Description));
			return preferred_adapter_name == gpu_name;
		};
		auto testStage = [&](bool nv, bool amd) -> bool {
			Platform::AdapterPolicy::setNVIDIA(nv);
			Platform::AdapterPolicy::setAMD(amd);

			if (!createDXGIFactory()) return false;

			bool const result = testAdapterName();

			destroyDXGIFactory();

			return result;
		};

		// Stage 1 - Disable and test
		if (testStage(false, false)) return true;
		// Stage 2 - Enable and test
		if (testStage(true, true)) return true;
		// Stage 3 - NVIDIA and test
		if (testStage(true, false)) return true;
		// Stage 4 - AMD and test
		if (testStage(false, true)) return true;
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
	bool Device::validateDXGIFactory() {
		if (!dxgi_factory->IsCurrent()) {
			HRESULT hr = S_OK;

			dxgi_factory.reset();
			dxgi_adapter.reset();

			win32::com_ptr<IDXGIDevice> dxgi_device;
			hr = gHR = d3d11_device->QueryInterface(dxgi_device.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: ID3D11Device::QueryInterface -> IDXGIDevice");
				return false;
			}

			win32::com_ptr<IDXGIAdapter> dxgi_adapter_tmp;
			hr = gHR = dxgi_device->GetAdapter(dxgi_adapter_tmp.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIDevice::GetAdapter");
				return false;
			}

			hr = gHR = dxgi_adapter_tmp->QueryInterface(dxgi_adapter.put());
			if (FAILED(hr)) {
				Logger::error("Windows API failed: IDXGIAdapter::QueryInterface -> IDXGIAdapter1");
				return false;
			}

			//hr = gHR = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
			//if (FAILED(hr))
			//{
			//	Logger::error("Windows API failed: IDXGIAdapter1::GetParent -> IDXGIFactory2");
			//	return false;
			//}

			// 创建 1.2 的组件，强制要求平台更新
			hr = gHR = dxgi_loader.CreateFactory(IID_PPV_ARGS(dxgi_factory.put()));
			if (FAILED(hr)) {
				Logger::error("Windows API failed: CreateDXGIFactory2 -> IDXGIFactory2");
				assert(false); return false;
			}

			assert(dxgi_factory->IsCurrent());
		}
		return true;
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
