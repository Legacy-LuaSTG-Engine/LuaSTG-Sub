#include <iostream>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Mouse.h>
#include <Keyboard.h>

struct MainWindow
{
	WNDCLASSEXW wndcls;
	ATOM clsatom;
	HWND window;

	DirectX::Mouse mouse;
	DirectX::Keyboard keyboard;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (MainWindow* self = (MainWindow*)GetWindowLongPtrW(hWnd, GWLP_USERDATA))
		{
			return self->wincb(hWnd, uMsg, wParam, lParam);
		}
		switch (uMsg)
		{
		case WM_CREATE:
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCTW*)lParam)->lpCreateParams);
			return 0;
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	LRESULT wincb(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
			break;
		}

		switch (uMsg)
		{
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
			break;
		}

		switch (uMsg)
		{
		case WM_CLOSE:
			PostQuitMessage(EXIT_SUCCESS);
			return 0;
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	MainWindow()
		: wndcls({sizeof(WNDCLASSEXW)})
		, clsatom(0)
		, window(NULL)
	{
		wndcls.style = CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = &WndProc;
		wndcls.hInstance = GetModuleHandleW(NULL);
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.lpszClassName = L"MainWindow";

		clsatom = RegisterClassExW(&wndcls);
		
		window = CreateWindowExW(0, wndcls.lpszClassName, L"MainWindow", WS_OVERLAPPEDWINDOW, 0, 0, 1280, 720, NULL, NULL, wndcls.hInstance, this);

		ShowWindow(window, SW_SHOWDEFAULT);
		UpdateWindow(window);

		mouse.SetWindow(window);

		int tim = 0;

		MSG msg = {};
		//for (;;)
		//{
		//	BOOL const result = GetMessageW(&msg, NULL, 0, 0);
		//	if (result > FALSE)
		//	{
		//		TranslateMessage(&msg);
		//		DispatchMessageW(&msg);
		//	}
		//	else
		//	{
		//		break;
		//	}
		//}
		BOOL exit = FALSE;
		while (!exit)
		{
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					exit = TRUE;
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}

			DirectX::Mouse::State v = mouse.GetState();
			mouse.ResetScrollWheelValue();
			char buffer[256] = {};
			std::snprintf(buffer, 256, "[%d] (%d, %d) (%d) %d%d%d%d%d\n", tim++, v.x, v.y, v.scrollWheelValue, v.leftButton, v.middleButton, v.rightButton, v.xButton1, v.xButton2);
			OutputDebugStringA(buffer);

			Sleep(100);
		}
	}
	~MainWindow()
	{
		if (window) DestroyWindow(window);
		if (clsatom) UnregisterClassW(wndcls.lpszClassName, wndcls.hInstance);
	}
};

_Use_decl_anno_impl_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
	MainWindow ma;
	return EXIT_SUCCESS;
}
