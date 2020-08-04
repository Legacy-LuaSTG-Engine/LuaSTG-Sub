#include "E2DCodePage.hpp"
#include <stdexcept>

namespace Eyes2D {
	namespace String {
		std::wstring EYESDLLAPI MultiToWide(int codepage, const std::string& multi) {
			std::wstring wide;
			if (multi.empty()) {
				return wide;
			}
			constexpr DWORD kFlags = MB_ERR_INVALID_CHARS;
			constexpr size_t lmtmax = static_cast<size_t>((std::numeric_limits<int>::max)());// Add "()" to kill "max" macro
			if (multi.length() > lmtmax) {
				throw std::overflow_error("Input string too long: size_t-length doesn't fit into int.");
			}
			const int multiLength = static_cast<int>(multi.length());
			const int wideLength = ::MultiByteToWideChar(
				codepage,
				kFlags,
				multi.data(),
				multiLength,
				nullptr,
				0
			);
			if (wideLength == 0) {
				const DWORD error = ::GetLastError();
				throw E2DException(0, error, L"Eyes2D::String::MultiToWide", L"Cannot get result wstring length when convert from multibyte to widechar (MultiByteToWideChar failed).");
			}
			wide.resize(wideLength);
			int result = ::MultiByteToWideChar(
				codepage,
				kFlags,
				multi.data(),
				multiLength,
				&wide[0],
				wideLength
			);
			if (result == 0) {
				const DWORD error = ::GetLastError();
				throw E2DException(0, error, L"Eyes2D::String::MultiToWide", L"Cannot convert from multibyte to widechar (MultiByteToWideChar failed).");
			}
			return std::move(wide);
		}

		std::string EYESDLLAPI WideToMulti(int codepage, const std::wstring& wide) {
			std::string multi;
			if (wide.empty()) {
				return multi;
			}
			constexpr size_t lmtmax = static_cast<size_t>((std::numeric_limits<int>::max)());// Add "()" to kill "max" macro
			if (wide.length() > lmtmax) {
				throw std::overflow_error("Input wstring too long: size_t-length doesn't fit into int.");
			}
			const int wideLength = static_cast<int>(wide.length());
			const int multiLength = ::WideCharToMultiByte(
				codepage,
				NULL,
				wide.data(),
				wideLength,
				nullptr,
				0,
				NULL,
				FALSE
			);
			if (multiLength == 0) {
				const DWORD error = ::GetLastError();
				throw E2DException(0, error, L"Eyes2D::String::WideToMulti", L"Cannot get result wstring length when convert from widechar to multibyte (WideCharToMultiByte failed).");
			}
			multi.resize(multiLength);
			int result = ::WideCharToMultiByte(
				codepage,
				NULL,
				wide.data(),
				wideLength,
				&multi[0],
				multiLength,
				NULL,
				FALSE
			);
			if (result == 0) {
				const DWORD error = ::GetLastError();
				throw E2DException(0, error, L"Eyes2D::String::WideToMulti", L"Cannot convert from widechar to multibyte (WideCharToMultiByte failed).");
			}
			return std::move(multi);
		}
	}
}
