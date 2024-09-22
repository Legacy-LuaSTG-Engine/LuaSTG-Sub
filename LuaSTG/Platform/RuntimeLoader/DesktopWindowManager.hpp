#pragma once

namespace Platform::RuntimeLoader
{
	class DesktopWindowManager
	{
	private:
		HMODULE dll_dwmapi{};
		HMODULE dll_gdi32{};
		decltype(DwmIsCompositionEnabled)* api_DwmIsCompositionEnabled{};
		decltype(DwmEnableBlurBehindWindow)* api_DwmEnableBlurBehindWindow{};
		decltype(DwmExtendFrameIntoClientArea)* api_DwmExtendFrameIntoClientArea{};
		decltype(DwmGetColorizationColor)* api_DwmGetColorizationColor{};
		decltype(DwmDefWindowProc)* api_DwmDefWindowProc{};
		decltype(DwmSetWindowAttribute)* api_DwmSetWindowAttribute{};
		decltype(DwmGetWindowAttribute)* api_DwmGetWindowAttribute{};
		decltype(DwmFlush)* api_DwmFlush{};
		decltype(CreateRectRgn)* api_CreateRectRgn{};
		decltype(DeleteObject)* api_DeleteObject{};
	public:
		HRESULT IsCompositionEnabled(BOOL* pfEnabled);
		HRESULT EnableBlurBehindWindow(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind);
		HRESULT ExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset);
		HRESULT GetColorizationColor(DWORD* pcrColorization, BOOL* pfOpaqueBlend);
		BOOL DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);
		HRESULT SetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
		HRESULT GetWindowAttribute(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
		HRESULT Flush();

		// NOTE: Update window transparency on message
		//   WM_DWMCOMPOSITIONCHANGED
		//   WM_DWMCOLORIZATIONCOLORCHANGED

		BOOL IsWindowTransparencySupported();
		HRESULT SetWindowTransparency(HWND hWnd, BOOL bEnable);
	public:
		DesktopWindowManager();
		~DesktopWindowManager();
	};
}
