#pragma once
#include <string_view>
#include <string>
#include <functional>
#ifndef NDEBUG
#include <source_location>
#endif

namespace win32 {
	void set_logger_writer(std::function<void(std::string_view)> const& writer);

	using hresult = int32_t;

#ifndef NDEBUG
#define SL , std::source_location const& sl = std::source_location::current()
#else
#define SL
#endif

	hresult check_hresult(hresult hr SL);

	hresult check_hresult(hresult hr, std::string_view api SL);

	bool check_hresult_as_boolean(hresult hr SL);

	bool check_hresult_as_boolean(hresult hr, std::string_view api SL);
#undef SL
}
