#include <cassert>
#include <cmath>
#include <string>

#include "win32/base.hpp"
#include "win32/win32.hpp"

#include "CleanWindows.hpp"
#include <windowsx.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <dwmapi.h>

#include "win32/abi.hpp"

#include "WindowsVersion.hpp"
#include "WindowTheme.hpp"
#include "ImmersiveTitleBarController.hpp"
#include "RuntimeLoader/DirectWrite.hpp"
#include "RuntimeLoader/DesktopWindowManager.hpp"

namespace platform::windows {

	struct ColorSchema {
		D2D_COLOR_F const title_bar_background;
		D2D_COLOR_F const title_text;
		D2D_COLOR_F const button_rest_background;
		D2D_COLOR_F const button_rest_text;
		D2D_COLOR_F const button_disabled_background;
		D2D_COLOR_F const button_disabled_text;
		D2D_COLOR_F const button_hover_background;
		D2D_COLOR_F const button_hover_text;
		D2D_COLOR_F const button_pressed_background;
		D2D_COLOR_F const button_pressed_text;
		D2D_COLOR_F const close_button_hover_background;
		D2D_COLOR_F const close_button_hover_text;
		D2D_COLOR_F const close_button_pressed_background;
		D2D_COLOR_F const close_button_pressed_text;
		D2D_COLOR_F const stroke;
	};

	struct ColorSchema const light {
		.title_bar_background = D2D1::ColorF(0xf3f3f3),
			.title_text = D2D1::ColorF(D2D1::ColorF::Black, 0.8956f),
			.button_rest_background = D2D1::ColorF(D2D1::ColorF::Black, 0.0f), // transparent
			.button_rest_text = D2D1::ColorF(D2D1::ColorF::Black, 0.8956f),
			.button_disabled_background = D2D1::ColorF(D2D1::ColorF::Black, 0.0f), // transparent
			.button_disabled_text = D2D1::ColorF(D2D1::ColorF::Black, 0.3614f),
			.button_hover_background = D2D1::ColorF(D2D1::ColorF::Black, 0.0373f),
			.button_hover_text = D2D1::ColorF(D2D1::ColorF::Black, 0.8956f),
			.button_pressed_background = D2D1::ColorF(D2D1::ColorF::Black, 0.0241f),
			.button_pressed_text = D2D1::ColorF(D2D1::ColorF::Black, 0.6063f),
			.close_button_hover_background = D2D1::ColorF(0xC42B1C),
			.close_button_hover_text = D2D1::ColorF(D2D1::ColorF::White),
			.close_button_pressed_background = D2D1::ColorF(0xC42B1C, 0.9f),
			.close_button_pressed_text = D2D1::ColorF(D2D1::ColorF::White, 0.7f),
			.stroke = D2D1::ColorF(0x757575, 0.4f),
	};
	static ColorSchema const dark{
		.title_bar_background = D2D1::ColorF(0x202020),
		.title_text = D2D1::ColorF(D2D1::ColorF::White),
		.button_rest_background = D2D1::ColorF(D2D1::ColorF::Black, 0.0f), // transparent
		.button_rest_text = D2D1::ColorF(D2D1::ColorF::White),
		.button_disabled_background = D2D1::ColorF(D2D1::ColorF::Black, 0.0f), // transparent
		.button_disabled_text = D2D1::ColorF(D2D1::ColorF::White, 0.3628f),
		.button_hover_background = D2D1::ColorF(D2D1::ColorF::White, 0.0605f),
		.button_hover_text = D2D1::ColorF(D2D1::ColorF::White),
		.button_pressed_background = D2D1::ColorF(D2D1::ColorF::White, 0.0419f),
		.button_pressed_text = D2D1::ColorF(D2D1::ColorF::White, 0.786f),
		.close_button_hover_background = D2D1::ColorF(0xC42B1C),
		.close_button_hover_text = D2D1::ColorF(D2D1::ColorF::White),
		.close_button_pressed_background = D2D1::ColorF(0xC42B1C, 0.9f),
		.close_button_pressed_text = D2D1::ColorF(D2D1::ColorF::White, 0.7f),
		.stroke = D2D1::ColorF(0x757575, 0.4f),
	};

