#pragma once
#include <string_view>

namespace platform
{
#ifdef _HRESULT_DEFINED
	using HResult = HRESULT;
	using WChar = WCHAR;
#else
	using HResult = long;
	using WChar = wchar_t;
#endif

	class HResultChecker
	{
	private:
		HResult hr_{};
	#ifdef _DEBUG
		int const line_{};
		WChar const* const source_{};
	#endif
	public:
		HResultChecker& operator=(HResult const hr);
		operator HResult() const noexcept;
	public:
		explicit HResultChecker(WChar const* const source, int const line, HResult const hr) noexcept;
		explicit HResultChecker(WChar const* const source, int const line) noexcept;
		explicit HResultChecker(HResult const hr) noexcept;
		explicit HResultChecker() noexcept;
	public:
		using PrintCallback = void (*)(std::string_view const);
		static void SetPrintCallback(PrintCallback pfn = nullptr) noexcept;
	};

	struct HResultToBool
	{
		HResult hr{};
		HResultToBool& operator=(HResult v) noexcept;
		operator bool() const noexcept;
		explicit HResultToBool() noexcept;
		explicit HResultToBool(HResult const hr_) noexcept;
	};
}

#ifdef _DEBUG
#define gHR platform::HResultChecker(__FILEW__, __LINE__)
#else
#define gHR platform::HResultChecker()
#endif

#define bHR platform::HResultToBool()
