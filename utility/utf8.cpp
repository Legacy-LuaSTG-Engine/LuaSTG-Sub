#include "utf8.hpp"

#include <cassert>

static_assert(CHAR_BIT == 8);
constexpr uint32_t const CP_UTF8 = 65001;
extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
	uint32_t CodePage,
	uint32_t dwFlags,
	wchar_t const* lpWideCharStr,
	int cchWideChar,
	char* lpMultiByteStr,
	int cbMultiByte,
	char const* lpDefaultChar,
	int* lpUsedDefaultChar);
extern "C" __declspec(dllimport) int __stdcall MultiByteToWideChar(
	uint32_t CodePage,
	uint32_t dwFlags,
	char const* lpMultiByteStr,
	int cbMultiByte,
	wchar_t* lpWideCharStr,
	int cchWideChar);

namespace utf8
{
	std::string to_string(std::wstring_view wstr)
	{
		std::string str;
		if (wstr.empty()) {
			return str;
		}
		int const length = WideCharToMultiByte(
			CP_UTF8, 0,
			wstr.data(), static_cast<int>(wstr.length()),
			nullptr, 0,
			nullptr, nullptr);
		if (length <= 0) {
			assert(false);
			return str;
		}
		str.resize(static_cast<size_t>(length));
		int const result = WideCharToMultiByte(
			CP_UTF8, 0,
			wstr.data(), static_cast<int>(wstr.length()),
			str.data(), length,
			nullptr, nullptr);
		if (result != length) {
			assert(false);
			str.clear();
		}
		return str;
	}
	std::wstring to_wstring(std::string_view str)
	{
		std::wstring wstr;
		if (str.empty()) {
			return wstr;
		}
		int const length = MultiByteToWideChar(
			CP_UTF8, 0,
			str.data(), static_cast<int>(str.length()),
			nullptr, 0);
		if (length <= 0) {
			assert(false);
			return wstr;
		}
		wstr.resize(static_cast<size_t>(length));
		int const result = MultiByteToWideChar(
			CP_UTF8, 0,
			str.data(), static_cast<int>(str.length()),
			wstr.data(), length);
		if (result != length) {
			assert(false);
			wstr.clear();
		}
		return wstr;
	}
}