	struct IconSchema {
		wchar_t const font[32];
		wchar_t const chrome_minimize[2];
		wchar_t const chrome_maximize[2];
		wchar_t const chrome_restore[2];
		wchar_t const chrome_close[2];
		wchar_t const chrome_fullScreen[2];
		wchar_t const chrome_back_to_window[2];
		// TODO: High Contrast Mode
	};

	static IconSchema const icon_windows10{
		.font{L"Segoe MDL2 Assets"},
		.chrome_minimize{L"\uE921"},
		.chrome_maximize{L"\uE922"},
		.chrome_restore{L"\uE923"},
		.chrome_close{L"\uE8BB"},
		.chrome_fullScreen{L"\uE92D"},
		.chrome_back_to_window{L"\uE92C"},
	};
	static IconSchema const icon_windows11{
		.font{L"Segoe Fluent Icons"},
		.chrome_minimize{L"\uE921"},
		.chrome_maximize{L"\uE922"},
		.chrome_restore{L"\uE923"},
		.chrome_close{L"\uE8BB"},
		.chrome_fullScreen{L"\uE92D"},
		.chrome_back_to_window{L"\uE92C"},
	};

	constexpr float const title_bar_height{ 32.0f };
	constexpr float const title_left_padding{ 16.0f };
	constexpr float const title_bottom_padding{ 2.0f };
	constexpr float const title_font_size{ 12.0f };
	constexpr float const icon_font_size{ 10.0f };
	constexpr float const button_width{ 46.0f };
	constexpr float const button_icon_width{ 16.0f };
	constexpr float const button_icon_height{ 16.0f };
	constexpr float const button_icon_padding_left{ 15.0f };
	constexpr float const button_icon_padding_top{ 8.0f };
	constexpr float const stroke_width{ 1.0f };

	class ImmersiveTitleBarController::Impl {
	private:
		Platform::RuntimeLoader::DirectWrite dwrite;
		Platform::RuntimeLoader::DesktopWindowManager dwm;
		UINT win32_window_width{};
		UINT win32_window_height{};
		UINT win32_window_dpi{ win32::getUserDefaultScreenDpi() };
		win32::com_ptr<ID2D1DeviceContext> d2d1_device_context;
		win32::com_ptr<IDWriteFactory> dwrite_factory;
		win32::com_ptr<IDWriteTextFormat> dwrite_text_format_title;
		win32::com_ptr<IDWriteTextFormat> dwrite_text_format_icon;
		std::wstring title_text{ L"Render Window" };
		bool feature_enable{ true };
		bool dark_mode{ false };
		bool system_windows11{ false };
		bool window_minimized{ false };
		bool window_maximized{ false };
		bool enter_title_bar{ false };
		bool pointer_on_minimize_button{ false };
		bool pointer_on_maximize_button{ false };
		bool pointer_on_close_button{ false };
		bool minimize_button_down{ false };
		bool maximize_button_down{ false };
		bool close_button_down{ false };
		bool title_bar_down{ false };

	private:
		float getScaling() const noexcept { return win32::getScalingFromDpi(win32_window_dpi); }
		ColorSchema const& getColorSchema() const noexcept { return dark_mode ? dark : light; }
		IconSchema const& getIconSchema() const noexcept { return system_windows11 ? icon_windows11 : icon_windows10; }
		void trackMouseLeave(HWND win32_window) {
			assert(win32_window);
			TRACKMOUSEEVENT t{};
			t.cbSize = sizeof(t);
			t.dwFlags = TME_CANCEL | TME_LEAVE | TME_NONCLIENT;
			t.hwndTrack = win32_window;
			t.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&t);
			t.dwFlags = TME_LEAVE | TME_NONCLIENT;
			TrackMouseEvent(&t);
		}
		void resetButtons() {
			pointer_on_minimize_button = false;
			pointer_on_maximize_button = false;
			pointer_on_close_button = false;
			minimize_button_down = false;
			maximize_button_down = false;
			close_button_down = false;
			title_bar_down = false;
		}
		void exitTitleBar() {
			enter_title_bar = false;
			resetButtons();
		}
		void initializeWindowDpi(HWND win32_window) {
			win32_window_dpi = win32::getDpiForWindow(win32_window);
		}
		void drawTitleBar() {
			auto const scaling = getScaling();
			auto const& color_schema = getColorSchema();

			win32::com_ptr<ID2D1SolidColorBrush> title_bar_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.title_bar_background, title_bar_background_color.put());

