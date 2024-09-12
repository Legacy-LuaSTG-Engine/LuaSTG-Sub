#pragma once

namespace platform::windows {

	class ImmersiveTitleBarController {
	private:
		class Impl;
		Impl* impl;

	public:
		struct HandleWindowMessageResult {
			bool returnResult{ false };
			LRESULT result{ 0 };
		};

	public:
		void setEnable(bool enable);
		BOOL adjustWindowRectExForDpi(RECT* lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
		HandleWindowMessageResult handleWindowMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2);
		bool createResources(HWND window, ID2D1DeviceContext* context);
		void destroyResources();
		bool setTitle(std::string const& text);
		bool setClientSize(HWND window, UINT width, UINT height);
		bool update();
		bool isVisible();
		bool draw(ID2D1Bitmap1* target, D2D1_POINT_2F offset = {});

	public:
		ImmersiveTitleBarController();
		ImmersiveTitleBarController(ImmersiveTitleBarController const&) = delete;
		ImmersiveTitleBarController(ImmersiveTitleBarController&& other) noexcept;
		~ImmersiveTitleBarController();
	};

}
