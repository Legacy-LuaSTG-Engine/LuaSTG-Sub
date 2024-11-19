#include "Core/Graphics/Window_Win32.hpp"
#include "Core/ApplicationModel_Win32.hpp"
#include "Core/i18n.hpp"
#include "core/Configuration.hpp"
#include "win32/win32.hpp"
#include "win32/abi.hpp"
#include "Platform/WindowsVersion.hpp"
#include "Platform/WindowTheme.hpp"
#include "utf8.hpp"
#include <WinUser.h>
#include <WinNls.h>

static constexpr int const LUASTG_WM_UPDAE_TITLE = WM_APP + __LINE__;
static constexpr int const LUASTG_WM_RECREATE = WM_APP + __LINE__;
static constexpr int const LUASTG_WM_SETICON = WM_APP + __LINE__;
static constexpr int const LUASTG_WM_SET_WINDOW_MODE = WM_APP + __LINE__;
static constexpr int const LUASTG_WM_SET_FULLSCREEN_MODE = WM_APP + __LINE__;

namespace Core::Graphics
{
	static MONITORINFO getMonitorInfo(HMONITOR monitor) {
		assert(monitor);
		BOOL br{};
		MONITORINFO info{ .cbSize{sizeof(MONITORINFO)} };
		br = GetMonitorInfoW(monitor, &info);
		assert(br);
		return info;
	}

	void* Display_Win32::getNativeHandle() {
		return win32_monitor;
	}
	Vector2U Display_Win32::getSize() {
		auto const info = getMonitorInfo(win32_monitor);
		auto const& rc = info.rcMonitor;
		return Vector2U(static_cast<uint32_t>(rc.right - rc.left), static_cast<uint32_t>(rc.bottom - rc.top));
	}
	Vector2I Display_Win32::getPosition() {
		auto const info = getMonitorInfo(win32_monitor);
		auto const& rc = info.rcMonitor;
		return Vector2I(rc.left, rc.top);
	}
	RectI Display_Win32::getRect() {
		auto const info = getMonitorInfo(win32_monitor);
		auto const& rc = info.rcMonitor;
		return RectI(rc.left, rc.top, rc.right, rc.bottom);
	}
	Vector2U Display_Win32::getWorkAreaSize() {
		auto const info = getMonitorInfo(win32_monitor);
		auto const& rc = info.rcWork;
		return Vector2U(static_cast<uint32_t>(rc.right - rc.left), static_cast<uint32_t>(rc.bottom - rc.top));
	}
	Vector2I Display_Win32::getWorkAreaPosition() {
		auto const info = getMonitorInfo(win32_monitor);
		auto const& rc = info.rcWork;
		return Vector2I(rc.left, rc.top);
	}
	RectI Display_Win32::getWorkAreaRect() {
		auto const info = getMonitorInfo(win32_monitor);
		auto const& rc = info.rcWork;
		return RectI(rc.left, rc.top, rc.right, rc.bottom);
	}
	bool Display_Win32::isPrimary() {
		auto const info = getMonitorInfo(win32_monitor);
		return !!(info.dwFlags & MONITORINFOF_PRIMARY);
	}
	float Display_Win32::getDisplayScale() {
		return win32::getDpiScalingForMonitor(win32_monitor);
	}

	Display_Win32::Display_Win32(HMONITOR monitor) : win32_monitor(monitor) {
	}
	Display_Win32::~Display_Win32() = default;

	bool IDisplay::getAll(size_t* count, IDisplay** output) {
		assert(count);
		std::vector<HMONITOR> list;
		struct Context {
			std::vector<HMONITOR> list;
			static BOOL CALLBACK callback(HMONITOR monitor, HDC, LPRECT, LPARAM data) {
				auto context = reinterpret_cast<Context*>(data);
				context->list.emplace_back(monitor);
				return TRUE;
			};
		};
		Context context{};
		if (!EnumDisplayMonitors(NULL, NULL, &Context::callback, reinterpret_cast<LPARAM>(&context))) {
			return false;
		}
		*count = context.list.size();
		if (output) {
			for (size_t i = 0; i < context.list.size(); i += 1) {
				auto display = new Display_Win32(context.list.at(i));
				output[i] = display;
			}
		}
		return true;
	}
	bool IDisplay::getPrimary(IDisplay** output) {
		assert(output);
		struct Context {
			HMONITOR primary{};
			static BOOL CALLBACK callback(HMONITOR monitor, HDC, LPRECT, LPARAM data) {
				auto context = reinterpret_cast<Context*>(data);
				auto const info = getMonitorInfo(monitor);
				if (info.dwFlags & MONITORINFOF_PRIMARY) {
					context->primary = monitor;
					return FALSE;
				}
				return TRUE;
			};
		};
		Context context{};
		if (!EnumDisplayMonitors(NULL, NULL, &Context::callback, reinterpret_cast<LPARAM>(&context))) {
			return false;
		}
		auto display = new Display_Win32(context.primary);
		*output = display;
		return true;
	}
	bool IDisplay::getNearestFromWindow(IWindow* window, IDisplay** output) {
		assert(window);
		assert(output);
		HMONITOR monitor = MonitorFromWindow(static_cast<HWND>(window->getNativeHandle()), MONITOR_DEFAULTTOPRIMARY);
		if (!monitor) {
			return false;
		}
		auto display = new Display_Win32(monitor);
		*output = display;
		return true;
	}
}