			d2d1_device_context->FillRectangle(
				D2D1::RectF(0.0f, 0.0f, (float)win32_window_width, title_bar_height * scaling),
				title_bar_background_color.get()
			);

			win32::com_ptr<ID2D1SolidColorBrush> title_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.title_text, title_text_color.put());

			auto const title_text_width = (float)win32_window_width - (3.0f * button_width + 2.0f * title_left_padding) * scaling;
			if (title_text_width > 0) {
				win32::com_ptr<IDWriteTextLayout> title_text_layout;
				dwrite_factory->CreateTextLayout(
					title_text.c_str(), static_cast<UINT>(title_text.length()),
					dwrite_text_format_title.get(),
					title_text_width,
					(title_bar_height - title_bottom_padding) * scaling,
					title_text_layout.put()
				);
				DWRITE_TEXT_RANGE range{
					.startPosition = 0,
					.length = static_cast<UINT>(title_text.length()),
				};
				title_text_layout->SetFontSize(title_font_size * scaling, range);
				title_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				title_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				title_text_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

				DWRITE_TEXT_METRICS metrics{};
				title_text_layout->GetMetrics(&metrics);
				if (metrics.width > title_text_width) {
					win32::com_ptr<IDWriteTextLayout> title_text_more_layout;
					dwrite_factory->CreateTextLayout(
						L"...", 3,
						dwrite_text_format_title.get(),
						100.0f,
						(title_bar_height - title_bottom_padding) * scaling,
						title_text_more_layout.put()
					);
					title_text_more_layout->SetFontSize(title_font_size * scaling, range);
					title_text_more_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
					title_text_more_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

					d2d1_device_context->DrawTextLayout(
						D2D1::Point2F(title_left_padding * scaling + title_text_width, 0.0f),
						title_text_more_layout.get(),
						title_text_color.get()
					);

					DWRITE_TRIMMING trimming{};
					trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
					title_text_layout->SetTrimming(&trimming, NULL);
				}

				d2d1_device_context->DrawTextLayout(
					D2D1::Point2F(title_left_padding * scaling, 0.0f),
					title_text_layout.get(),
					title_text_color.get()
				);
			}
		}
		void drawButtons() {
			auto const scaling = getScaling();
			auto const& color_schema = getColorSchema();
			auto const& icon_schema = getIconSchema();

			float const height = title_bar_height * scaling;
			float const close_button_right = (float)win32_window_width;
			float const close_button_left = close_button_right - button_width * scaling;
			float const max_button_right = close_button_left;
			float const max_button_left = max_button_right - button_width * scaling;
			float const min_button_right = max_button_left;
			float const min_button_left = min_button_right - button_width * scaling;

			win32::com_ptr<ID2D1SolidColorBrush> rest_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_rest_background, rest_background_color.put());
			win32::com_ptr<ID2D1SolidColorBrush> rest_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_rest_text, rest_text_color.put());

			win32::com_ptr<ID2D1SolidColorBrush> disabled_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_disabled_background, disabled_background_color.put());
			win32::com_ptr<ID2D1SolidColorBrush> disabled_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_disabled_text, disabled_text_color.put());

			win32::com_ptr<ID2D1SolidColorBrush> hover_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_hover_background, hover_background_color.put());
			win32::com_ptr<ID2D1SolidColorBrush> hover_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_hover_text, hover_text_color.put());

			win32::com_ptr<ID2D1SolidColorBrush> press_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_pressed_background, press_background_color.put());
			win32::com_ptr<ID2D1SolidColorBrush> press_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.button_pressed_text, press_text_color.put());

			auto getMinimizeButtonBackgroundBrush = [&]() -> ID2D1SolidColorBrush* {
				if (minimize_button_down) {
					return press_background_color.get();
				}
				return (pointer_on_minimize_button ? hover_background_color : rest_background_color).get();
			};
			auto getMinimizeButtonTextBrush = [&]() -> ID2D1SolidColorBrush* {
				if (minimize_button_down) {
					return press_text_color.get();
				}
				return (pointer_on_minimize_button ? hover_text_color : rest_text_color).get();
			};

			auto getMaximizeButtonBackgroundBrush = [&]() -> ID2D1SolidColorBrush* {
				if (maximize_button_down) {
					return press_background_color.get();
				}
				return (pointer_on_maximize_button ? hover_background_color : rest_background_color).get();
			};
			auto getMaximizeButtonTextBrush = [&]() -> ID2D1SolidColorBrush* {
				if (maximize_button_down) {
					return press_text_color.get();
				}
				return (pointer_on_maximize_button ? hover_text_color : rest_text_color).get();
			};

			win32::com_ptr<ID2D1SolidColorBrush> close_hover_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.close_button_hover_background, close_hover_background_color.put());
			win32::com_ptr<ID2D1SolidColorBrush> close_hover_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.close_button_hover_text, close_hover_text_color.put());

			win32::com_ptr<ID2D1SolidColorBrush> close_press_background_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.close_button_pressed_background, close_press_background_color.put());
			win32::com_ptr<ID2D1SolidColorBrush> close_press_text_color;
			d2d1_device_context->CreateSolidColorBrush(color_schema.close_button_pressed_text, close_press_text_color.put());

			auto getCloseButtonBackgroundBrush = [&]() -> ID2D1SolidColorBrush* {
				if (close_button_down) {
					return close_press_background_color.get();
				}
				return (pointer_on_close_button ? close_hover_background_color : rest_background_color).get();
			};
			auto getCloseButtonTextBrush = [&]() -> ID2D1SolidColorBrush* {
				if (close_button_down) {
					return close_press_text_color.get();
				}
				return (pointer_on_close_button ? close_hover_text_color : rest_text_color).get();
			};

			if (pointer_on_minimize_button || minimize_button_down) {
				d2d1_device_context->FillRectangle(
					D2D1::RectF(min_button_left, 0.0f, min_button_right, height),
					getMinimizeButtonBackgroundBrush()
				);
			}
			if (pointer_on_maximize_button || maximize_button_down) {
				d2d1_device_context->FillRectangle(
					D2D1::RectF(max_button_left, 0.0f, max_button_right, height),
					getMaximizeButtonBackgroundBrush()
				);
			}
			if (pointer_on_close_button || close_button_down) {
				d2d1_device_context->FillRectangle(
					D2D1::RectF(close_button_left, 0.0f, close_button_right, height),
					getCloseButtonBackgroundBrush()
				);
			}

			DWRITE_TEXT_RANGE const text_range{
				.startPosition = 0,
				.length = 1,
			};

			win32::com_ptr<IDWriteTextLayout> minimize_icon_text_layout;
			dwrite_factory->CreateTextLayout(
				icon_schema.chrome_minimize, 1,
				dwrite_text_format_icon.get(),
				button_icon_width * scaling, button_icon_height * scaling,
				minimize_icon_text_layout.put()
			);
			minimize_icon_text_layout->SetFontSize(icon_font_size * scaling, text_range);
			minimize_icon_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			minimize_icon_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			d2d1_device_context->DrawTextLayout(
				D2D1::Point2F(min_button_left + button_icon_padding_left * scaling, button_icon_padding_top * scaling),
				minimize_icon_text_layout.get(),
				getMinimizeButtonTextBrush()
			);

			win32::com_ptr<IDWriteTextLayout> maximize_icon_text_layout;
			dwrite_factory->CreateTextLayout(
				window_maximized ? icon_schema.chrome_restore : icon_schema.chrome_maximize, 1,
				dwrite_text_format_icon.get(),
				button_icon_width * scaling, button_icon_height * scaling,
				maximize_icon_text_layout.put()
			);
			maximize_icon_text_layout->SetFontSize(icon_font_size * scaling, text_range);
			maximize_icon_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			maximize_icon_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			d2d1_device_context->DrawTextLayout(
				D2D1::Point2F(max_button_left + button_icon_padding_left * scaling, button_icon_padding_top * scaling),
				maximize_icon_text_layout.get(),
				getMaximizeButtonTextBrush()
			);

			win32::com_ptr<IDWriteTextLayout> close_icon_text_layout;
			dwrite_factory->CreateTextLayout(
				icon_schema.chrome_close, 1,
				dwrite_text_format_icon.get(),
				button_icon_width * scaling, button_icon_height * scaling,
				close_icon_text_layout.put()
			);
			close_icon_text_layout->SetFontSize(icon_font_size * scaling, text_range);
			close_icon_text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			close_icon_text_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			d2d1_device_context->DrawTextLayout(
				D2D1::Point2F(close_button_left + button_icon_padding_left * scaling, button_icon_padding_top * scaling),
				close_icon_text_layout.get(),
				getCloseButtonTextBrush()
			);
		}
		UINT getFrameBorderThickness(HWND win32_window) {
			if (!system_windows11) {
				return 0;
			}
			HRESULT hr{};
			UINT value{};
			hr = dwm.GetWindowAttribute(win32_window, DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
			if (SUCCEEDED(hr)) {
				return value;
			}
			return 0;
		}

	public:
		void setEnable(bool enable) { feature_enable = enable; }
		BOOL adjustWindowRectExForDpi(RECT* lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) {
			if (!feature_enable || !system_windows11) {
				return win32::adjustWindowRectExForDpi(lpRect, dwStyle, bMenu, dwExStyle, dpi);
			}
			auto const old_top = lpRect->top;
			auto const result = win32::adjustWindowRectExForDpi(lpRect, dwStyle, bMenu, dwExStyle, dpi);
			if (system_windows11) {
				auto const outline = win32::scaleByDpi(1, dpi); // MAGIC NUMBER: outline width
				lpRect->top = old_top - outline; // MAGIC NUMBER: outline width
			}
			else {
				lpRect->top = old_top;
			}
			return result;
		}
		HandleWindowMessageResult handleWindowMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2) {
			if (!feature_enable || !system_windows11) {
				return { false, 0 };
			}
			switch (message) {
			case WM_CREATE:
				dark_mode = Platform::WindowTheme::ShouldApplicationEnableDarkMode();
				//updateBackDrop(window);
				break;
			case WM_SIZE:
				switch (arg1) {
				case SIZE_MINIMIZED:
					window_minimized = true;
					window_maximized = false;
					break;
				case SIZE_MAXIMIZED:
					window_minimized = false;
					window_maximized = true;
					break;
				case SIZE_MAXSHOW:
				case SIZE_MAXHIDE:
					break; // ignore
				default:
				case SIZE_RESTORED:
					window_minimized = false;
					window_maximized = false;
					break;
				}
				win32_window_width = LOWORD(arg2);
				win32_window_height = HIWORD(arg2);
				break;
			case WM_DPICHANGED:
				assert(LOWORD(arg1) == HIWORD(arg1));
				win32_window_dpi = LOWORD(arg1);
				break;
			case WM_SETTINGCHANGE:
				dark_mode = Platform::WindowTheme::ShouldApplicationEnableDarkMode();
				//updateBackDrop(window);
				break;
			case WM_NCHITTEST:
				if (enter_title_bar) {
					initializeWindowDpi(window);
					auto const test = DefWindowProcW(window, message, arg1, arg2);
					if (test != HTCLIENT) {
						return { true, test };
					}
					auto const screenX = GET_X_LPARAM(arg2);
					auto const screenY = GET_Y_LPARAM(arg2);
					POINT pt{ screenX, screenY };
					MapWindowPoints(HWND_DESKTOP, window, &pt, 1);
					int const i_title_height = static_cast<int>(title_bar_height);
					if (pt.y < win32::scaleByDpi(i_title_height, win32_window_dpi)) {
						int const i_button_width = static_cast<int>(button_width);
						if (pt.x < (static_cast<int>(win32_window_width) - win32::scaleByDpi(i_button_width * 3, win32_window_dpi))) {
							return { true, HTCAPTION };
						}
						if (pt.x < (static_cast<int>(win32_window_width) - win32::scaleByDpi(i_button_width * 2, win32_window_dpi))) {
							return { true, HTMINBUTTON };
						}
						if (pt.x < (static_cast<int>(win32_window_width) - win32::scaleByDpi(i_button_width * 1, win32_window_dpi))) {
							return { true, HTMAXBUTTON };
						}
						return { true, HTCLOSE };
					}
					else {
						return { true, test };
					}
				}
				break;
				//case WM_NCPAINT:
				//    if constexpr (true) {
				//        HDC hdc = GetDCEx(window, (HRGN)arg1, DCX_WINDOW | DCX_INTERSECTRGN);
				//        RECT rc{};
				//        GetWindowRect(window, &rc);
				//        HBRUSH b = CreateSolidBrush(RGB(255, 0, 0));
				//        FillRect(hdc, &rc, b);
				//        DeleteObject(b);
				//        ReleaseDC(window, hdc);
				//        return { true, 0 };
				//    }
				//    break;
			case WM_NCCALCSIZE:
				if constexpr (true) {
					initializeWindowDpi(window);
					WINDOWPLACEMENT placement{};
					GetWindowPlacement(window, &placement);
					LONG old_top{};
					if (!arg1 /* == FALSE */) {
						auto rect = reinterpret_cast<RECT*>(arg2);
						old_top = rect->top;
					}
					else {
						auto data = reinterpret_cast<NCCALCSIZE_PARAMS*>(arg2);
						old_top = data->rgrc[0].top;
					}
					HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
					assert(monitor);
					MONITORINFO monitor_info{ .cbSize = sizeof(MONITORINFO) };
					[[maybe_unused]] BOOL const br = GetMonitorInfoW(monitor, &monitor_info);
					assert(br);
					[[maybe_unused]] auto const result = DefWindowProcW(window, message, arg1, arg2);
					auto const outline = getFrameBorderThickness(window);
					//auto const h_padded_border = win32::getSystemMetricsForDpi(SM_CXPADDEDBORDER, win32_window_dpi);
					//auto const h_border = win32::getSystemMetricsForDpi(SM_CYBORDER, win32_window_dpi);
					//auto const h_edge = win32::getSystemMetricsForDpi(SM_CYEDGE, win32_window_dpi);
					//auto const h_size_frame = win32::getSystemMetricsForDpi(SM_CYSIZEFRAME, win32_window_dpi);
					//auto const h_fixed_frame = win32::getSystemMetricsForDpi(SM_CYFIXEDFRAME, win32_window_dpi);
					//auto const h_caption = win32::getSystemMetricsForDpi(SM_CYCAPTION, win32_window_dpi);
					if (!arg1 /* == FALSE */) {
						auto rect = reinterpret_cast<RECT*>(arg2);
						if (placement.showCmd == SW_MAXIMIZE) {
							//rect->top = old_top + h_padded_border + h_size_frame;
							rect->top = monitor_info.rcWork.top;
						}
						else if (system_windows11) {
							rect->top = old_top + outline;
						}
						else {
							rect->top = old_top;
						}
					}
					else {
						auto data = reinterpret_cast<NCCALCSIZE_PARAMS*>(arg2);
						if (placement.showCmd == SW_MAXIMIZE) {
							//data->rgrc[0].top = old_top + h_padded_border + h_size_frame;
							data->rgrc[0].top = monitor_info.rcWork.top;
						}
						else if (system_windows11) {
							data->rgrc[0].top = old_top + outline;
						}
						else {
							data->rgrc[0].top = old_top;
						}
					}
					return { true, WVR_REDRAW };
				}
				break;
			case WM_MOUSEMOVE:
				if constexpr (true) {
					auto const scaling = getScaling();
					int const x = GET_X_LPARAM(arg2);
					int const y = GET_Y_LPARAM(arg2);
					if (!enter_title_bar
						&& (x >= (win32_window_width - button_width * scaling * 3) || y < (1.0f * scaling))
						&& y < (title_bar_height * scaling)
						) {
						enter_title_bar = true;
					}
					else if (y >= (title_bar_height * scaling)) {
						exitTitleBar();
					}
				}
				break;
			case WM_NCMOUSEMOVE:
				if (enter_title_bar) {
					trackMouseLeave(window);
					auto const test = SendMessageW(window, WM_NCHITTEST, 0, arg2);
					pointer_on_minimize_button = false;
					pointer_on_maximize_button = false;
					pointer_on_close_button = false;
					switch (test) {
					case HTMINBUTTON:
						pointer_on_minimize_button = true;
						//minimize_button_down = false;
						maximize_button_down = false;
						close_button_down = false;
						return { true, 0 };
					case HTMAXBUTTON:
						pointer_on_maximize_button = true;
						minimize_button_down = false;
						//maximize_button_down = false;
						close_button_down = false;
						return { true, 0 };
					case HTCLOSE:
						pointer_on_close_button = true;
						minimize_button_down = false;
						maximize_button_down = false;
						//close_button_down = false;
						return { true, 0 };
					default:
						minimize_button_down = false;
						maximize_button_down = false;
						close_button_down = false;
						break;
					}
				}
				break;
			case WM_NCLBUTTONDOWN:
				if (enter_title_bar) {
					trackMouseLeave(window);
					auto const test = SendMessageW(window, WM_NCHITTEST, 0, arg2);
					switch (test) {
					case HTMINBUTTON:
						minimize_button_down = true;
						return { true, 0 };
					case HTMAXBUTTON:
						maximize_button_down = true;
						return { true, 0 };
					case HTCLOSE:
						close_button_down = true;
						return { true, 0 };
					case HTCAPTION:
						title_bar_down = true;
						break;
					}
				}
				break;
			case WM_NCLBUTTONUP:
				if (enter_title_bar) {
					auto const test = SendMessageW(window, WM_NCHITTEST, 0, arg2);
					switch (test) {
					case HTMINBUTTON:
						if (minimize_button_down) {
							minimize_button_down = false;
							SendMessageW(window, WM_SYSCOMMAND, SC_MINIMIZE, arg2);
							return { true, 0 };
						}
						break;
					case HTMAXBUTTON:
						if (maximize_button_down) {
							maximize_button_down = false;
							SendMessageW(window, WM_SYSCOMMAND, window_maximized ? SC_RESTORE : SC_MAXIMIZE, arg2);
							return { true, 0 };
						}
						break;
					case HTCLOSE:
						if (close_button_down) {
							close_button_down = false;
							SendMessageW(window, WM_SYSCOMMAND, SC_CLOSE, arg2);
							return { true, 0 };
						}
						break;
					case HTCAPTION:
						if (title_bar_down) {
							title_bar_down = false;
						}
						break;
					}
				}
				break;
			case WM_NCMOUSELEAVE:
				if (enter_title_bar) {
					if (!title_bar_down) {
						exitTitleBar();
					}
					return { true, 0 };
				}
				break;
			case WM_EXITSIZEMOVE:
				if (title_bar_down) {
					title_bar_down = false; // some hack, click on caption won't generate WM_NCLBUTTONUP
				}
				break;
			}
			return { false, 0 };
		}
		bool createResources(HWND window, ID2D1DeviceContext* context) {
			if (!system_windows11) {
				return true; // fail, but success
			}
			assert(window);
			assert(context);
			HRESULT hr{};
			initializeWindowDpi(window);
			RECT client{};
			if (!GetClientRect(window, &client)) { // TODO: log error
				return false;
			}
			win32_window_width = client.right - client.left;
			win32_window_height = client.bottom - client.top;
			d2d1_device_context = context;
			hr = dwrite.CreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_PPV_ARGS(dwrite_factory.put()));
			if (FAILED(hr)) {
				win32::check_hresult(hr, "DWriteCreateFactory");
				return false;
			}
			win32::com_ptr<IDWriteFontCollection> system_font_collection;
			hr = dwrite_factory->GetSystemFontCollection(system_font_collection.put(), false);
			if (FAILED(hr)) {
				win32::check_hresult(hr, "IDWriteFactory::GetSystemFontCollection");
				return false;
			}
			hr = dwrite_factory->CreateTextFormat(
				L"微软雅黑",
				system_font_collection.get(),
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				title_font_size,
				L"zh-cn",
				dwrite_text_format_title.put()
			);
			if (FAILED(hr)) {
				win32::check_hresult(hr, "CreateTextFormat(\"微软雅黑\")");
				return false;
			}
			hr = dwrite_factory->CreateTextFormat(
				getIconSchema().font,
				system_font_collection.get(),
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				icon_font_size,
				L"en-us",
				dwrite_text_format_icon.put()
			);
			if (FAILED(hr)) {
				win32::check_hresult(hr, "CreateTextFormat(\"Segoe Fluent Icons\")");
				return false;
			}
			dark_mode = Platform::WindowTheme::ShouldApplicationEnableDarkMode();
			window_minimized = IsMinimized(window) != FALSE;
			window_maximized = IsMaximized(window) != FALSE;
			return true;
		}
		void destroyResources() {
			win32_window_width = 0;
			win32_window_height = 0;
			win32_window_dpi = 0;
			d2d1_device_context.reset();
			dwrite_factory.reset();
			dwrite_text_format_title.reset();
			dwrite_text_format_icon.reset();
			dark_mode = false;
			window_minimized = false;
			window_maximized = false;
			exitTitleBar();
		}
		bool setTitle(std::string const& text) {
			if (!system_windows11) {
				return true; // fail, but success
			}
			title_text = win32::to_wstring(text);
			return true;
		}
		bool setClientSize(HWND window, UINT width, UINT height) {
			assert(window);
			assert(width > 0);
			assert(height > 0);
			auto const style = static_cast<DWORD>(GetWindowLongPtrW(window, GWL_STYLE));
			auto const style_ex = static_cast<DWORD>(GetWindowLongPtrW(window, GWL_EXSTYLE));
			auto const dpi = win32::getDpiForWindow(window);
			RECT rc{ 0, 0, LONG(width), LONG(height) };
			adjustWindowRectExForDpi(&rc, style, false, style_ex, dpi);
			return FALSE != SetWindowPos(
				window, nullptr,
				0, 0,
				rc.right - rc.left,
				rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOMOVE | SWP_FRAMECHANGED
			);
		}
		bool isVisible() const noexcept {
			if (!feature_enable || !system_windows11) {
				return false;
			}
			if (!enter_title_bar || window_minimized) {
				return false;
			}
			return true;
		}
		UINT getHeight() const {
			auto const scaling = getScaling();
			auto const height = title_bar_height * scaling;
			return static_cast<UINT>(std::ceil(height));
		}
		bool draw(ID2D1Bitmap1* target, D2D1_POINT_2F offset) {
			if (!feature_enable || !system_windows11) {
				return true; // fail, but success
			}
			if (!enter_title_bar || window_minimized) {
				return true;
			}
			assert(target);
			assert(d2d1_device_context);
			HRESULT hr{};
			d2d1_device_context->BeginDraw();
			d2d1_device_context->SetTarget(target);
			d2d1_device_context->SetTransform(D2D1::Matrix3x2F::Translation(offset.x, offset.y));
			drawTitleBar();
			drawButtons();
			d2d1_device_context->SetTransform(D2D1::Matrix3x2F::Identity());
			d2d1_device_context->SetTarget(nullptr);
			hr = d2d1_device_context->EndDraw();
			if (FAILED(hr)) {
				win32::check_hresult(hr, "ID2D1RenderTarget::EndDraw failed");
				return false;
			}
			return true;
		}

	public:
		Impl() {
			system_windows11 = Platform::WindowsVersion::Is11();
		}
		~Impl() {
			destroyResources();
		}
	};

}

