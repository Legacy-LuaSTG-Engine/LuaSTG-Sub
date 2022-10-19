#include "Core/i18n.hpp"

struct i18n_entry_t
{
	std::string_view const zh_cn;
	std::string_view const en_us;
};

static i18n_lang_t g_i18n_lang = i18n_lang_t::zh_cn;
static std::unordered_map<std::string_view, i18n_entry_t> const g_i18n_map = {
	{"[core].system_call_failed_f",{
		.zh_cn = "[core] {} 调用失败",
		.en_us = "[core] {} failed",
	}},
	{"[core].system_dll_load_failed_f",{
		.zh_cn = "[core] 无法加载 {} 模块",
		.en_us = "[core] load module {} failed",
	}},
	{"[core].system_dll_load_func_failed_f",{
		.zh_cn = "[core] 无法从模块 {0} 加载接口 {1}",
		.en_us = "[core] load interface {1} from module {0} failed",
	}},

	{"Enable",{
		.zh_cn = "启用",
		.en_us = "Enable",
	}},
	{"Disable",{
		.zh_cn = "禁用",
		.en_us = "Disable",
	}},
	{"unknown",{
		.zh_cn = "未知",
		.en_us = "unknown",
	}},
	{"support",{
		.zh_cn = "支持",
		.en_us = "support",
	}},
	{"not_support",{
		.zh_cn = "不支持",
		.en_us = "not support",
	}},
	{"not_support.requires_Windows_8",{
		.zh_cn = "不支持（最低系统要求为 Windows 8）",
		.en_us = "not support (requires Windows 8 or above)",
	}},
	{"not_support.requires_Windows_8_point_1",{
		.zh_cn = "不支持（最低系统要求为 Windows 8.1）",
		.en_us = "not support (requires Windows 8.1 or above)",
	}},
	{"not_support.requires_Windows_10",{
		.zh_cn = "不支持（最低系统要求为 Windows 10）",
		.en_us = "not support (requires Windows 10 or above)",
	}},
	{"not_support.requires_Windows_10_and_hardware",{
		.zh_cn = "不支持（最低系统要求为 Windows 10，且硬件支持该功能）",
		.en_us = "not support (requires Windows 10 or above, and hardware support)",
	}},

	{"[core].Device_D3D11.start_creating_graphic_components",{
		.zh_cn = "[core] 开始创建图形组件",
		.en_us = "[core] Start creating graphic components",
	}},
	{"[core].Device_D3D11.created_graphic_components",{
		.zh_cn = "[core] 已创建图形组件",
		.en_us = "[core] Graphic components created",
	}},
	{"[core].Device_D3D11.start_creating_basic_DXGI_components",{
		.zh_cn = "[core] 开始创建基本 DXGI 组件",
		.en_us = "[core] Start creating basic DXGI components",
	}},
	{"[core].Device_D3D11.created_basic_DXGI_components",{
		.zh_cn = "[core] 已创建基本 DXGI 组件",
		.en_us = "[core] Basic DXGI components created",
	}},

	{"[core].Device_D3D11.enum_all_adapters",{
		.zh_cn = "[core] 枚举所有图形设备",
		.en_us = "[core] Enumerate all graphics devices",
	}},
	{"[core].Device_D3D11.DXGI_detail_fmt",{
		.zh_cn = "[core] DXGI 组件功能支持：\n"
			"    桌面窗口管理器优化级别：{}\n"
			"    交换链模式：\n"
			"        位块传输模式：支持\n"
			"        序列交换模式：{}\n"
			"        快速交换模式：{}\n"
			"    低延迟呈现：{}\n"
			"    立即刷新：{}"
			,
		.en_us = "[core] DXGI components feature support:\n"
			"    Desktop Window Manager optimization level: {}\n"
			"    SwapChain swap effect:\n"
			"        Discard: support\n"
			"        Flip Sequential: {}\n"
			"        Flip Discard: {}\n"
			"    Frame Latency Waitable Object: {}\n"
			"    Present Allow Tearing: {}"
			,
	}},

	{"DXGI_adapter_type_hardware",{
		.zh_cn = "硬件设备",
		.en_us = "Hardware",
	}},
	{"DXGI_adapter_type_software",{
		.zh_cn = "软件设备",
		.en_us = "Software",
	}},
	{"DXGI_adapter_type_hardware_remote",{
		.zh_cn = "远程硬件设备",
		.en_us = "Remote Hardware",
	}},
	{"DXGI_adapter_type_software_remote",{
		.zh_cn = "远程软件设备",
		.en_us = "Remote Software",
	}},
	{"DXGI_adapter_type_software_warning",{
		.zh_cn = "（警告：软件或远程设备性能不足流畅运行程序）",
		.en_us = " (WARNING: The software or remote device does not have enough performance to run the program smoothly)",
	}},

	{"[core].Device_D3D11.DXGI_adapter_detail_fmt",{
		.zh_cn = "[core] 图形设备[{}]：\n"
			"    设备名称：{}\n"
			"    Direct3D 功能级别：{}\n"
			"    设备类型：{}{}\n"
			"    专用显存：{}\n"
			"    专用内存：{}\n"
			"    共享内存：{}\n"
			"    供应商 ID：0x{:08X}\n"
			"    设备 ID：0x{:08X}\n"
			"    子系统 ID：0x{:08X}\n"
			"    修订号：0x{:08X}\n"
			"    设备 LUID：{:08X}-{:08X}"
			,
		.en_us = "[core] Graphics Device[{}]:\n"
			"    Description: {}\n"
			"    Direct3D Feature Level: {}\n"
			"    Type: {}{}\n"
			"    Dedicated Video Memory: {}\n"
			"    Dedicated System Memory: {}\n"
			"    Shared System Memory: {}\n"
			"    Vendor ID: 0x{:08X}\n"
			"    Device ID: 0x{:08X}\n"
			"    SubSystem ID: 0x{:08X}\n"
			"    Revision: 0x{:08X}\n"
			"    Adapter LUID：{:08X}-{:08X}"
			,
	}},
	{"[core].Device_D3D11.DXGI_adapter_detail_error_fmt",{
		.zh_cn = "[core] 图形设备[{}]：<无法读取信息>",
		.en_us = "[core] Graphics Device[{}]: <Unable to read information>",
	}},

	{"[core].Device_D3D11.select_DXGI_adapter_fmt",{
		.zh_cn = "[core] 已选择图形设备：{}",
		.en_us = "[core] Select Graphics Device: {}",
	}},
	{"[core].Device_D3D11.no_available_DXGI_adapter",{
		.zh_cn = "[core] 没有可用的图形设备",
		.en_us = "[core] No Graphics Device available",
	}},
	{"[core].Device_D3D11.DXGI_adapter_no_output_warning_fmt",{
		.zh_cn = "[core] 图形设备 {} 似乎没有连接到任何显示输出，这可能导致：\n"
			"    独占全屏时会通过 PCI-E 复制渲染缓冲区\n"
			"    独占全屏时桌面窗口管理器接管画面合成\n"
			"    性能下降、呈现延迟变高"
			,
		.en_us = "[core] Graphics Device {} doesn't appear to connect to any display output, which could result in:\n"
			"    Buffer is copied over PCI-E when in exclusive fullscreen\n"
			"    Desktop Window Manager takes over desktop composition when in exclusive fullscreen\n"
			"    Degraded performance and increased frame latency"
			,
	}},

	{"DXGI_output_connected",{
		.zh_cn = "已连接",
		.en_us = "True",
	}},
	{"DXGI_output_not_connect",{
		.zh_cn = "未连接",
		.en_us = "False",
	}},

	{"[core].Device_D3D11.DXGI_output_detail_fmt",{
		.zh_cn = "[core] 图形设备[{}] - 显示输出设备[{}]：\n"
			"    连接状态：{}\n"
			"    显示区域：({}, {}) ({} x {})\n"
			"    旋转：{}\n"
			"    硬件表面合成：{}"
			,
		.en_us = "[core] Graphics Device[{}] - Display Output[{}]:\n"
			"    Attached To Desktop: {}\n"
			"    Desktop Coordinates: ({}, {}) ({} x {})\n"
			"    Rotation: {}\n"
			"    Hardware Composition: {}"
			,
	}},
	{"[core].Device_D3D11.DXGI_output_detail_fmt2",{
		.zh_cn = "[core] 图形设备[{}] - 显示输出设备[{}]：\n"
			"    连接状态：{}\n"
			"    显示区域：({}, {}) ({} x {})\n"
			"    旋转：{}\n"
			"    多平面叠加：{}\n"
			"    多平面叠加功能：{}\n"
			"    硬件表面合成：{}"
			,
		.en_us = "[core] Graphics Device[{}] - Display Output[{}]:\n"
			"    Attached To Desktop: {}\n"
			"    Desktop Coordinates: ({}, {}) ({} x {})\n"
			"    Rotation: {}\n"
			"    Multi-Plane Overlay: {}\n"
			"    Multi-Plane Overlay Feature: {}\n"
			"    Hardware Composition: {}"
			,
	}},
	{"[core].Device_D3D11.DXGI_output_detail_error_fmt",{
		.zh_cn = "[core] 图形设备[{}] - 显示输出设备[{}]：<无法读取信息>",
		.en_us = "[core] Graphics Device[{}] - Display Output[{}]: <Unable to read information>",
	}},

	{"[core].Device_D3D11.start_creating_basic_D3D11_components",{
		.zh_cn = "[core] 开始创建基本 Direct3D 11 组件",
		.en_us = "[core] Start creating basic Direct3D 11 components",
	}},
	{"[core].Device_D3D11.created_basic_D3D11_components",{
		.zh_cn = "[core] 已创建基本 Direct3D 11 组件",
		.en_us = "[core] Basic Direct3D 11 components created",
	}},

#pragma warning(disable:4002)
#define ADD(KEY, V1, V2) {KEY,{V1,V2}}

	// Swapchain DXGI
	ADD("DXGI.SwapChain.SwapEffect.Discard",
		.zh_cn = "位块传输模式",
		.en_us = "Discard",
	),
	ADD("DXGI.SwapChain.SwapEffect.FlipSequential",
		.zh_cn = "序列交换模式",
		.en_us = "Flip Sequential",
	),
	ADD("DXGI.SwapChain.SwapEffect.FlipDiscard",
		.zh_cn = "快速交换模式",
		.en_us = "Flip Discard",
	),
	ADD("DXGI.DisplayMode.RefreshRate.Desktop",
		.zh_cn = "桌面刷新率",
		.en_us = "Desktop",
	),
	// Swapchain
	ADD("[core].SwapChain_D3D11.start_creating_swapchain",
		.zh_cn = "[core] 开始创建 SwapChain",
		.en_us = "[core] Start creating SwapChain",
	),
	ADD("[core].SwapChain_D3D11.created_swapchain",
		.zh_cn = "[core] 已创建 SwapChain",
		.en_us = "[core] SwapChain created",
	),
	ADD("[core].SwapChain_D3D11.created_swapchain_info_fmt",
		.zh_cn = "[core] 已创建 SwapChain：\n"
			"    显示模式：{}x{}@{}\n"
			"    独占全屏：{}\n"
			"    交换链模式：{}\n"
			"    立即刷新：{}\n"
			"    低延迟呈现：{}",
		.en_us = "[core] SwapChain created:\n"
			"    Display Mode: {}x{}@{}\n"
			"    Exclusive Fullscreen：{}\n"
			"    SwapChain swap effect: {}\n"
			"    Present Allow Tearing: {}\n"
			"    Frame Latency Waitable Object: {}",
	),
	ADD("[core].SwapChain_D3D11.create_swapchain_failed_null_window",
		.zh_cn = "[core] 无法创建 SwapChain，窗口为空",
		.en_us = "[core] Failed to create SwapChain, Window is null",
	),
	ADD("[core].SwapChain_D3D11.create_swapchain_failed_null_DXGI",
		.zh_cn = "[core] 无法创建 SwapChain，DXGI 组件为空",
		.en_us = "[core] Failed to create SwapChain, DXGI components are null",
	),
	ADD("[core].SwapChain_D3D11.create_swapchain_failed_null_device",
		.zh_cn = "[core] 无法创建 SwapChain，图形设备为空",
		.en_us = "[core] Failed to create SwapChain, Device is null",
	),
	ADD("[core].SwapChain_D3D11.create_swapchain_failed_invalid_size_fmt",
		.zh_cn = "[core] 无法创建大小为 ({}x{}) 的 SwapChain",
		.en_us = "[core] Cannot create SwapChain with size ({}x{})",
	),
	ADD("[core].SwapChain_D3D11.resize_swapchain_failed_invalid_size_fmt",
		.zh_cn = "[core] 无法更改 SwapChain 的大小为 ({}x{})",
		.en_us = "[core] Cannot resize SwapChain with size ({}x{})",
	),
	ADD("[core].SwapChain_D3D11.resize_swapchain_failed_null_SwapChain",
		.zh_cn = "[core] 无法更改 SwapChain 的大小，交换链为空",
		.en_us = "[core] Failed to resize SwapChain, SwapChain is null",
	),
	// Swapchain Exclusive Fullscreen
	ADD("[core].SwapChain_D3D11.enter_exclusive_fullscreen",
		.zh_cn = "[core] 尝试进入独占全屏",
		.en_us = "[core] Try to enter exclusive fullscreen",
	),
	ADD("[core].SwapChain_D3D11.leave_exclusive_fullscreen",
		.zh_cn = "[core] 尝试退出独占全屏",
		.en_us = "[core] Try to leave exclusive fullscreen",
	),
	ADD("[core].SwapChain_D3D11.exclusive_fullscreen_unavailable",
		.zh_cn = "[core] 独占全屏不可用",
		.en_us = "[core] Exclusive fullscreen unavailable",
	),
	// Swapchain DisplayMode
	ADD("[core].SwapChain_D3D11.start_enumerating_DisplayMode",
		.zh_cn = "[core] 开始枚举显示模式",
		.en_us = "[core] Start enumerating DisplayMode",
	),
	ADD("[core].SwapChain_D3D11.found_N_DisplayMode_fmt",
		.zh_cn = "[core] 共找到 {} 个支持的显示模式：",
		.en_us = "[core] Found {} supported DisplayMode:",
	),
	ADD("[core].SwapChain_D3D11.enumerating_DisplayMode_failed",
		.zh_cn = "[core] 找不到可用的显示模式",
		.en_us = "[core] No DisplayMode available",
	),
	ADD("[core].SwapChain_D3D11.match_DisplayMode_failed_invalid_size_fmt",
		.zh_cn = "[core] 无法匹配大小为 ({}x{}) 的显示模式",
		.en_us = "[core] Cannot match DisplayMode with size ({}x{})",
	),
	ADD("[core].SwapChain_D3D11.match_DisplayMode_failed_null_SwapChain",
		.zh_cn = "[core] 无法匹配显示模式，交换链为空",
		.en_us = "[core] Failed to match DisplayMode, SwapChain is null"
	),
	ADD("[core].SwapChain_D3D11.match_DisplayMode_failed_null_Device",
		.zh_cn = "[core] 无法匹配显示模式，图形设备为空",
		.en_us = "[core] Failed to match DisplayMode, Device is null"
	),
	// Swapchain RenderAttachment
	ADD("[core].SwapChain_D3D11.start_creating_RenderAttachment",
		.zh_cn = "[core] 开始创建 RenderAttachment",
		.en_us = "[core] Start creating RenderAttachment"
	),
	ADD("[core].SwapChain_D3D11.created_RenderAttachment",
		.zh_cn = "[core] 已创建 RenderAttachment",
		.en_us = "[core] RenderAttachment created"
	),
	ADD("[core].SwapChain_D3D11.create_RenderAttachment_failed_null_SwapChain",
		.zh_cn = "[core] 无法创建 RenderAttachment，交换链为空",
		.en_us = "[core] Failed to create RenderAttachment, SwapChain is null"
	),
	ADD("[core].SwapChain_D3D11.create_RenderAttachment_failed_null_Device",
		.zh_cn = "[core] 无法创建 RenderAttachment，图形设备为空",
		.en_us = "[core] Failed to create RenderAttachment, Device is null"
	),

#pragma warning(default:4002)
};

void i18n_set_lang(i18n_lang_t lang)
{
	g_i18n_lang = lang;
}
std::string_view i18n(std::string_view const& key)
{
	assert(!key.empty());
	auto it = g_i18n_map.find(key);
	assert(it != g_i18n_map.end());
	if (it != g_i18n_map.end())
	{
		assert(!it->second.zh_cn.empty());
		assert(!it->second.en_us.empty());
		switch (g_i18n_lang)
		{
		default:
		case i18n_lang_t::zh_cn:
			return it->second.zh_cn;
		case i18n_lang_t::en_us:
			return it->second.en_us;
		}
	}
	return key;
}
