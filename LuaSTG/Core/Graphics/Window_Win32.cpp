#include "Core/Graphics/Window_Win32.hpp"
#include "Core/i18n.hpp"
#include "utility/encoding.hpp"
#include "platform/WindowsVersion.hpp"
#include "platform/HighDPI.hpp"
#include "platform/WindowTheme.hpp"

namespace LuaSTG::Core::Graphics
{
	LRESULT CALLBACK Window_Win32::win32_window_callback(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
	{
		if (Window_Win32* self = (Window_Win32*)GetWindowLongPtrW(window, GWLP_USERDATA))
		{
			return self->onMessage(window, message, arg1, arg2);
		}
		switch (message)
		{
		case WM_NCCREATE:
			platform::HighDPI::EnableNonClientDpiScaling(window);
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
		// 窗口挪动器
		auto rResult = m_sizemove.handleSizeMove(window, message, arg1, arg2);
		if (rResult.bReturn)
			return rResult.lResult;
		// 特殊
		for (auto& v : m_eventobj)
		{
			if (v) v->onNativeWindowMessage(window, message, arg1, arg2);
		}
		// 其他消息
		switch (message)
		{
		case WM_ACTIVATEAPP:
			if (arg1 /* == TRUE */)
			{
				platform::WindowTheme::UpdateColorMode(window, FALSE);
				dispatchEvent(EventType::WindowActive);
			}
			else
			{
				platform::WindowTheme::UpdateColorMode(window, TRUE);
				dispatchEvent(EventType::WindowInactive);
			}
			break;
		case WM_SIZE:
			break;
		case WM_ENTERSIZEMOVE:
			win32_window_is_sizemove = TRUE;
			InvalidateRect(window, NULL, FALSE); // 标记窗口区域为需要重新绘制，以便产生 WM_PAINT 消息
			return 0;
		case WM_EXITSIZEMOVE:
			win32_window_is_sizemove = FALSE;
			ValidateRect(window, NULL);
			return 0;
		case WM_PAINT:
			if (win32_window_is_sizemove)
			{
				dispatchEvent(EventType::WindowSizeMovePaint);
			}
			else
			{
				ValidateRect(window, NULL); // 正常情况下，WM_PAINT 忽略掉
			}
			return 0;
		case WM_DPICHANGED:
			if (getFrameStyle() != WindowFrameStyle::None)
			{
				setSize(getSize()); // 刷新一次尺寸（因为非客户区可能会变化）
				setCentered();
				return 0;
			}
			break;
		case WM_SETTINGCHANGE:
		case WM_THEMECHANGED:
			platform::WindowTheme::UpdateColorMode(window, TRUE);
			break;
		case WM_DEVICECHANGE:
			if (arg1 == 0x0007 /* DBT_DEVNODES_CHANGED */)
			{
				dispatchEvent(EventType::DeviceChange);
			}
			break;
		case WM_SETCURSOR:
			SetCursor(win32_window_cursor);
			return TRUE;
		case WM_MENUCHAR:
			// 快捷键能不能死全家
			return MAKELRESULT(0, MNC_CLOSE);
		case WM_SYSCOMMAND:
			// 别他妈让 ALT 键卡住消息循环，傻逼
			switch (arg1 & 0xFFF0)
			{
			case SC_KEYMENU: return 0;
			default: break;
			}
			break;
		case WM_CLOSE:
			dispatchEvent(EventType::WindowClose);
			PostQuitMessage(EXIT_SUCCESS);
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
		cls.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
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

		win32_window = CreateWindowExW(
			win32_window_style_ex,
			win32_window_class.lpszClassName,
			utility::encoding::to_wide(win32_window_text).c_str(),
			win32_window_style,
			0, 0, (int)win32_window_width, (int)win32_window_height,
			NULL, NULL, win32_window_class.hInstance, this);
		if (win32_window == NULL)
		{
			spdlog::error("[luastg] (LastError = {}) CreateWindowExW failed", GetLastError());
			return false;
		}

		// 配置输入法

		win32_window_imc = ImmAssociateContext(win32_window, NULL);
		if (win32_window_ime_enable)
		{
			ImmAssociateContext(win32_window, win32_window_imc);
		}

		// 配置窗口挪动器

		m_sizemove.setWindow(win32_window);

		// 暗色模式
		platform::WindowTheme::UpdateColorMode(win32_window, TRUE);

		return true;
	}
	void Window_Win32::destroyWindow()
	{
		m_sizemove.setWindow(NULL);
		if (win32_window)
		{
			if (win32_window_imc)
				ImmAssociateContext(win32_window, win32_window_imc);
			DestroyWindow(win32_window);
		}
		win32_window = NULL;
		win32_window_imc = NULL;
	}
	bool Window_Win32::recreateWindow()
	{
		dispatchEvent(EventType::WindowDestroy);
		destroyWindow();
		if (!createWindow()) return false;
		dispatchEvent(EventType::WindowCreate);
		return true;
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
		win32_window_dpi = platform::HighDPI::GetDpiForWindow(win32_window);
		// 计算包括窗口框架的尺寸
		RECT rc = { v.a.x , v.a.y , v.b.x , v.b.y };
		platform::HighDPI::AdjustWindowRectExForDpi(
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
		win32_window_dpi = platform::HighDPI::GetDpiForWindow(win32_window);
		return win32_window_dpi;
	}

	void Window_Win32::dispatchEvent(EventType t)
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
		case EventType::WindowSizeMovePaint:
			for (auto& v : m_eventobj)
			{
				if (v) v->onWindowSizeMovePaint();
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
		HICON hIcon = LoadIconW(win32_window_class.hInstance, MAKEINTRESOURCEW(id));
		SendMessageW(win32_window, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
		SendMessageW(win32_window, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
		DestroyIcon(hIcon);
	}

	void Window_Win32::setIMEState(bool enable)
	{
		if (win32_window_ime_enable)
			ImmAssociateContext(win32_window, win32_window_imc);
		else
			ImmAssociateContext(win32_window, NULL);
	}
	bool Window_Win32::getIMEState()
	{
		return win32_window_ime_enable;
	}

	void Window_Win32::setTitleText(StringView str)
	{
		win32_window_text = str;
		SetWindowTextW(win32_window, utility::encoding::to_wide(str).c_str());
	}
	StringView Window_Win32::getTitleText()
	{
		return win32_window_text;
	}

	bool Window_Win32::setFrameStyle(WindowFrameStyle style)
	{
		m_framestyle = style;
		switch (style)
		{
		default:
			assert(false); return false;
		case LuaSTG::Core::Graphics::WindowFrameStyle::None:
			win32_window_style = WS_POPUP;
			win32_window_style_ex = 0;
			break;
		case LuaSTG::Core::Graphics::WindowFrameStyle::Fixed:
			win32_window_style = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
			win32_window_style_ex = 0;
			break;
		case LuaSTG::Core::Graphics::WindowFrameStyle::Normal:
			win32_window_style = WS_OVERLAPPEDWINDOW;
			win32_window_style_ex = 0;
			break;
		}
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

	Vector2I Window_Win32::getSize()
	{
		return Vector2I((int32_t)win32_window_width, (int32_t)win32_window_height);
	}
	bool Window_Win32::setSize(Vector2I v)
	{
		win32_window_width = (UINT)v.x;
		win32_window_height = (UINT)v.y;
		return setClientRect(RectI(0, 0, v.x, v.y));
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
		case LuaSTG::Core::Graphics::WindowLayer::Unknown:
			assert(false); return false;
		case LuaSTG::Core::Graphics::WindowLayer::Invisible:
			pLayer = NULL;
			break;
		case LuaSTG::Core::Graphics::WindowLayer::Bottom:
			pLayer = HWND_BOTTOM;
			break;
		case LuaSTG::Core::Graphics::WindowLayer::Normal:
			pLayer = HWND_NOTOPMOST;
			break;
		case LuaSTG::Core::Graphics::WindowLayer::Top:
			pLayer = HWND_TOP;
			break;
		case LuaSTG::Core::Graphics::WindowLayer::TopMost:
			pLayer = HWND_TOPMOST;
			break;
		}
		UINT flags = (SWP_NOMOVE | SWP_NOSIZE);
		if (layer == LuaSTG::Core::Graphics::WindowLayer::Invisible)
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
		return (float)getDPI() / (float)USER_DEFAULT_SCREEN_DPI;
	}

	Vector2I Window_Win32::getMonitorSize()
	{
		RECT area = {};
		if (platform::MonitorList::GetRectFromWindow(win32_window, &area))
		{
			return Vector2I(area.right - area.left, area.bottom - area.top);
		}
		return Vector2I();
	}
	void Window_Win32::setCentered()
	{
		platform::MonitorList::MoveWindowToCenter(win32_window);
	}
	void Window_Win32::setFullScreen()
	{
		platform::MonitorList::ResizeWindowToFullScreen(win32_window);
		setCursorToRightBottom();
	}

	uint32_t Window_Win32::getMonitorCount()
	{
		m_monitors.Refresh();
		return m_monitors.GetCount();
	}
	RectI Window_Win32::getMonitorRect(uint32_t index)
	{
		RECT const rc = m_monitors.GetRect(index);
		return RectI(rc.left, rc.top, rc.right, rc.bottom);
	}
	void Window_Win32::setMonitorCentered(uint32_t index)
	{
		m_monitors.MoveWindowToCenter(index, win32_window);
	}
	void Window_Win32::setMonitorFullScreen(uint32_t index)
	{
		m_monitors.ResizeWindowToFullScreen(index, win32_window);
		setCursorToRightBottom();
	}

	void Window_Win32::setCustomSizeMoveEnable(bool v)
	{
		m_sizemove.setEnable(v ? TRUE : FALSE);
	}
	void Window_Win32::setCustomMinimizeButtonRect(RectI v)
	{
		m_sizemove.setMinimizeButtonRect(RECT{
			.left   = v.a.x,
			.top    = v.a.y,
			.right  = v.b.x,
			.bottom = v.b.y,
		});
	}
	void Window_Win32::setCustomCloseButtonRect(RectI v)
	{
		m_sizemove.setCloseButtonRect(RECT{
			.left   = v.a.x,
			.top    = v.a.y,
			.right  = v.b.x,
			.bottom = v.b.y,
		});
	}
	void Window_Win32::setCustomMoveButtonRect(RectI v)
	{
		m_sizemove.setTitleBarRect(RECT{
			.left   = v.a.x,
			.top    = v.a.y,
			.right  = v.b.x,
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
		case LuaSTG::Core::Graphics::WindowCursor::None:
			win32_window_cursor = NULL;
			break;
		case LuaSTG::Core::Graphics::WindowCursor::Arrow:
			win32_window_cursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
			break;
		}
		return true;
	}
	WindowCursor Window_Win32::getCursor()
	{
		return m_cursor;
	}
	void Window_Win32::setCursorToRightBottom()
	{
		RECT rc = {};
		if (!GetClientRect(win32_window, &rc))
			return;
		POINT pt = { rc.right, rc.bottom };
		if (!ClientToScreen(win32_window, &pt))
			return;
		SetCursorPos(pt.x - 1, pt.y - 1);
	}

	Window_Win32::Window_Win32()
	{
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
	bool Window_Win32::create(Vector2I size, StringView title_text, WindowFrameStyle style, bool show, Window_Win32** pp_window)
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
	bool IWindow::create(Vector2I size, StringView title_text, WindowFrameStyle style, bool show, IWindow** pp_window)
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