namespace platform::windows {

	void ImmersiveTitleBarController::setEnable(bool enable) {
		return impl->setEnable(enable);
	}
	BOOL ImmersiveTitleBarController::adjustWindowRectExForDpi(RECT* lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) {
		return impl->adjustWindowRectExForDpi(lpRect, dwStyle, bMenu, dwExStyle, dpi);
	}
	ImmersiveTitleBarController::HandleWindowMessageResult ImmersiveTitleBarController::handleWindowMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2) {
		return impl->handleWindowMessage(window, message, arg1, arg2);
	}
	bool ImmersiveTitleBarController::createResources(HWND window, ID2D1DeviceContext* context) {
		return impl->createResources(window, context);
	}
	void ImmersiveTitleBarController::destroyResources() {
		return impl->destroyResources();
	}
	bool ImmersiveTitleBarController::setTitle(std::string const& text) {
		return impl->setTitle(text);
	}
	bool ImmersiveTitleBarController::setClientSize(HWND window, UINT width, UINT height) {
		return impl->setClientSize(window, width, height);
	}
	bool ImmersiveTitleBarController::update() {
		return true;
	}
	bool ImmersiveTitleBarController::isVisible() {
		return impl->isVisible();
	}
	UINT ImmersiveTitleBarController::getHeight() {
		return impl->getHeight();
	}
	bool ImmersiveTitleBarController::draw(ID2D1Bitmap1* target, D2D1_POINT_2F offset) {
		return impl->draw(target, offset);
	}

	ImmersiveTitleBarController::ImmersiveTitleBarController() : impl(new Impl()) {}
	ImmersiveTitleBarController::ImmersiveTitleBarController(ImmersiveTitleBarController&& other) noexcept : impl(std::exchange(other.impl, nullptr)) {}
	ImmersiveTitleBarController::~ImmersiveTitleBarController() { delete impl; }

}
