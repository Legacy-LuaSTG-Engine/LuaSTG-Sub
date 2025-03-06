#include "Core/Graphics/Direct3D11/Device.hpp"
#include "core/Configuration.hpp"
#include "Core/i18n.hpp"
#include "Platform/WindowsVersion.hpp"
#include "Platform/AdapterPolicy.hpp"
#include "Platform/Direct3D11.hpp"
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
				return i18n("DXGI_adapter_type_software_remote");
			}
			else {
				return i18n("DXGI_adapter_type_hardware_remote");
			}
		}
		else {
			if (flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				return i18n("DXGI_adapter_type_software");
			}
			else {
				return i18n("DXGI_adapter_type_hardware");
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
		default: return i18n("unknown");
		}
	}
	std::string multi_plane_overlay_flags_to_string(UINT const flags) {
		std::string buffer;
		if (flags & DXGI_OVERLAY_SUPPORT_FLAG_DIRECT) {
			buffer.append("直接呈现");
		}
		if (flags & DXGI_OVERLAY_SUPPORT_FLAG_SCALING) {
			if (!buffer.empty()) buffer.append("、");
			buffer.append("缩放呈现");
		}
		if (buffer.empty()) {
			buffer.append("无");
		}
		return buffer;
	};
	std::string hardware_composition_flags_to_string(UINT const flags) {
		std::string buffer;
		if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN) {
			buffer.append("全屏");
		}
		if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED) {
			if (!buffer.empty()) buffer.append("、");
			buffer.append("窗口");
		}
		if (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED) {
			if (!buffer.empty()) buffer.append("、");
			buffer.append("鼠标指针缩放");
		}
		if (buffer.empty()) {
			buffer.append("无");
		}
		return buffer;
	};
	std::string_view rotation_to_string(DXGI_MODE_ROTATION const rot) {
		switch (rot) {
		default:
		case DXGI_MODE_ROTATION_UNSPECIFIED: return "未知";
		case DXGI_MODE_ROTATION_IDENTITY: return "无";
		case DXGI_MODE_ROTATION_ROTATE90: return "90 度";
		case DXGI_MODE_ROTATION_ROTATE180: return "180 度";
		case DXGI_MODE_ROTATION_ROTATE270: return "270 度";
		}
	};
	std::string_view threading_feature_to_string(D3D11_FEATURE_DATA_THREADING const v) {
		if (v.DriverConcurrentCreates) {
			if (v.DriverCommandLists) {
				return "异步资源创建、多线程命令队列";
			}
			else {
				return "异步资源创建";
			}
		}
		else {
			if (v.DriverCommandLists) {
				return "多线程命令队列";
			}
			else {
				return "不支持";
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
namespace Core::Graphics::Direct3D11 {
	Device::Device(std::string_view const& preferred_gpu)
		: preferred_adapter_name(preferred_gpu) {
		// 创建图形组件

		i18n_log_info("[core].Device_D3D11.start_creating_graphic_components");

		testAdapterPolicy();
		if (!createDXGI())
			throw std::runtime_error("create basic DXGI components failed");
		if (!createD3D11())
			throw std::runtime_error("create basic D3D11 components failed");
		if (!createWIC())
			throw std::runtime_error("create basic WIC components failed");
		if (!createD2D1())
			throw std::runtime_error("create basic D2D1 components failed");
		if (!createDWrite())
			throw std::runtime_error("create basic DWrite components failed");

		i18n_log_info("[core].Device_D3D11.created_graphic_components");
	}
	Device::~Device() {
		// 清理对象
		destroyDWrite(); // 长生存期
		destroyD2D1();
		destroyWIC(); // 长生存期
		destroyD3D11();
		destroyDXGI();
		assert(m_eventobj.size() == 0);
		assert(m_eventobj_late.size() == 0);
	}

	bool Device::createDXGIFactory() {
		HRESULT hr = S_OK;

		// 创建 1.2 的组件，强制要求平台更新
		hr = gHR = dxgi_loader.CreateFactory(IID_PPV_ARGS(&dxgi_factory));
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("CreateDXGIFactory2 -> IDXGIFactory2");
			assert(false); return false;
		}

		return true;
	}
	void Device::destroyDXGIFactory() {
		dxgi_factory.Reset();
	}
	bool Device::selectAdapter() {
		HRESULT hr = S_OK;

		// 枚举所有图形设备

		i18n_log_info("[core].Device_D3D11.enum_all_adapters");

		struct AdapterCandidate {
			Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
			std::string adapter_name;
			DXGI_ADAPTER_DESC1 adapter_info;
			D3D_FEATURE_LEVEL feature_level;
			BOOL link_to_output;
		};
		std::vector<AdapterCandidate> adapter_candidate;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter_temp;
		for (UINT idx = 0; bHR = dxgi_factory->EnumAdapters1(idx, &dxgi_adapter_temp); idx += 1) {
			// 检查此设备是否支持 Direct3D 11 并获取特性级别
			bool supported_d3d11 = false;
			D3D_FEATURE_LEVEL level_info = D3D_FEATURE_LEVEL_10_0;
			hr = gHR = d3d11_loader.CreateDeviceFromAdapter(
				dxgi_adapter_temp.Get(),
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
			if (bHR = gHR = dxgi_adapter_temp->GetDesc1(&desc_)) {
				bool soft_dev_type = (desc_.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) || (desc_.Flags & DXGI_ADAPTER_FLAG_REMOTE);
				dev_name = utf8::to_string(desc_.Description);
				i18n_log_info_fmt("[core].Device_D3D11.DXGI_adapter_detail_fmt"
								  , idx
								  , dev_name
								  , d3d_feature_level_to_string(level_info)
								  , adapter_flags_to_string(desc_.Flags)
								  , soft_dev_type ? i18n("DXGI_adapter_type_software_warning") : ""
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
				i18n_core_system_call_report_error("IDXGIAdapter1::GetDesc1");
				i18n_log_error_fmt("[core].Device_D3D11.DXGI_adapter_detail_error_fmt", idx);
				supported_d3d11 = false; // 排除未知错误
			}

			// 枚举显示输出
			bool has_linked_output = false;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output_temp;
			for (UINT odx = 0; bHR = dxgi_adapter_temp->EnumOutputs(odx, &dxgi_output_temp); odx += 1) {
				Microsoft::WRL::ComPtr<IDXGIOutput6> dxgi_output_temp6;
				hr = gHR = dxgi_output_temp.As(&dxgi_output_temp6);
				if (FAILED(hr)) {
					i18n_core_system_call_report_error("IDXGIOutput::QueryInterface -> IDXGIOutput6");
					// 不是严重错误
				}

				DXGI_OUTPUT_DESC1 o_desc = {};
				bool read_o_desc = false;
				UINT comp_sp_flags = 0;

				if (dxgi_output_temp6) {
					if (!(bHR = gHR = dxgi_output_temp6->CheckHardwareCompositionSupport(&comp_sp_flags))) {
						comp_sp_flags = 0;
						i18n_core_system_call_report_error("IDXGIOutput6::CheckHardwareCompositionSupport");
					}
					if (bHR = gHR = dxgi_output_temp6->GetDesc1(&o_desc)) {
						read_o_desc = true;
					}
					else {
						i18n_core_system_call_report_error("IDXGIOutput6::GetDesc1");
					}
				}
				if (!read_o_desc) {
					DXGI_OUTPUT_DESC desc_0_ = {};
					if (bHR = gHR = dxgi_output_temp->GetDesc(&desc_0_)) {
						std::memcpy(o_desc.DeviceName, desc_0_.DeviceName, sizeof(o_desc.DeviceName));
						o_desc.DesktopCoordinates = desc_0_.DesktopCoordinates;
						o_desc.AttachedToDesktop = desc_0_.AttachedToDesktop;
						o_desc.Rotation = desc_0_.Rotation;
						o_desc.Monitor = desc_0_.Monitor;
						read_o_desc = true;
					}
					else {
						i18n_core_system_call_report_error("IDXGIOutput::GetDesc");
					}
				}

				if (read_o_desc) {
					i18n_log_info_fmt("[core].Device_D3D11.DXGI_output_detail_fmt"
									  , idx, odx
									  , o_desc.AttachedToDesktop ? i18n("DXGI_output_connected") : i18n("DXGI_output_not_connect")
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
					i18n_log_error_fmt("[core].Device_D3D11.DXGI_output_detail_error_fmt", idx, odx);
				}
			}
			dxgi_output_temp.Reset();

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
		dxgi_adapter_temp.Reset();

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
			M_D3D_SET_DEBUG_NAME(dxgi_adapter.Get(), "Device_D3D11::dxgi_adapter");
			i18n_log_info_fmt("[core].Device_D3D11.select_DXGI_adapter_fmt", dxgi_adapter_name);
			if (!link_to_output) {
				i18n_log_warn_fmt("[core].Device_D3D11.DXGI_adapter_no_output_warning_fmt", dxgi_adapter_name);
			}
		}
		else {
			i18n_log_critical("[core].Device_D3D11.no_available_DXGI_adapter");
			return false;
		}

		return true;
	}
	bool Device::createDXGI() {
		HRESULT hr = S_OK;

		i18n_log_info("[core].Device_D3D11.start_creating_basic_DXGI_components");

		// 创建工厂

		if (!createDXGIFactory()) return false;

		// 检测特性支持情况

		Microsoft::WRL::ComPtr<IDXGIFactory3> dxgi_factory3;
		hr = gHR = dxgi_factory.As(&dxgi_factory3);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("IDXGIFactory1::QueryInterface -> IDXGIFactory3");
			// 不是严重错误
		}

		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgi_factory4;
		hr = gHR = dxgi_factory.As(&dxgi_factory4);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("IDXGIFactory1::QueryInterface -> IDXGIFactory4");
			// 不是严重错误
		}

		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi_factory5;
		hr = gHR = dxgi_factory.As(&dxgi_factory5);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("IDXGIFactory1::QueryInterface -> IDXGIFactory5");
			// 不是严重错误
		}

		Microsoft::WRL::ComPtr<IDXGIFactory6> dxgi_factory6;
		hr = gHR = dxgi_factory.As(&dxgi_factory6);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("IDXGIFactory1::QueryInterface -> IDXGIFactory6");
			// 不是严重错误
		}

		Microsoft::WRL::ComPtr<IDXGIFactory7> dxgi_factory7;
		hr = gHR = dxgi_factory.As(&dxgi_factory7);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("IDXGIFactory1::QueryInterface -> IDXGIFactory7");
			// 不是严重错误
		}

		if (dxgi_factory5) {
			BOOL value = FALSE;
			hr = gHR = dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &value, sizeof(value));
			if (SUCCEEDED(hr)) {
				dxgi_support_tearing = value;
			}
			else {
				i18n_core_system_call_report_error("IDXGIFactory5::CheckFeatureSupport -> DXGI_FEATURE_PRESENT_ALLOW_TEARING");
				// 不是严重错误
			}
		}

		// 打印特性支持情况
		i18n_log_info_fmt("[core].Device_D3D11.DXGI_detail_fmt"
						  , dxgi_support_tearing ? i18n("support") : i18n("not_support.requires_Windows_10_and_hardware")
		);

		// 获取适配器

		if (!selectAdapter()) {
			if (!core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
				return false;
			}
		}

		// 检查适配器支持

		if (dxgi_adapter) {
			Microsoft::WRL::ComPtr<IDXGIAdapter2> dxgi_adapter2;
			hr = gHR = dxgi_adapter.As(&dxgi_adapter2);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIAdapter1::QueryInterface -> IDXGIAdapter2");
				// 不是严重错误
			}

			Microsoft::WRL::ComPtr<IDXGIAdapter3> dxgi_adapter3;
			hr = gHR = dxgi_adapter.As(&dxgi_adapter3);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIAdapter1::QueryInterface -> IDXGIAdapter3");
				// 不是严重错误
			}

			Microsoft::WRL::ComPtr<IDXGIAdapter2> dxgi_adapter4;
			hr = gHR = dxgi_adapter.As(&dxgi_adapter4);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIAdapter1::QueryInterface -> IDXGIAdapter4");
				// 不是严重错误
			}

			i18n_log_info("[core].Device_D3D11.created_basic_DXGI_components");
		}

		return true;
	}
	void Device::destroyDXGI() {
		destroyDXGIFactory();
		dxgi_adapter.Reset();

		dxgi_adapter_name.clear();
		dxgi_adapter_name_list.clear();

		dxgi_support_tearing = FALSE;
	}
	bool Device::createD3D11() {
		HRESULT hr = S_OK;

		// 创建

		i18n_log_info("[core].Device_D3D11.start_creating_basic_D3D11_components");

		if (dxgi_adapter) {
			hr = gHR = d3d11_loader.CreateDeviceFromAdapter(
				dxgi_adapter.Get(),
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				D3D_FEATURE_LEVEL_10_0,
				&d3d11_device,
				&d3d_feature_level,
				&d3d11_devctx);
		}
		else if (core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
			D3D_DRIVER_TYPE d3d_driver_type = D3D_DRIVER_TYPE_UNKNOWN;
			hr = gHR = d3d11_loader.CreateDeviceFromSoftAdapter(
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				D3D_FEATURE_LEVEL_10_0,
				&d3d11_device,
				&d3d_feature_level,
				&d3d11_devctx,
				&d3d_driver_type);
			if (SUCCEEDED(hr)) {
				switch (d3d_driver_type) {
				case D3D_DRIVER_TYPE_REFERENCE:
					spdlog::info("[core] 设备类型：参考光栅化设备");
					break;
				case D3D_DRIVER_TYPE_SOFTWARE:
					spdlog::info("[core] 设备类型：软件光栅化设备");
					break;
				case D3D_DRIVER_TYPE_WARP:
					spdlog::info("[core] 设备类型：Windows 高级光栅化平台（WARP）");
					break;
				}
			}
		}
		if (!d3d11_device) {
			i18n_core_system_call_report_error("D3D11CreateDevice");
			return false;
		}
		M_D3D_SET_DEBUG_NAME(d3d11_device.Get(), "Device_D3D11::d3d11_device");
		M_D3D_SET_DEBUG_NAME(d3d11_devctx.Get(), "Device_D3D11::d3d11_devctx");
		if (!dxgi_adapter) {
			hr = gHR = Platform::Direct3D11::GetDeviceAdater(d3d11_device.Get(), &dxgi_adapter);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("Platform::Direct3D11::GetDeviceAdater");
				return false;
			}
		}

		// 特性检查

		hr = gHR = d3d11_device.As(&d3d11_device1);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::QueryInterface -> ID3D11Device1");
			// 不是严重错误
		}
		hr = gHR = d3d11_devctx.As(&d3d11_devctx1);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11DeviceContext::QueryInterface -> ID3D11DeviceContext1");
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
			if (FAILED(hr1)) i18n_core_system_call_report_error(name1);

			D3D11_FEATURE_DATA_FORMAT_SUPPORT2 data2 = { .InFormat = format };
			HRESULT const hr2 = gHR = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &data2, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT2));
			if (FAILED(hr2)) i18n_core_system_call_report_error(name2);

			return D3DX11_FEATURE_DATA_FORMAT_SUPPORT{ .InFormat = format, .OutFormatSupport = data1.OutFormatSupport, .OutFormatSupport2 = data2.OutFormatSupport2 };
		};
		auto check_format_support_1 = [&](DXGI_FORMAT const format, std::string_view const& name) ->D3DX11_FEATURE_DATA_FORMAT_SUPPORT {
			std::string name1("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_FORMAT_SUPPORT ("); name1.append(name); name1.append(")");

			D3D11_FEATURE_DATA_FORMAT_SUPPORT data1 = { .InFormat = format };
			HRESULT const hr1 = gHR = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data1, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT));
			if (FAILED(hr1)) i18n_core_system_call_report_error(name1);

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
			i18n_core_system_call_report_error("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_THREADING");
			// 不是严重错误
		}

		D3D11_FEATURE_DATA_ARCHITECTURE_INFO d3d11_feature_arch = {};
		HRESULT hr_arch = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_ARCHITECTURE_INFO, &d3d11_feature_arch, sizeof(d3d11_feature_arch));
		if (FAILED(hr_arch)) {
			i18n_core_system_call_report_error("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_ARCHITECTURE_INFO");
			// 不是严重错误
		}

		D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11_feature_o2 = {};
		HRESULT hr_o2 = d3d11_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &d3d11_feature_o2, sizeof(d3d11_feature_o2));
		if (FAILED(hr_o2)) {
			i18n_core_system_call_report_error("ID3D11Device::CheckFeatureSupport -> D3D11_FEATURE_D3D11_OPTIONS2");
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

	#define _FORMAT_MAKE_SUPPORT i18n("support") : i18n("not_support")

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

		spdlog::info("[core] Direct3D 11 设备功能支持：\n"
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
			spdlog::warn("[core] 此设备没有完整的 B8G8R8A8 格式支持，程序可能无法正常运行");
		}

		testMultiPlaneOverlay();

		i18n_log_info("[core].Device_D3D11.created_basic_D3D11_components");

		tracy_context = tracy_d3d11_context_create(d3d11_device.Get(), d3d11_devctx.Get());

		return true;
	}
	void Device::destroyD3D11() {
		tracy_d3d11_context_destroy(tracy_context);
		tracy_context = nullptr;

		d3d_feature_level = D3D_FEATURE_LEVEL_10_0;

		d3d11_device.Reset();
		d3d11_device1.Reset();
		d3d11_devctx.Reset();
		d3d11_devctx1.Reset();
	}
	bool Device::createWIC() {
		HRESULT hr = S_OK;

		hr = gHR = CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic_factory2));
		if (SUCCEEDED(hr)) {
			hr = gHR = wic_factory2.As(&wic_factory);
			assert(SUCCEEDED(hr));
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IWICImagingFactory2::QueryInterface -> IWICImagingFactory");
				return false;
			}
		}
		else {
			i18n_core_system_call_report_error("CoCreateInstance -> IWICImagingFactory2");
			// 没有那么严重，来再一次
			hr = gHR = CoCreateInstance(CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic_factory));
			assert(SUCCEEDED(hr));
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("CoCreateInstance -> IWICImagingFactory");
				return false;
			}
		}

		return true;
	}
	void Device::destroyWIC() {
		wic_factory.Reset();
		wic_factory2.Reset();
	}
	bool Device::createD2D1() {
		HRESULT hr = S_OK;

		hr = gHR = d2d1_loader.CreateFactory(
			D2D1_FACTORY_TYPE_MULTI_THREADED,
			IID_PPV_ARGS(&d2d1_factory));
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("D2D1CreateFactory -> ID2D1Factory1");
			assert(false); return false;
		}

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
		hr = gHR = d3d11_device.As(&dxgi_device);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::QueryInterface -> IDXGIDevice");
			assert(false); return false;
		}

		hr = gHR = d2d1_factory->CreateDevice(dxgi_device.Get(), &d2d1_device);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID2D1Factory1::CreateDevice");
			assert(false); return false;
		}

		hr = gHR = d2d1_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d1_devctx);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID2D1Device::CreateDeviceContext");
			assert(false); return false;
		}

		return true;
	}
	void Device::destroyD2D1() {
		d2d1_factory.Reset();
		d2d1_device.Reset();
		d2d1_devctx.Reset();
	}
	bool Device::createDWrite() {
		HRESULT hr = S_OK;

		hr = gHR = dwrite_loader.CreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_PPV_ARGS(&dwrite_factory));
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("DWriteCreateFactory -> DWRITE_FACTORY_TYPE_SHARED");
			return false;
		}

		return true;
	}
	void Device::destroyDWrite() {
		dwrite_factory.Reset();
	}
	bool Device::doDestroyAndCreate() {
		dispatchEvent(EventType::DeviceDestroy);

		//destroyDWrite(); // 长生存期
		destroyD2D1();
		//destroyWIC(); // 长生存期
		destroyD3D11();
		destroyDXGI();

		testAdapterPolicy();

		if (!createDXGI()) return false;
		if (!createD3D11()) return false;
		//if (!createWIC()) return false; // 长生存期
		if (!createD2D1()) return false;
		//if (!createDWrite()) return false; // 长生存期

		dispatchEvent(EventType::DeviceCreate);

		return true;
	}
	bool Device::testAdapterPolicy() {
		if (preferred_adapter_name.empty()) return true; // default GPU

		auto testAdapterName = [&]() -> bool {
			HRESULT hr = S_OK;
			Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
			hr = gHR = dxgi_factory->EnumAdapters1(0, &dxgi_adapter);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIFactory1::EnumAdapters1 -> #0");
				assert(false); return false;
			}
			DXGI_ADAPTER_DESC1 desc = {};
			hr = gHR = dxgi_adapter->GetDesc1(&desc);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIAdapter1::GetDesc1");
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

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter_;
		for (UINT i = 0; bHR = dxgi_factory->EnumAdapters1(i, &adapter_); i += 1) {
			Microsoft::WRL::ComPtr<IDXGIOutput> output_;
			for (UINT j = 0; bHR = adapter_->EnumOutputs(j, &output_); j += 1) {
				DXGI_OUTPUT_DESC desc = {};
				hr = gHR = output_->GetDesc(&desc);
				if (FAILED(hr)) {
					i18n_core_system_call_report_error("IDXGIOutput3::CheckOverlaySupport -> DXGI_FORMAT_B8G8R8A8_UNORM");
					assert(false); return false;
				}

				BOOL overlay_ = FALSE;
				Microsoft::WRL::ComPtr<IDXGIOutput2> output2_;
				if (bHR = output_.As(&output2_)) {
					overlay_ = output2_->SupportsOverlays();
				}

				UINT overlay_flags = 0;
				Microsoft::WRL::ComPtr<IDXGIOutput3> output3_;
				if (bHR = output_.As(&output3_)) {
					hr = gHR = output3_->CheckOverlaySupport(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						d3d11_device.Get(),
						&overlay_flags);
					if (FAILED(hr)) {
						i18n_core_system_call_report_error("IDXGIOutput3::CheckOverlaySupport -> DXGI_FORMAT_B8G8R8A8_UNORM");
					}
				}

				UINT composition_flags = 0;
				Microsoft::WRL::ComPtr<IDXGIOutput6> output6_;
				if (bHR = output_.As(&output6_)) {
					hr = gHR = output6_->CheckHardwareCompositionSupport(&composition_flags);
					if (FAILED(hr)) {
						i18n_core_system_call_report_error("IDXGIOutput6::CheckHardwareCompositionSupport");
					}
				}

				i18n_log_info_fmt("[core].Device_D3D11.DXGI_output_detail_fmt2"
								  , i, j
								  , desc.AttachedToDesktop ? i18n("DXGI_output_connected") : i18n("DXGI_output_not_connect")
								  , desc.DesktopCoordinates.left
								  , desc.DesktopCoordinates.top
								  , desc.DesktopCoordinates.right - desc.DesktopCoordinates.left
								  , desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top
								  , rotation_to_string(desc.Rotation)
								  , overlay_ ? i18n("support") : i18n("not_support")
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

			dxgi_factory.Reset();
			dxgi_adapter.Reset();

			Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
			hr = gHR = d3d11_device.As(&dxgi_device);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("ID3D11Device::QueryInterface -> IDXGIDevice");
				return false;
			}

			Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter_tmp;
			hr = gHR = dxgi_device->GetAdapter(&dxgi_adapter_tmp);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIDevice::GetAdapter");
				return false;
			}

			hr = gHR = dxgi_adapter_tmp.As(&dxgi_adapter);
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("IDXGIAdapter::QueryInterface -> IDXGIAdapter1");
				return false;
			}

			//hr = gHR = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
			//if (FAILED(hr))
			//{
			//	i18n_core_system_call_report_error("IDXGIAdapter1::GetParent -> IDXGIFactory2");
			//	return false;
			//}

			// 创建 1.2 的组件，强制要求平台更新
			hr = gHR = dxgi_loader.CreateFactory(IID_PPV_ARGS(&dxgi_factory));
			if (FAILED(hr)) {
				i18n_core_system_call_report_error("CreateDXGIFactory2 -> IDXGIFactory2");
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
		Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
		if (bHR = dxgi_adapter.As(&adapter)) {
			DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
			if (bHR = gHR = adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info)) {
				data.local.budget = info.Budget;
				data.local.current_usage = info.CurrentUsage;
				data.local.available_for_reservation = info.AvailableForReservation;
				data.local.current_reservation = info.CurrentReservation;
			}
			if (bHR = gHR = adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info)) {
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
namespace Core::Graphics {
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
