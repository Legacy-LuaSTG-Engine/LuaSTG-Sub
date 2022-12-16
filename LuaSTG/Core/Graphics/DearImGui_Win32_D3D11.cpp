#include "Core/Graphics/DearImGui_Win32_D3D11.hpp"
#include "imgui.h"
#include <windowsx.h>

namespace Core::Graphics
{
	constexpr UINT const MSG_MOUSE_CAPTURE = WM_USER + 0x20;
	constexpr UINT const MSG_SET_IME_POS = WM_USER + 0x21;

	constexpr WPARAM const MSG_MOUSE_CAPTURE_SET = 1;
	constexpr WPARAM const MSG_MOUSE_CAPTURE_REL = 2;

	constexpr int const IM_VK_KEYPAD_ENTER = VK_RETURN + 256;

	static bool mapMouseCursor(LPWSTR* outValue)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		{
			return false;
		}
		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			*outValue = NULL;
		}
		else
		{
			LPWSTR win32_cursor = IDC_ARROW;
			switch (imgui_cursor)
			{
			case ImGuiMouseCursor_Arrow: win32_cursor = IDC_ARROW; break;
			case ImGuiMouseCursor_TextInput: win32_cursor = IDC_IBEAM; break;
			case ImGuiMouseCursor_ResizeAll: win32_cursor = IDC_SIZEALL; break;
			case ImGuiMouseCursor_ResizeEW: win32_cursor = IDC_SIZEWE; break;
			case ImGuiMouseCursor_ResizeNS: win32_cursor = IDC_SIZENS; break;
			case ImGuiMouseCursor_ResizeNESW: win32_cursor = IDC_SIZENESW; break;
			case ImGuiMouseCursor_ResizeNWSE: win32_cursor = IDC_SIZENWSE; break;
			case ImGuiMouseCursor_Hand: win32_cursor = IDC_HAND; break;
			case ImGuiMouseCursor_NotAllowed: win32_cursor = IDC_NO; break;
			}
			*outValue = win32_cursor;
		}
		return true;
	}
	static bool isVkDown(int vk)
	{
		return (::GetKeyState(vk) & 0x8000) != 0;
	}
	static void addKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(key, down);
		io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
		IM_UNUSED(native_scancode);
	}
	static ImGuiKey mapVirtualKeyToImGuiKey(WPARAM wParam)
	{
		switch (wParam)
		{
		case VK_TAB: return ImGuiKey_Tab;
		case VK_LEFT: return ImGuiKey_LeftArrow;
		case VK_RIGHT: return ImGuiKey_RightArrow;
		case VK_UP: return ImGuiKey_UpArrow;
		case VK_DOWN: return ImGuiKey_DownArrow;
		case VK_PRIOR: return ImGuiKey_PageUp;
		case VK_NEXT: return ImGuiKey_PageDown;
		case VK_HOME: return ImGuiKey_Home;
		case VK_END: return ImGuiKey_End;
		case VK_INSERT: return ImGuiKey_Insert;
		case VK_DELETE: return ImGuiKey_Delete;
		case VK_BACK: return ImGuiKey_Backspace;
		case VK_SPACE: return ImGuiKey_Space;
		case VK_RETURN: return ImGuiKey_Enter;
		case VK_ESCAPE: return ImGuiKey_Escape;
		case VK_OEM_7: return ImGuiKey_Apostrophe;
		case VK_OEM_COMMA: return ImGuiKey_Comma;
		case VK_OEM_MINUS: return ImGuiKey_Minus;
		case VK_OEM_PERIOD: return ImGuiKey_Period;
		case VK_OEM_2: return ImGuiKey_Slash;
		case VK_OEM_1: return ImGuiKey_Semicolon;
		case VK_OEM_PLUS: return ImGuiKey_Equal;
		case VK_OEM_4: return ImGuiKey_LeftBracket;
		case VK_OEM_5: return ImGuiKey_Backslash;
		case VK_OEM_6: return ImGuiKey_RightBracket;
		case VK_OEM_3: return ImGuiKey_GraveAccent;
		case VK_CAPITAL: return ImGuiKey_CapsLock;
		case VK_SCROLL: return ImGuiKey_ScrollLock;
		case VK_NUMLOCK: return ImGuiKey_NumLock;
		case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
		case VK_PAUSE: return ImGuiKey_Pause;
		case VK_NUMPAD0: return ImGuiKey_Keypad0;
		case VK_NUMPAD1: return ImGuiKey_Keypad1;
		case VK_NUMPAD2: return ImGuiKey_Keypad2;
		case VK_NUMPAD3: return ImGuiKey_Keypad3;
		case VK_NUMPAD4: return ImGuiKey_Keypad4;
		case VK_NUMPAD5: return ImGuiKey_Keypad5;
		case VK_NUMPAD6: return ImGuiKey_Keypad6;
		case VK_NUMPAD7: return ImGuiKey_Keypad7;
		case VK_NUMPAD8: return ImGuiKey_Keypad8;
		case VK_NUMPAD9: return ImGuiKey_Keypad9;
		case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
		case VK_DIVIDE: return ImGuiKey_KeypadDivide;
		case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case VK_ADD: return ImGuiKey_KeypadAdd;
		case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
		case VK_LSHIFT: return ImGuiKey_LeftShift;
		case VK_LCONTROL: return ImGuiKey_LeftCtrl;
		case VK_LMENU: return ImGuiKey_LeftAlt;
		case VK_LWIN: return ImGuiKey_LeftSuper;
		case VK_RSHIFT: return ImGuiKey_RightShift;
		case VK_RCONTROL: return ImGuiKey_RightCtrl;
		case VK_RMENU: return ImGuiKey_RightAlt;
		case VK_RWIN: return ImGuiKey_RightSuper;
		case VK_APPS: return ImGuiKey_Menu;
		case '0': return ImGuiKey_0;
		case '1': return ImGuiKey_1;
		case '2': return ImGuiKey_2;
		case '3': return ImGuiKey_3;
		case '4': return ImGuiKey_4;
		case '5': return ImGuiKey_5;
		case '6': return ImGuiKey_6;
		case '7': return ImGuiKey_7;
		case '8': return ImGuiKey_8;
		case '9': return ImGuiKey_9;
		case 'A': return ImGuiKey_A;
		case 'B': return ImGuiKey_B;
		case 'C': return ImGuiKey_C;
		case 'D': return ImGuiKey_D;
		case 'E': return ImGuiKey_E;
		case 'F': return ImGuiKey_F;
		case 'G': return ImGuiKey_G;
		case 'H': return ImGuiKey_H;
		case 'I': return ImGuiKey_I;
		case 'J': return ImGuiKey_J;
		case 'K': return ImGuiKey_K;
		case 'L': return ImGuiKey_L;
		case 'M': return ImGuiKey_M;
		case 'N': return ImGuiKey_N;
		case 'O': return ImGuiKey_O;
		case 'P': return ImGuiKey_P;
		case 'Q': return ImGuiKey_Q;
		case 'R': return ImGuiKey_R;
		case 'S': return ImGuiKey_S;
		case 'T': return ImGuiKey_T;
		case 'U': return ImGuiKey_U;
		case 'V': return ImGuiKey_V;
		case 'W': return ImGuiKey_W;
		case 'X': return ImGuiKey_X;
		case 'Y': return ImGuiKey_Y;
		case 'Z': return ImGuiKey_Z;
		case VK_F1: return ImGuiKey_F1;
		case VK_F2: return ImGuiKey_F2;
		case VK_F3: return ImGuiKey_F3;
		case VK_F4: return ImGuiKey_F4;
		case VK_F5: return ImGuiKey_F5;
		case VK_F6: return ImGuiKey_F6;
		case VK_F7: return ImGuiKey_F7;
		case VK_F8: return ImGuiKey_F8;
		case VK_F9: return ImGuiKey_F9;
		case VK_F10: return ImGuiKey_F10;
		case VK_F11: return ImGuiKey_F11;
		case VK_F12: return ImGuiKey_F12;
		default: return ImGuiKey_None;
		}
	}
	static void updateKeyModifiers()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiMod_Ctrl, isVkDown(VK_CONTROL));
		io.AddKeyEvent(ImGuiMod_Shift, isVkDown(VK_SHIFT));
		io.AddKeyEvent(ImGuiMod_Alt, isVkDown(VK_MENU));
		io.AddKeyEvent(ImGuiMod_Super, isVkDown(VK_APPS));
	}
	
	inline WPARAM convertImVec2ToWPARAM(ImVec2 const& v)
	{
		return ((int16_t)v.x) & (((int32_t)(int16_t)v.y) << 16);
	}
	inline ImVec2 convertWPARAMToImVec2(WPARAM const v)
	{
		return ImVec2((float)(int16_t)(v & 0xFFFFu), (float)(int16_t)((v & 0xFFFF0000u) >> 16));
	}
	static void updateIME(ImGuiViewport* viewport, ImGuiPlatformImeData* data)
	{
		if (viewport->PlatformHandleRaw)
		{
			PostMessageW((HWND)viewport->PlatformHandleRaw, MSG_SET_IME_POS, convertImVec2ToWPARAM(data->InputPos), 0);
		}
	}

	bool DearImGui_Backend::createWindowResources()
	{
		if (!QueryPerformanceFrequency(&m_freq))
			return false;
		if (!QueryPerformanceCounter(&m_time))
			return false;

		ImGuiIO& io = ImGui::GetIO();
		assert(io.BackendPlatformUserData == nullptr);
		io.BackendPlatformName = "LuaSTG Sub";
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendPlatformUserData = (void*)this;
		io.SetPlatformImeDataFn = &updateIME;

		ImGui::GetMainViewport()->PlatformHandleRaw = (void*)m_window->GetWindow();

		auto w_size_ =  m_window->getSize();
		io.DisplaySize = ImVec2((float)w_size_.x, (float)w_size_.y);

		return true;
	}
	void DearImGui_Backend::onWindowCreate()
	{
		createWindowResources();
	}
	void DearImGui_Backend::onWindowDestroy()
	{
		m_mouse_window = NULL;
		m_is_mouse_tracked = false;
		m_mouse_button_down = 0;
		m_last_cursor = (int)ImGuiMouseCursor_Arrow;

		ImGuiIO& io = ImGui::GetIO();
		assert(io.BackendPlatformUserData != nullptr);
		io.BackendPlatformName = nullptr;
		io.BackendPlatformUserData = nullptr;
	}
	IWindowEventListener::NativeWindowMessageResult DearImGui_Backend::onNativeWindowMessage(void* arg1, uint32_t arg2, uintptr_t arg3, intptr_t arg4)
	{
		HWND hWnd = (HWND)arg1; UINT uMsg = arg2; WPARAM wParam = arg3; LPARAM lParam = arg4;

		if (!ImGui::GetCurrentContext())
			return IWindowEventListener::NativeWindowMessageResult();

		switch (uMsg)
		{
			// -------- 需要特殊处理的鼠标消息 --------

		case WM_MOUSEMOVE:
			// 调用 TrackMouseEvent 来接收 WM_MOUSELEAVE 通知
			m_mouse_window = hWnd;
			if (m_is_mouse_tracked)
			{
				m_is_mouse_tracked = true;
				TRACKMOUSEEVENT tme = {
					.cbSize = sizeof(TRACKMOUSEEVENT),
					.dwFlags = TME_LEAVE,
					.hwndTrack = hWnd,
					.dwHoverTime = 0,
				};
				::TrackMouseEvent(&tme);
			}
			m_queue.write({ hWnd, uMsg, wParam, lParam });
			return IWindowEventListener::NativeWindowMessageResult();
		case WM_MOUSELEAVE:
			// 鼠标已经离开了
			if (m_mouse_window == hWnd)
			{
				m_mouse_window = NULL;
			}
			m_is_mouse_tracked = false;
			m_queue.write({ hWnd, uMsg, wParam, lParam });
			return IWindowEventListener::NativeWindowMessageResult();

			// -------- 要交给 ImGui 处理的消息 --------

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_CHAR:

		case WM_ACTIVATEAPP:
		case WM_SIZE:
			m_queue.write({ hWnd, uMsg, wParam, lParam });
			return IWindowEventListener::NativeWindowMessageResult();

			// -------- 从 ImGui 发送过来的消息（工作线程） --------

		case MSG_MOUSE_CAPTURE:
			switch (wParam)
			{
			case MSG_MOUSE_CAPTURE_SET: if (::GetCapture() == NULL) ::SetCapture(hWnd); break;
			case MSG_MOUSE_CAPTURE_REL: if (::GetCapture() == hWnd) ::ReleaseCapture(); break;
			}
			return IWindowEventListener::NativeWindowMessageResult(0, true);
		case MSG_SET_IME_POS:
			if (HIMC himc = ::ImmGetContext(hWnd))
			{
				ImVec2 const pt = convertWPARAMToImVec2(wParam);
				COMPOSITIONFORM cf = {
					.dwStyle = CFS_FORCE_POSITION,
					.ptCurrentPos = POINT{
						.x = (LONG)pt.x,
						.y = (LONG)pt.y,
					},
					.rcArea = RECT{},
				};
				::ImmSetCompositionWindow(himc, &cf);
				::ImmReleaseContext(hWnd, himc);
			}
			return IWindowEventListener::NativeWindowMessageResult(0, true);

			// -------- 其他消息（目前应该没有更多需要处理的消息） --------
		}

		return IWindowEventListener::NativeWindowMessageResult();
	}

	void DearImGui_Backend::processMessage()
	{
		ImGuiIO& io = ImGui::GetIO();
		Win32Message msg = {};

		auto processInputEvent = [&]() -> bool
		{
			// mouse

			switch (msg.uMsg)
			{
			case WM_MOUSEMOVE:
				io.AddMousePosEvent((float)GET_X_LPARAM(msg.lParam), (float)GET_Y_LPARAM(msg.lParam));
				return true;
			case WM_MOUSELEAVE:
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
				return true;
			case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
				do {
					int button = 0;
					if (msg.uMsg == WM_LBUTTONDOWN || msg.uMsg == WM_LBUTTONDBLCLK) { button = 0; }
					if (msg.uMsg == WM_RBUTTONDOWN || msg.uMsg == WM_RBUTTONDBLCLK) { button = 1; }
					if (msg.uMsg == WM_MBUTTONDOWN || msg.uMsg == WM_MBUTTONDBLCLK) { button = 2; }
					if (msg.uMsg == WM_XBUTTONDOWN || msg.uMsg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? 3 : 4; }
					if (m_mouse_button_down == 0) ::PostMessageW(m_window->GetWindow(), MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_SET, 0);
					m_mouse_button_down |= 1 << button;
					io.AddMouseButtonEvent(button, true);
				} while (false);
				return true;
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
				do {
					int button = 0;
					if (msg.uMsg == WM_LBUTTONUP) { button = 0; }
					if (msg.uMsg == WM_RBUTTONUP) { button = 1; }
					if (msg.uMsg == WM_MBUTTONUP) { button = 2; }
					if (msg.uMsg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? 3 : 4; }
					m_mouse_button_down &= ~(1 << button);
					if (m_mouse_button_down == 0) ::PostMessageW(m_window->GetWindow(), MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_REL, 0);
					io.AddMouseButtonEvent(button, false);
				} while (false);
				return true;
			case WM_MOUSEWHEEL:
				io.AddMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(msg.wParam) / (float)WHEEL_DELTA);
				return true;
			case WM_MOUSEHWHEEL:
				io.AddMouseWheelEvent((float)GET_WHEEL_DELTA_WPARAM(msg.wParam) / (float)WHEEL_DELTA, 0.0f);
				return true;
			}

			// keyboard

			switch (msg.uMsg)
			{
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				if (msg.wParam < 256)
				{
					bool const is_key_down = (msg.uMsg == WM_KEYDOWN || msg.uMsg == WM_SYSKEYDOWN);

					// Submit modifiers
					updateKeyModifiers();

					// Obtain virtual key code
					// (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
					int vk = (int)msg.wParam;
					if ((msg.wParam == VK_RETURN) && (HIWORD(msg.lParam) & KF_EXTENDED))
						vk = IM_VK_KEYPAD_ENTER;

					// Submit key event
					ImGuiKey const key = mapVirtualKeyToImGuiKey(vk);
					int const scancode = (int)LOBYTE(HIWORD(msg.lParam));
					if (key != ImGuiKey_None)
						addKeyEvent(key, is_key_down, vk, scancode);

					// Submit individual left/right modifier events
					if (vk == VK_SHIFT)
					{
						// Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
						if (isVkDown(VK_LSHIFT) == is_key_down) { addKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
						if (isVkDown(VK_RSHIFT) == is_key_down) { addKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
					}
					else if (vk == VK_CONTROL)
					{
						if (isVkDown(VK_LCONTROL) == is_key_down) { addKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
						if (isVkDown(VK_RCONTROL) == is_key_down) { addKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
					}
					else if (vk == VK_MENU)
					{
						if (isVkDown(VK_LMENU) == is_key_down) { addKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
						if (isVkDown(VK_RMENU) == is_key_down) { addKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }
					}
				}
				return true;
			case WM_SETFOCUS:
			case WM_KILLFOCUS:
				io.AddFocusEvent(msg.uMsg == WM_SETFOCUS);
				return true;
			case WM_CHAR:
				if (msg.wParam > 0 && msg.wParam < 0x10000) io.AddInputCharacterUTF16((uint16_t)msg.wParam);
				return true;
			}

			return false;
		};

		auto processOtherEvent = [&]() -> bool
		{
			switch (msg.uMsg)
			{
			case WM_ACTIVATEAPP:
				io.AddFocusEvent(msg.wParam == TRUE);
				return true;
			case WM_SIZE:
				io.DisplaySize = ImVec2((float)(LOWORD(msg.lParam)), (float)(HIWORD(msg.lParam)));
				return true;
			}

			return false;
		};

		for (size_t cnt = 0; cnt < m_queue.size && m_queue.read(msg); cnt += 1)
		{
			if (!processInputEvent())
			{
				processOtherEvent();
			}
		}

		// Left & right Shift keys: when both are pressed together, Windows tend to not generate the WM_KEYUP event for the first released one.
		if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !isVkDown(VK_LSHIFT))
			addKeyEvent(ImGuiKey_LeftShift, false, VK_LSHIFT);
		if (ImGui::IsKeyDown(ImGuiKey_RightShift) && !isVkDown(VK_RSHIFT))
			addKeyEvent(ImGuiKey_RightShift, false, VK_RSHIFT);

		// Sometimes WM_KEYUP for Win key is not passed down to the app (e.g. for Win+V on some setups, according to GLFW).
		if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) && !isVkDown(VK_LWIN))
			addKeyEvent(ImGuiKey_LeftSuper, false, VK_LWIN);
		if (ImGui::IsKeyDown(ImGuiKey_RightSuper) && !isVkDown(VK_RWIN))
			addKeyEvent(ImGuiKey_RightSuper, false, VK_RWIN);
	}
	bool DearImGui_Backend::updateMouseCursor()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
			return false;

		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			m_window->setCursor(WindowCursor::None);
		}
		else
		{
			// Show OS mouse cursor
			switch (imgui_cursor)
			{
			case ImGuiMouseCursor_Arrow:        m_window->setCursor(WindowCursor::Arrow); break;
			case ImGuiMouseCursor_TextInput:    m_window->setCursor(WindowCursor::TextInput); break;
			case ImGuiMouseCursor_ResizeAll:    m_window->setCursor(WindowCursor::Resize); break;
			case ImGuiMouseCursor_ResizeEW:     m_window->setCursor(WindowCursor::ResizeEW); break;
			case ImGuiMouseCursor_ResizeNS:     m_window->setCursor(WindowCursor::ResizeNS); break;
			case ImGuiMouseCursor_ResizeNESW:   m_window->setCursor(WindowCursor::ResizeNESW); break;
			case ImGuiMouseCursor_ResizeNWSE:   m_window->setCursor(WindowCursor::ResizeNWSE); break;
			case ImGuiMouseCursor_Hand:         m_window->setCursor(WindowCursor::Hand); break;
			case ImGuiMouseCursor_NotAllowed:   m_window->setCursor(WindowCursor::NotAllowed); break;
			}
		}

		return true;
	}

	void DearImGui_Backend::update()
	{
		ImGuiIO& io = ImGui::GetIO();
		assert(io.BackendPlatformUserData != nullptr);

		processMessage();

		LARGE_INTEGER current_time = {};
		::QueryPerformanceCounter(&current_time);
		io.DeltaTime = (float)(current_time.QuadPart - m_time.QuadPart) / (float)m_freq.QuadPart;
		m_time = current_time;
		
		auto w_size_ = m_window->getSize();
		io.DisplaySize = ImVec2((float)w_size_.x, (float)w_size_.y);

		ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
		if (m_last_cursor != mouse_cursor)
		{
			m_last_cursor = mouse_cursor;
			updateMouseCursor();
		}
	}

	DearImGui_Backend::DearImGui_Backend(Window_Win32* p_window, Device_D3D11* p_device)
		: m_window(p_window)
		, m_device(p_device)
	{
		if (!createWindowResources())
			throw std::runtime_error("DearImGui_Backend::DearImGui_Backend");
		m_window->addEventListener(this);
		m_device->addEventListener(this);
	}
	DearImGui_Backend::~DearImGui_Backend()
	{
		m_window->removeEventListener(this);
		m_device->removeEventListener(this);
	}
}