namespace Core::Graphics
{
	void       Window_Win32::textInput_updateBuffer() {
		m_text_input_buffer_u8.clear();
		for (auto const code : m_text_input_buffer) {
			if (code <= 0x7f) {
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(code));
			}
			else if (code <= 0x7ff) {
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0xc0 | ((code >> 6) & 0x1f)));
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0x80 | (code & 0x3f)));
			}
			else if (code <= 0xffff) {
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0xe0 | ((code >> 12) & 0xf)));
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0x80 | ((code >> 6) & 0x3f)));
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0x80 | (code & 0x3f)));
			}
			else if (code <= 0x10ffff) {
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0xf0 | ((code >> 18) & 0x7)));
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0x80 | ((code >> 12) & 0x3f)));
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0x80 | ((code >> 6) & 0x3f)));
				m_text_input_buffer_u8.push_back(static_cast<char8_t>(0x80 | (code & 0x3f)));
			}
			else {
				m_text_input_buffer_u8.push_back(u8'?');
			}
		}
	}
	void       Window_Win32::textInput_addChar32(char32_t const code) {
		m_text_input_buffer.insert(m_text_input_buffer.begin() + m_text_input_cursor, code);
		m_text_input_cursor += 1;
		textInput_updateBuffer();
	}

	bool       Window_Win32::textInput_isEnabled() {
		return m_text_input_enabled;
	}
	void       Window_Win32::textInput_setEnabled(bool const enabled) {
		m_text_input_enabled = enabled;
	}
	StringView Window_Win32::textInput_getBuffer() {
		return { reinterpret_cast<char const*>(m_text_input_buffer_u8.data()), m_text_input_buffer_u8.size()};
	}
	void       Window_Win32::textInput_clearBuffer() {
		m_text_input_buffer.clear();
		m_text_input_buffer_u8.clear();
		m_text_input_cursor = 0;
	}
	uint32_t   Window_Win32::textInput_getCursorPosition() {
		return m_text_input_cursor;
	}
	void       Window_Win32::textInput_setCursorPosition(uint32_t code_point_index) {
		m_text_input_cursor = std::min(code_point_index, static_cast<uint32_t>(m_text_input_buffer.size()));
	}
	void       Window_Win32::textInput_addCursorPosition(int32_t offset_by_code_point) {
		if (offset_by_code_point == 0) {
			return;
		}
		if (offset_by_code_point > 0) {
			if ((static_cast<uint64_t>(m_text_input_cursor) + offset_by_code_point) <= m_text_input_buffer.size()) {
				m_text_input_cursor += offset_by_code_point;
			}
			else {
				m_text_input_cursor = static_cast<uint32_t>(m_text_input_buffer.size());
			}
		}
		else {
			if ((static_cast<int64_t>(m_text_input_cursor) + offset_by_code_point) >= 0) {
				m_text_input_cursor += offset_by_code_point;
			}
			else {
				m_text_input_cursor = 0;
			}
		}
	}
	void       Window_Win32::textInput_removeBufferRange(uint32_t code_point_index, uint32_t code_point_count) {
		// TODO
	}
	void       Window_Win32::textInput_removeBufferRangeFormCurrentCursorPosition(uint32_t code_point_count) {
		for (uint32_t i = 0; i < code_point_count; i += 1) {
			if (m_text_input_cursor > 0) {
				m_text_input_buffer.erase(m_text_input_buffer.begin() + (m_text_input_cursor - 1));
				m_text_input_cursor -= 1;
			}
			else {
				break;
			}
		}
		textInput_updateBuffer();
	}
	void       Window_Win32::textInput_insertBufferRange(uint32_t code_point_index, StringView str) {
		// TODO
	}

	void Window_Win32::setInputMethodPosition(Vector2I position) {
		if (!win32_window) {
			return;
		}
		if (HIMC himc = ImmGetContext(win32_window); win32_window) {
			COMPOSITIONFORM composition_form = {};
			composition_form.ptCurrentPos.x = position.x;
			composition_form.ptCurrentPos.y = position.y;
			composition_form.dwStyle = CFS_FORCE_POSITION;
			ImmSetCompositionWindow(himc, &composition_form);
			CANDIDATEFORM candidate_form = {};
			candidate_form.dwStyle = CFS_CANDIDATEPOS;
			candidate_form.ptCurrentPos.x = position.x;
			candidate_form.ptCurrentPos.y = position.y;
			ImmSetCandidateWindow(himc, &candidate_form);
			ImmReleaseContext(win32_window, himc);
		}
	}
}

namespace Core::Graphics
{
	static DWORD mapWindowStyle(WindowFrameStyle style, bool fullscreen) {
		if (fullscreen) {
			return WS_POPUP;
		}
		switch (style)
		{
		default:
			assert(false); return WS_POPUP;
		case WindowFrameStyle::None:
			return WS_POPUP;
		case WindowFrameStyle::Fixed:
			return WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
		case WindowFrameStyle::Normal:
			return WS_OVERLAPPEDWINDOW;
		}
	}

#define APPMODEL ((ApplicationModel_Win32*)m_framework)

