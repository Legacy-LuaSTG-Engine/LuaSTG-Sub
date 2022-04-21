#include "Core/ApplicationModel.hpp"

#include <string>
#include <vector>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "resource.h"
#include "platform/HighDPI.hpp"
#include "utility/encoding.hpp"

namespace LuaSTG::Core
{
	class Application : public IApplication
	{
	private:
		volatile intptr_t ref_{ 1 };
	public:
		intptr_t retain()
		{
			ref_++;
			return ref_;
		}
		intptr_t release()
		{
			ref_--;
			intptr_t tmp_ = ref_;
			if (ref_ < 1)
			{
				delete this;
			}
			return tmp_;
		}

		// Event Listener
	private:
		std::vector<IApplicationEventListener*> app_event_listener;
		std::vector<IWindowEventListener*> window_event_listener;
	public:
		void addApplicationEventListener(IApplicationEventListener* e)
		{
			removeApplicationEventListener(e);
			if (e) app_event_listener.emplace_back(e);
		}
		void removeApplicationEventListener(IApplicationEventListener* e)
		{
			for (auto it = app_event_listener.begin(); it != app_event_listener.end();)
			{
				if (*it == e)
				{
					it = app_event_listener.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
		#define dispatchApplicationEventListener(X, ...) for (auto e : app_event_listener) e->X(__VA_ARGS__);
		void addWindowEventListener(IWindowEventListener* e)
		{
			removeWindowEventListener(e);
			if (e) window_event_listener.emplace_back(e);
		}
		void removeWindowEventListener(IWindowEventListener* e)
		{
			for (auto it = window_event_listener.begin(); it != window_event_listener.end();)
			{
				if (*it == e)
				{
					it = window_event_listener.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
		#define dispatchWindowEventListener(X, ...) for (auto e : window_event_listener) e->X(__VA_ARGS__);

		// Main Window
	private:
		WNDCLASSEXW win32_window_class{ sizeof(WNDCLASSEXW) };
		ATOM win32_window_class_atom{ 0 };
		HWND win32_window{ NULL };
		HIMC win32_window_imc{ NULL };
		BOOL win32_window_ime_enable{ FALSE };
		DWORD win32_window_style{ WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX)};
		DWORD win32_window_style_ex{ 0 };
		DWORD win32_window_width{ 640 };
		DWORD win32_window_height{ 480 };
		UINT win32_window_dpi{ USER_DEFAULT_SCREEN_DPI };
		std::string win32_window_text{ "MainWindow" };
		
		static LRESULT CALLBACK win32_window_callback(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
		{
			if (Application* self = (Application*)GetWindowLongPtrW(window, GWLP_USERDATA))
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
		
		LRESULT onMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
		{
			switch (message)
			{
			case WM_NCCREATE:
				assert(false);
				break;
			case WM_CREATE:
				assert(false);
				break;
			case WM_CLOSE:
				PostQuitMessage(EXIT_SUCCESS);
				return 0;
			}
			return DefWindowProcW(window, message, arg1, arg2);
		}
		bool createMainWindowClass()
		{
			HINSTANCE hInstance = GetModuleHandleW(NULL);
			assert(hInstance); // 如果 hInstance 为 NULL 那肯定是见鬼了

			auto& cls = win32_window_class;
			cls.style = CS_HREDRAW | CS_VREDRAW;
			cls.lpfnWndProc = &win32_window_callback;
			cls.hInstance = hInstance;
			cls.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));
			cls.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
			cls.lpszClassName = L"LuaSTG::Core::MainWindow";
			cls.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));

			win32_window_class_atom = RegisterClassExW(&cls);
			if (win32_window_class_atom == 0)
			{
				spdlog::error("[luastg] (LastError = {}) RegisterClassExW failed", GetLastError());
				return false;
			}

			return true;
		}
		void destroyMainWindowClass()
		{
			if (win32_window_class_atom != 0)
			{
				UnregisterClassW(win32_window_class.lpszClassName, win32_window_class.hInstance);
			}
			win32_window_class_atom = 0;
		}
		bool createMainWindow()
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

			// 根据 DPI 调整窗口大小

			win32_window_dpi = platform::HighDPI::GetDpiForWindow(win32_window);
			if (win32_window_dpi == 0)
			{
				spdlog::error("[luastg] platform::HighDPI::GetDpiForWindow failed");
				return false;
			}
			
			RECT rc = {
				.left = 0,
				.top = 0,
				.right = (LONG)win32_window_width,
				.bottom = (LONG)win32_window_height,
			};
			if (!platform::HighDPI::AdjustWindowRectExForDpi(&rc, win32_window_style, FALSE, win32_window_style_ex, win32_window_dpi))
			{
				spdlog::error("[luastg] platform::HighDPI::AdjustWindowRectExForDpi failed");
				return false;
			}

			if (!SetWindowPos(win32_window, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE))
			{
				spdlog::error("[luastg] (LastError = {}) SetWindowPos failed", GetLastError());
				return false;
			}

			return true;
		}
		void destroyMainWindow()
		{
			if (win32_window)
			{
				DestroyWindow(win32_window);
			}
			win32_window = NULL;
			win32_window_imc = NULL;
		}
		bool recreateMainWindow()
		{
			dispatchWindowEventListener(onDestroy);
			destroyMainWindow();
			if (!createMainWindow()) return false;
			dispatchWindowEventListener(onCreate, win32_window);
			return true;
		}

	public:
		Application()
		{
			
		}
		~Application()
		{
			destroyMainWindow();
			destroyMainWindowClass();
		}
	};
}