	LRESULT CALLBACK Window_Win32::win32_window_callback(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
	{
		if (Window_Win32* self = (Window_Win32*)GetWindowLongPtrW(window, GWLP_USERDATA))
		{
			return self->onMessage(window, message, arg1, arg2);
		}
		switch (message)
		{
		case WM_NCCREATE:
			win32::enableNonClientDpiScaling(window);
			break;
		case WM_CREATE:
			SetLastError(0);
			SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCTW*)arg2)->lpCreateParams);
			if (DWORD const err = GetLastError())
			{
				spdlog::error("[luastg] (LastError = {}) SetWindowLongPtrW -> #GWLP_USERDATA failed", err);
				return -1;
			}
			return 0;
		}
		return DefWindowProcW(window, message, arg1, arg2);
	}

	LRESULT Window_Win32::onMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
	{
		if (enable_track_window_focus && win32_window_want_track_focus)
		{
			HWND focus_window = GetForegroundWindow();
			if (focus_window && focus_window != window)
			{
				win32_window_want_track_focus = FALSE;
				std::array<WCHAR, 256> buffer1{};
				std::array<WCHAR, 256> buffer2{};
				LRESULT const length1 = SendMessageW(focus_window, WM_GETTEXT, 256, (LPARAM)buffer1.data());
				int const length2 = GetClassNameW(focus_window, buffer2.data(), 256);
				spdlog::info("[core] 窗口焦点已改变，新的焦点窗口为：[{}] ({}) {}", (void*)focus_window,
					utf8::to_string(std::wstring_view(buffer2.data(), (size_t)length2)),
					utf8::to_string(std::wstring_view(buffer1.data(), (size_t)length1))
				);
			}
		}

		// 文本输入
		if (m_text_input_enabled) {
			switch (message) {
			case WM_CHAR:
				if (IS_HIGH_SURROGATE(arg1)) {
					m_text_input_last_high_surrogate = static_cast<char16_t>(arg1);
				}
				else if (IS_LOW_SURROGATE(arg1)) {
					if (IS_SURROGATE_PAIR(m_text_input_last_high_surrogate, arg1)) {
						auto const high = m_text_input_last_high_surrogate;
						auto const low = static_cast<char16_t>(arg1);
						auto const code = 0x10000 + (((static_cast<char32_t>(high) & 0x3ff) << 10) | (static_cast<char32_t>(low) & 0x3ff));
						textInput_addChar32(code);
						textInput_updateBuffer();
					}
					m_text_input_last_high_surrogate = {};
				}
				else if (arg1 <= 0xFFFF) {
					auto const code = static_cast<char32_t>(arg1);
					if (code == U'\b' /* backspace */) {
						textInput_removeBufferRangeFormCurrentCursorPosition(1);
					}
					else {
						textInput_addChar32(code);
					}
					textInput_updateBuffer();
				}
				break;
			}
		}
		// 窗口挪动器
		if (auto const result = m_sizemove.handleSizeMove(window, message, arg1, arg2); result.bReturn) {
			return result.lResult;
		}
		// 标题栏控制器
		if (auto const result = m_title_bar_controller.handleWindowMessage(window, message, arg1, arg2); result.returnResult) {
			return result.result;
		}
		// 特殊
		for (auto& v : m_eventobj)
		{
			if (v)
			{
				auto r = v->onNativeWindowMessage(window, message, arg1, arg2);
				if (r.should_return)
				{
					return r.result;
				}
			}
		}
		// 其他消息
		switch (message)
		{
		case WM_ACTIVATEAPP:
			if (arg1 /* == TRUE */)
			{
				win32_window_want_track_focus = FALSE;
				Platform::WindowTheme::UpdateColorMode(window, TRUE);
				dispatchEvent(EventType::WindowActive);
			}
			else
			{
				win32_window_want_track_focus = TRUE; // 要开始抓内鬼了
				Platform::WindowTheme::UpdateColorMode(window, FALSE);
				dispatchEvent(EventType::WindowInactive);
			}
			m_alt_down = FALSE; // 无论如何，清除该按键状态
			break;
		case WM_SIZE:
			if (!m_ignore_size_message)
			{
				EventData d = {};
				d.window_size = Vector2U(LOWORD(arg2), HIWORD(arg2));
				dispatchEvent(EventType::WindowSize, d);
			}
			break;
		case WM_ENTERSIZEMOVE:
			win32_window_is_sizemove = TRUE;
			InvalidateRect(window, NULL, FALSE); // 标记窗口区域为需要重新绘制，以便产生 WM_PAINT 消息
			return 0;
		case WM_EXITSIZEMOVE:
			win32_window_is_sizemove = FALSE;
			return 0;
		case WM_ENTERMENULOOP:
			win32_window_is_menu_loop = TRUE;
			InvalidateRect(window, NULL, FALSE); // 标记窗口区域为需要重新绘制，以便产生 WM_PAINT 消息
			return 0;
		case WM_EXITMENULOOP:
			win32_window_is_menu_loop = FALSE;
			return 0;
		case WM_PAINT:
			if (win32_window_is_sizemove || win32_window_is_menu_loop)
			{
				APPMODEL->runFrame();
			}
			else
			{
				ValidateRect(window, NULL); // 正常情况下，WM_PAINT 忽略掉
			}
			return 0;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (arg1 == VK_MENU)
			{
				m_alt_down = TRUE;
				return 0;
			}
			if (m_alt_down && arg1 == VK_RETURN)
			{
				_toggleFullScreenMode();
				return 0;
			}
			break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			if (arg1 == VK_MENU)
			{
				m_alt_down = FALSE;
				return 0;
			}
			break;
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* info = (MINMAXINFO*)arg2;
			RECT rect_min = { 0, 0, 320, 240 };
			UINT const dpi = win32::getDpiForWindow(window);
			if (m_title_bar_controller.adjustWindowRectExForDpi(&rect_min, win32_window_style, FALSE, win32_window_style_ex, dpi))
			{
				info->ptMinTrackSize.x = rect_min.right - rect_min.left;
				info->ptMinTrackSize.y = rect_min.bottom - rect_min.top;
			}
		}
		return 0;
		case WM_DPICHANGED:
			if (getFrameStyle() != WindowFrameStyle::None)
			{
				RECT rc{};
				GetWindowRect(window, &rc);
				setSize(getSize()); // 刷新一次尺寸（因为非客户区可能会变化）
				SetWindowPos(window, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				dispatchEvent(EventType::WindowDpiChanged);
				return 0;
			}
			dispatchEvent(EventType::WindowDpiChanged); // 仍然需要通知
			break;
		case WM_SETTINGCHANGE:
		case WM_THEMECHANGED:
			Platform::WindowTheme::UpdateColorMode(window, TRUE);
			break;
		case WM_DEVICECHANGE:
			if (arg1 == 0x0007 /* DBT_DEVNODES_CHANGED */)
			{
				dispatchEvent(EventType::DeviceChange);
			}
			break;
		case WM_SETCURSOR:
			if (LOWORD(arg2) == HTCLIENT)
			{
				SetCursor(win32_window_cursor);
				return TRUE;
			}
			break;
		case WM_MENUCHAR:
			// 快捷键能不能死全家
			return MAKELRESULT(0, MNC_CLOSE);
		case WM_SYSCOMMAND:
			// 鼠标左键点击标题栏图标或者 Alt+Space 不会出现菜单
			switch (arg1 & 0xFFF0)
			{
			case SC_KEYMENU:
			case SC_MOUSEMENU:
				return 0;
			}
			break;
		case WM_CLOSE:
			dispatchEvent(EventType::WindowClose);
			PostQuitMessage(EXIT_SUCCESS);
			return 0;
		case LUASTG_WM_UPDAE_TITLE:
			SetWindowTextW(window, win32_window_text_w.data());
			return 0;
		case LUASTG_WM_RECREATE:
		{
			BOOL result = FALSE;
			WINDOWPLACEMENT last_window_placement = {};
			last_window_placement.length = sizeof(last_window_placement);

			assert(win32_window);
			result = GetWindowPlacement(win32_window, &last_window_placement);
			assert(result); (void)result;

			destroyWindow();
			if (!createWindow()) return false;

			assert(win32_window);
			result = SetWindowPlacement(win32_window, &last_window_placement);
			assert(result); (void)result;
		}
		return 0;
		case LUASTG_WM_SETICON:
		{
			HICON hIcon = LoadIcon(win32_window_class.hInstance, MAKEINTRESOURCE(win32_window_icon_id));
			SendMessageW(win32_window, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
			SendMessageW(win32_window, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
			DestroyIcon(hIcon);
		}
		return 0;
		case LUASTG_WM_SET_WINDOW_MODE:
			_setWindowMode(reinterpret_cast<SetWindowedModeParameters*>(arg1), arg2);
			return 0;
		case LUASTG_WM_SET_FULLSCREEN_MODE:
			_setFullScreenMode(reinterpret_cast<IDisplay*>(arg2));
			return 0;
		}
		return DefWindowProcW(window, message, arg1, arg2);
	}
	bool Window_Win32::createWindowClass()
	{
		HINSTANCE hInstance = GetModuleHandleW(NULL);
		assert(hInstance); // 如果 hInstance 为 NULL 那肯定是见鬼了

		std::memset(win32_window_class_name, 0, sizeof(win32_window_class_name));
		std::swprintf(win32_window_class_name, std::size(win32_window_class_name), L"LuaSTG::Core::Window[%p]", this);

		auto& cls = win32_window_class;
		cls.style = CS_HREDRAW | CS_VREDRAW;
		cls.lpfnWndProc = &win32_window_callback;
		cls.hInstance = hInstance;
		cls.hCursor = LoadCursor(NULL, IDC_ARROW);
		cls.lpszClassName = win32_window_class_name;

		win32_window_class_atom = RegisterClassExW(&cls);
		if (win32_window_class_atom == 0)
		{
			spdlog::error("[luastg] (LastError = {}) RegisterClassExW failed", GetLastError());
			return false;
		}

		return true;
	}
	void Window_Win32::destroyWindowClass()
	{
		if (win32_window_class_atom != 0)
		{
			UnregisterClassW(win32_window_class.lpszClassName, win32_window_class.hInstance);
		}
		win32_window_class_atom = 0;
	}
	bool Window_Win32::createWindow()
	{
		if (win32_window_class_atom == 0)
		{
			return false;
		}

		// 直接创建窗口

		convertTitleText();
		if (!m_redirect_bitmap)
		{
			win32_window_style_ex |= WS_EX_NOREDIRECTIONBITMAP;
		}
		else
		{
			win32_window_style_ex &= ~DWORD(WS_EX_NOREDIRECTIONBITMAP);
		}
		win32_window = CreateWindowExW(
			win32_window_style_ex,
			win32_window_class.lpszClassName,
			win32_window_text_w.data(),
			win32_window_style,
			0, 0, (int)win32_window_width, (int)win32_window_height,
			NULL, NULL, win32_window_class.hInstance, this);
		if (win32_window == NULL)
		{
			spdlog::error("[luastg] (LastError = {}) CreateWindowExW failed", GetLastError());
			return false;
		}

		// 丢到显示器中间

		setCentered(false, nullptr);

		// 配置输入法

		if (!win32_window_ime_enable) {
			ImmAssociateContext(win32_window, nullptr);
		}

		// 配置窗口挪动器

		m_sizemove.setWindow(win32_window);

		// 标题栏控制器

		m_title_bar_controller.setEnable(auto_hide_title_bar && !m_fullscreen_mode);

		// 窗口样式

		Platform::WindowTheme::UpdateColorMode(win32_window, TRUE);
		setWindowCornerPreference(m_allow_windows_11_window_corner);
		if (win32_window_icon_id) {
			SendMessageW(win32_window, LUASTG_WM_SETICON, 0, 0);
		}

		return true;
	}
	void Window_Win32::destroyWindow()
	{
		m_sizemove.setWindow(NULL);
		if (win32_window) {
			DestroyWindow(win32_window);
			win32_window = NULL;
		}
	}
	bool Window_Win32::recreateWindow()
	{
		dispatchEvent(EventType::WindowDestroy);
		SendMessageW(win32_window, LUASTG_WM_RECREATE, 0, 0);
		ShowWindow(win32_window, SW_SHOWDEFAULT);
		dispatchEvent(EventType::WindowCreate);
		return true;
	}
	void Window_Win32::_toggleFullScreenMode()
	{
		if (m_fullscreen_mode) {
			SetWindowedModeParameters parameters{};
			parameters.size = Vector2U(win32_window_width, win32_window_height);
			parameters.style = m_framestyle;
			_setWindowMode(&parameters, true);
		}
		else {
			_setFullScreenMode(nullptr);
		}
	}
	void Window_Win32::_setWindowMode(SetWindowedModeParameters* parameters, bool ignore_size)
	{
		assert(parameters);

		m_title_bar_controller.setEnable(auto_hide_title_bar);

		HMONITOR win32_monitor{};
		if (parameters->display) {
			win32_monitor = static_cast<HMONITOR>(parameters->display->getNativeHandle());
		}
		else {
			win32_monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST);
		}
		assert(win32_monitor);
		MONITORINFO monitor_info = {};
		monitor_info.cbSize = sizeof(monitor_info);
		BOOL const get_monitor_info_result = GetMonitorInfoW(win32_monitor, &monitor_info);
		assert(get_monitor_info_result); (void)get_monitor_info_result;
		assert(monitor_info.rcMonitor.right > monitor_info.rcMonitor.left);
		assert(monitor_info.rcMonitor.bottom > monitor_info.rcMonitor.top);

		bool const new_fullsceen_mode = false;
		m_framestyle = parameters->style;
		DWORD new_win32_window_style = mapWindowStyle(m_framestyle, new_fullsceen_mode);

		RECT rect = { 0, 0, (int32_t)parameters->size.x, (int32_t)parameters->size.y };
		m_title_bar_controller.adjustWindowRectExForDpi(
			&rect, new_win32_window_style, FALSE, 0,
			win32::getDpiForWindow(win32_window));

		//m_ignore_size_message = TRUE;
		SetLastError(0);
		SetWindowLongPtrW(win32_window, GWL_STYLE, new_win32_window_style);
		DWORD const set_style_result = GetLastError();
		assert(set_style_result == 0); (void)set_style_result;
		//SetLastError(0);
		//SetWindowLongPtrW(win32_window, GWL_EXSTYLE, 0);
		//DWORD const set_style_ex_result = GetLastError();
		//assert(set_style_ex_result == 0); (void)set_style_ex_result;
		//m_ignore_size_message = FALSE;

		bool want_restore_placement = false;

		if (m_fullscreen_mode && ignore_size)
		{
			want_restore_placement = true;
		}
		else
		{
			BOOL const set_window_pos_result = SetWindowPos(
				win32_window,
				m_hidewindow ? NULL : HWND_TOP,
				(monitor_info.rcMonitor.right + monitor_info.rcMonitor.left) / 2 - (rect.right - rect.left) / 2,
				(monitor_info.rcMonitor.bottom + monitor_info.rcMonitor.top) / 2 - (rect.bottom - rect.top) / 2,
				rect.right - rect.left,
				rect.bottom - rect.top,
				SWP_FRAMECHANGED | (m_hidewindow ? SWP_NOZORDER : SWP_SHOWWINDOW));
			assert(set_window_pos_result); (void)set_window_pos_result;
		}

		RECT client_rect = {};
		BOOL get_client_rect_result = GetClientRect(win32_window, &client_rect);
		assert(get_client_rect_result); (void)get_client_rect_result;

		m_fullscreen_mode = new_fullsceen_mode;
		win32_window_style = new_win32_window_style;
		win32_window_width = UINT(client_rect.right - client_rect.left);
		win32_window_height = UINT(client_rect.bottom - client_rect.top);

		EventData event_data{};
		event_data.window_fullscreen_state = false;
		dispatchEvent(EventType::WindowFullscreenStateChange, event_data);

		if (want_restore_placement)
		{
			BOOL const set_placement_result = SetWindowPlacement(win32_window, &m_last_window_placement);
			assert(set_placement_result); (void)set_placement_result;
		}
	}
	void Window_Win32::_setFullScreenMode(IDisplay* display)
	{
		m_title_bar_controller.setEnable(false);

		if (!m_fullscreen_mode)
		{
			BOOL const get_placement_result = GetWindowPlacement(win32_window, &m_last_window_placement);
			assert(get_placement_result); (void)get_placement_result;
		}

		HMONITOR win32_monitor{};
		if (display) {
			win32_monitor = static_cast<HMONITOR>(display->getNativeHandle());
		}
		else {
			win32_monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST);
		}
		assert(win32_monitor);
		MONITORINFO monitor_info = {};
		monitor_info.cbSize = sizeof(monitor_info);
		BOOL const get_monitor_info_result = GetMonitorInfoW(win32_monitor, &monitor_info);
		assert(get_monitor_info_result); (void)get_monitor_info_result;
		assert(monitor_info.rcMonitor.right > monitor_info.rcMonitor.left);
		assert(monitor_info.rcMonitor.bottom > monitor_info.rcMonitor.top);

		bool const new_fullsceen_mode = true;
		DWORD new_win32_window_style = mapWindowStyle(m_framestyle, new_fullsceen_mode);

		//m_ignore_size_message = TRUE;
		SetLastError(0);
		SetWindowLongPtrW(win32_window, GWL_STYLE, new_win32_window_style);
		DWORD const set_style_result = GetLastError();
		assert(set_style_result == 0); (void)set_style_result;
		//SetLastError(0);
		//SetWindowLongPtrW(win32_window, GWL_EXSTYLE, 0);
		//DWORD const set_style_ex_result = GetLastError();
		//assert(set_style_ex_result == 0); (void)set_style_ex_result;
		//m_ignore_size_message = FALSE;

		BOOL const set_window_pos_result = SetWindowPos(
			win32_window,
			HWND_TOP,
			monitor_info.rcMonitor.left,
			monitor_info.rcMonitor.top,
			monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
			monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		assert(set_window_pos_result); (void)set_window_pos_result;

		m_fullscreen_mode = new_fullsceen_mode;
		win32_window_style = new_win32_window_style;
		win32_window_width = UINT(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
		win32_window_height = UINT(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);

		EventData event_data{};
		event_data.window_fullscreen_state = true;
		dispatchEvent(EventType::WindowFullscreenStateChange, event_data);
	}

	void Window_Win32::convertTitleText()
	{
		win32_window_text_w[0] = L'\0';
		int const size = MultiByteToWideChar(CP_UTF8, 0, win32_window_text.data(), (int)win32_window_text.size(), NULL, 0);
		if (size <= 0 || size > (int)(win32_window_text_w.size() - 1))
		{
			assert(false); return;
		}
		win32_window_text_w[size] = L'\0';
		int const result = MultiByteToWideChar(CP_UTF8, 0, win32_window_text.data(), (int)win32_window_text.size(), win32_window_text_w.data(), size);
		if (result <= 0 || result != size)
		{
			assert(false); return;
		}
		win32_window_text_w[result] = L'\0';
	}

	RectI Window_Win32::getRect()
	{
		RECT rc = {};
		GetWindowRect(win32_window, &rc);
		return RectI(rc.left, rc.top, rc.right, rc.bottom);
	}
	bool Window_Win32::setRect(RectI v)
	{
		return SetWindowPos(win32_window, NULL,
			v.a.x, v.a.y,
			v.b.x - v.a.x, v.b.y - v.a.y,
			SWP_NOZORDER) != FALSE;
	}
	RectI Window_Win32::getClientRect()
	{
		RECT rc = {};
		GetClientRect(win32_window, &rc);
		return RectI(rc.left, rc.top, rc.right, rc.bottom);
	}
	bool Window_Win32::setClientRect(RectI v)
	{
		// 更新 DPI
		win32_window_dpi = win32::getDpiForWindow(win32_window);
		// 计算包括窗口框架的尺寸
		RECT rc = { v.a.x , v.a.y , v.b.x , v.b.y };
		m_title_bar_controller.adjustWindowRectExForDpi(
			&rc,
			win32_window_style,
			FALSE,
			win32_window_style_ex,
			win32_window_dpi);
		// 获取最近的显示器的位置
		if (HMONITOR monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST))
		{
			MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
			if (GetMonitorInfoA(monitor, &moninfo))
			{
				// 偏移到该显示器0点位置
				rc.left += moninfo.rcMonitor.left;
				rc.right += moninfo.rcMonitor.left;
				rc.top += moninfo.rcMonitor.top;
				rc.bottom += moninfo.rcMonitor.top;
			}
		}
		// 最后再应用
		return SetWindowPos(win32_window, NULL,
			rc.left,
			rc.top,
			rc.right - rc.left,
			rc.bottom - rc.top,
			SWP_NOZORDER) != FALSE;
	}
	uint32_t Window_Win32::getDPI()
	{
		win32_window_dpi = win32::getDpiForWindow(win32_window);
		return win32_window_dpi;
	}
	void Window_Win32::setRedirectBitmapEnable(bool enable)
	{
		m_redirect_bitmap = enable ? TRUE : FALSE;
	}
	bool Window_Win32::getRedirectBitmapEnable()
	{
		return m_redirect_bitmap;
	}

	void Window_Win32::dispatchEvent(EventType t, EventData d)
	{
		// 回调
		m_is_dispatch_event = true;
		switch (t)
		{
		case EventType::WindowCreate:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowCreate();
			}
			break;
		case EventType::WindowDestroy:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowDestroy();
			}
			break;
		case EventType::WindowActive:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowActive();
			}
			break;
		case EventType::WindowInactive:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowInactive();
			}
			break;
		case EventType::WindowClose:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowClose();
			}
			break;
		case EventType::WindowSize:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowSize(d.window_size);
			}
			break;
		case EventType::WindowFullscreenStateChange:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowFullscreenStateChange(d.window_fullscreen_state);
			}
			break;
		case EventType::WindowDpiChanged:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowDpiChange();
			}
			break;
		case EventType::DeviceChange:
			for (auto& v : m_eventobj)
			{
				if (v) v->onDeviceChange();
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
	void Window_Win32::Window_Win32::addEventListener(IWindowEventListener* e)
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
	void Window_Win32::Window_Win32::removeEventListener(IWindowEventListener* e)
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

	void* Window_Win32::getNativeHandle() { return win32_window; }
	void Window_Win32::setNativeIcon(void* id)
	{
		win32_window_icon_id = (INT_PTR)id;
		SendMessageW(win32_window, LUASTG_WM_SETICON, 0, 0);
	}

	void Window_Win32::setIMEState(bool enable)
	{
		if (!win32_window_ime_enable && enable) {
		#pragma warning(push)
		#pragma warning(disable: 6387)
			// See: https://learn.microsoft.com/en-us/windows/win32/api/imm/nf-imm-immassociatecontextex
			// If the application calls this function with IACE_DEFAULT, the operating system restores the default input method context for the window.
			ImmAssociateContextEx(win32_window, nullptr /* In this case, the hIMC parameter is ignored. */, IACE_DEFAULT);
		#pragma warning (pop)
		}
		else if (win32_window_ime_enable && !enable) {
			ImmAssociateContext(win32_window, nullptr);
		}
		win32_window_ime_enable = enable;
	}
	bool Window_Win32::getIMEState()
	{
		return win32_window_ime_enable;
	}

	void Window_Win32::setTitleText(StringView str)
	{
		win32_window_text = str;
		m_title_bar_controller.setTitle(std::string(str));
		convertTitleText();
		PostMessageW(win32_window, LUASTG_WM_UPDAE_TITLE, 0, 0);
	}
	StringView Window_Win32::getTitleText()
	{
		return win32_window_text;
	}

	bool Window_Win32::setFrameStyle(WindowFrameStyle style)
	{
		m_framestyle = style;
		win32_window_style = mapWindowStyle(m_framestyle, m_fullscreen_mode);
		SetWindowLongPtrW(win32_window, GWL_STYLE, win32_window_style);
		//SetWindowLongPtrW(win32_window, GWL_EXSTYLE, win32_window_style_ex);
		UINT const flags = (SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE) | (!m_hidewindow ? SWP_SHOWWINDOW : 0);
		SetWindowPos(win32_window, NULL, 0, 0, 0, 0, flags);
		return true;
	}
	WindowFrameStyle Window_Win32::getFrameStyle()
	{
		return m_framestyle;
	}

	Vector2U Window_Win32::getSize()
	{
		return { win32_window_width, win32_window_height };
	}
	Vector2U Window_Win32::_getCurrentSize() {
		RECT rc{};
		GetClientRect(win32_window, &rc);
		return Vector2U(static_cast<uint32_t>(rc.right - rc.left), static_cast<uint32_t>(rc.bottom - rc.top));
	}
	bool Window_Win32::setSize(Vector2U v)
	{
		win32_window_width = v.x;
		win32_window_height = v.y;
		return setClientRect(RectI(0, 0, (int)v.x, (int)v.y));
	}

	WindowLayer Window_Win32::getLayer()
	{
		if (m_hidewindow)
			return WindowLayer::Invisible;
		if (WS_EX_TOPMOST & GetWindowLongPtrW(win32_window, GWL_EXSTYLE))
			return WindowLayer::TopMost;
		return WindowLayer::Unknown;
	}
	bool Window_Win32::setLayer(WindowLayer layer)
	{
		HWND pLayer = NULL;
		switch (layer)
		{
		default:
		case WindowLayer::Unknown:
			assert(false); return false;
		case WindowLayer::Invisible:
			pLayer = NULL;
			break;
		case WindowLayer::Bottom:
			pLayer = HWND_BOTTOM;
			break;
		case WindowLayer::Normal:
			pLayer = HWND_NOTOPMOST;
			break;
		case WindowLayer::Top:
			pLayer = HWND_TOP;
			break;
		case WindowLayer::TopMost:
			pLayer = HWND_TOPMOST;
			break;
		}
		UINT flags = (SWP_NOMOVE | SWP_NOSIZE);
		if (layer == WindowLayer::Invisible)
		{
			flags |= (SWP_NOZORDER | SWP_HIDEWINDOW);
			m_hidewindow = TRUE;
		}
		else
		{
			flags |= (SWP_SHOWWINDOW);
			m_hidewindow = FALSE;
		}
		return SetWindowPos(win32_window, pLayer, 0, 0, 0, 0, flags) != FALSE;
	}

	float Window_Win32::getDPIScaling()
	{
		return win32::getScalingFromDpi(getDPI());
	}

	void Window_Win32::setWindowMode(Vector2U size, WindowFrameStyle style, IDisplay* display)
	{
		SetWindowedModeParameters parameters{};
		parameters.size = size;
		parameters.style = style;
		parameters.display = display;
		SendMessageW(win32_window, LUASTG_WM_SET_WINDOW_MODE, reinterpret_cast<WPARAM>(&parameters), FALSE);
	}
	void Window_Win32::setFullScreenMode(IDisplay* display)
	{
		SendMessageW(win32_window, LUASTG_WM_SET_FULLSCREEN_MODE, 0, reinterpret_cast<LPARAM>(display));
	}
	void Window_Win32::setCentered(bool show, IDisplay* display) {
		Core::ScopeObject<IDisplay> local_display;
		if (!display) {
			if (!IDisplay::getNearestFromWindow(this, ~local_display)) {
				return;
			}
			display = local_display.get();
		}
		RECT r{};
		[[maybe_unused]] auto const result1 = GetWindowRect(win32_window, &r);
		assert(result1);
		auto const& m = display->getRect();
		UINT flags = SWP_FRAMECHANGED;
		if (show) {
			flags |= SWP_SHOWWINDOW;
		}
		[[maybe_unused]] auto const result2 = SetWindowPos(
			win32_window,
			HWND_TOP,
			(m.a.x + m.b.x) / 2 - (r.right - r.left) / 2,
			(m.a.y + m.b.y) / 2 - (r.bottom - r.top) / 2,
			r.right - r.left,
			r.bottom - r.top,
			flags);
		assert(result2);
	}

	void Window_Win32::setCustomSizeMoveEnable(bool v)
	{
		m_sizemove.setEnable(v ? TRUE : FALSE);
	}
	void Window_Win32::setCustomMinimizeButtonRect(RectI v)
	{
		m_sizemove.setMinimizeButtonRect(RECT{
			.left = v.a.x,
			.top = v.a.y,
			.right = v.b.x,
			.bottom = v.b.y,
			});
	}
	void Window_Win32::setCustomCloseButtonRect(RectI v)
	{
		m_sizemove.setCloseButtonRect(RECT{
			.left = v.a.x,
			.top = v.a.y,
			.right = v.b.x,
			.bottom = v.b.y,
			});
	}
	void Window_Win32::setCustomMoveButtonRect(RectI v)
	{
		m_sizemove.setTitleBarRect(RECT{
			.left = v.a.x,
			.top = v.a.y,
			.right = v.b.x,
			.bottom = v.b.y,
			});
	}

	bool Window_Win32::setCursor(WindowCursor type)
	{
		m_cursor = type;
		switch (type)
		{
		default:
			assert(false); return false;

		case WindowCursor::None:
			win32_window_cursor = NULL;
			break;

		case WindowCursor::Arrow:
			win32_window_cursor = LoadCursor(NULL, IDC_ARROW);
			break;
		case WindowCursor::Hand:
			win32_window_cursor = LoadCursor(NULL, IDC_HAND);
			break;

		case WindowCursor::Cross:
			win32_window_cursor = LoadCursor(NULL, IDC_CROSS);
			break;
		case WindowCursor::TextInput:
			win32_window_cursor = LoadCursor(NULL, IDC_IBEAM);
			break;

		case WindowCursor::Resize:
			win32_window_cursor = LoadCursor(NULL, IDC_SIZEALL);
			break;
		case WindowCursor::ResizeEW:
			win32_window_cursor = LoadCursor(NULL, IDC_SIZEWE);
			break;
		case WindowCursor::ResizeNS:
			win32_window_cursor = LoadCursor(NULL, IDC_SIZENS);
			break;
		case WindowCursor::ResizeNESW:
			win32_window_cursor = LoadCursor(NULL, IDC_SIZENESW);
			break;
		case WindowCursor::ResizeNWSE:
			win32_window_cursor = LoadCursor(NULL, IDC_SIZENWSE);
			break;

		case WindowCursor::NotAllowed:
			win32_window_cursor = LoadCursor(NULL, IDC_NO);
			break;
		case WindowCursor::Wait:
			win32_window_cursor = LoadCursor(NULL, IDC_WAIT);
			break;
		}
		POINT pt = {};
		GetCursorPos(&pt);
		SetCursorPos(pt.x, pt.y);
		return true;
	}
	WindowCursor Window_Win32::getCursor()
	{
		return m_cursor;
	}

	void Window_Win32::setWindowCornerPreference(bool allow)
	{
		m_allow_windows_11_window_corner = allow;

		if (!Platform::WindowsVersion::Is11())
		{
			return;
		}

		assert(win32_window);

		DWM_WINDOW_CORNER_PREFERENCE attr = allow ? DWMWCP_DEFAULT : DWMWCP_DONOTROUND;
		HRESULT hr = gHR = dwmapi_loader.SetWindowAttribute(
			win32_window, DWMWA_WINDOW_CORNER_PREFERENCE, &attr, sizeof(attr));
		if (FAILED(hr))
		{
			std::string msg;
			msg.reserve(64);
			msg.append("DwmSetWindowAttribute -> ");
			msg.append(allow ? "DWMWCP_DEFAULT" : "DWMWCP_DONOTROUND");
			i18n_core_system_call_report_error(msg);
		}
	}
	void Window_Win32::setTitleBarAutoHidePreference(bool allow) {
		auto_hide_title_bar = allow;
		m_title_bar_controller.setEnable(auto_hide_title_bar && !m_fullscreen_mode);
		if (!m_fullscreen_mode) {
			WINDOWPLACEMENT placement{ .length{sizeof(WINDOWPLACEMENT)} };
			GetWindowPlacement(win32_window, &placement);
			SetWindowPos(win32_window, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			SetWindowPlacement(win32_window, &placement);
		}
		else {
			SetWindowPos(win32_window, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		}
	}

	Window_Win32::Window_Win32()
	{
		enable_track_window_focus = core::ConfigurationLoader::getInstance().getDebug().isTrackWindowFocus();
		win32_window_dpi = win32::getUserDefaultScreenDpi();
		win32_window_text_w.fill(L'\0');
		if (!createWindowClass())
			throw std::runtime_error("createWindowClass failed");
		if (!createWindow())
			throw std::runtime_error("createWindow failed");
	}
	Window_Win32::~Window_Win32()
	{
		destroyWindow();
		destroyWindowClass();
	}

	bool Window_Win32::create(Window_Win32** pp_window)
	{
		try
		{
			*pp_window = new Window_Win32();
			return true;
		}
		catch (...)
		{
			*pp_window = nullptr;
			return false;
		}
	}
	bool Window_Win32::create(Vector2U size, StringView title_text, WindowFrameStyle style, bool show, Window_Win32** pp_window)
	{
		try
		{
			auto* p = new Window_Win32();
			*pp_window = p;
			p->setSize(size);
			p->setTitleText(title_text);
			p->setFrameStyle(style);
			if (show)
				p->setLayer(WindowLayer::Normal);
			return true;
		}
		catch (...)
		{
			*pp_window = nullptr;
			return false;
		}
	}

	bool IWindow::create(IWindow** pp_window)
	{
		try
		{
			*pp_window = new Window_Win32();
			return true;
		}
		catch (...)
		{
			*pp_window = nullptr;
			return false;
		}
	}
	bool IWindow::create(Vector2U size, StringView title_text, WindowFrameStyle style, bool show, IWindow** pp_window)
	{
		try
		{
			auto* p = new Window_Win32();
			*pp_window = p;
			p->setSize(size);
			p->setTitleText(title_text);
			p->setFrameStyle(style);
			if (show)
				p->setLayer(WindowLayer::Normal);
			return true;
		}
		catch (...)
		{
			*pp_window = nullptr;
			return false;
		}
	}
}
