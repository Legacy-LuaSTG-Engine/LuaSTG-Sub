#include "FileSystemWindows.hpp"
#include "utf8.hpp"
#include <memory_resource>
#include <windows.h>
#include <wrl/wrappers/corewrappers.h>

using std::string_view_literals::operator ""sv;

namespace {
	constexpr auto unc_prefix{ LR"(\\?\)"sv };

	bool isSeparator(wchar_t const c) { return c == L'/' || c == L'\\'; }
	bool isPathEquals(std::pmr::wstring const& a, std::pmr::wstring const& b) {
		std::wstring_view va(a);
		std::wstring_view vb(b);
		if (va.starts_with(unc_prefix)) {
			va = va.substr(unc_prefix.size());
		}
		if (vb.starts_with(unc_prefix)) {
			vb = vb.substr(unc_prefix.size());
		}
		if (va.size() != vb.size()) {
			return false;
		}
		for (size_t i = 0; i < va.size(); i += 1) {
			if ((isSeparator(va[i]) && !isSeparator(vb[i])) || (!isSeparator(va[i]) && isSeparator(vb[i]))) {
				return false;
			}
			if (va[i] != vb[i]) {
				return false;
			}
		}
		return true;
	}
}

namespace win32 {
	bool isFilePathCaseCorrect(std::string_view const& path, std::string& correct) {
		std::wstring temp;
		auto const result = isFilePathCaseCorrect(utf8::to_wstring(path), temp);
		correct.assign(utf8::to_string(temp));
		return result;
	}

	bool isFilePathCaseCorrect(std::string const& path, std::string& correct) {
		return isFilePathCaseCorrect(std::string_view(path), correct);
	}

	bool isFilePathCaseCorrect(std::wstring_view const& path, std::wstring& correct) {
		if (path.empty()) {
			return false;
		}

		uint8_t buf[4096]{};
		std::pmr::monotonic_buffer_resource res(buf, sizeof(buf), std::pmr::get_default_resource());
		std::pmr::wstring str(&res);
		str.reserve(unc_prefix.size() + path.size());
		str.append(unc_prefix);
		str.append(path);

		DWORD const full_path_size = GetFullPathNameW(str.c_str(), 0, nullptr, nullptr);
		if (full_path_size == 0) {
			return false;
		}

		std::pmr::wstring full_path(&res);
		full_path.resize(full_path_size);
		DWORD const full_path_length = GetFullPathNameW(str.c_str(), full_path_size, full_path.data(), nullptr);
		if (full_path_length == 0) {
			return false;
		}
		full_path.resize(full_path_length);

		Microsoft::WRL::Wrappers::FileHandle file;
		file.Attach(CreateFileW(full_path.data(), FILE_GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
		if (!file.IsValid()) {
			return false;
		}

		DWORD const final_path_size = GetFinalPathNameByHandleW(file.Get(), nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if (final_path_size == 0) {
			return false;
		}

		std::pmr::wstring final_path(&res);
		final_path.resize(final_path_size);
		DWORD const final_path_length = GetFinalPathNameByHandleW(file.Get(), final_path.data(), final_path_size, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if (final_path_length == 0) {
			return false;
		}
		final_path.resize(final_path_length);

		file.Close();
		correct.assign(final_path);

		return isPathEquals(full_path, final_path);
	}

	bool isFilePathCaseCorrect(std::wstring const& path, std::wstring& correct) {
		return isFilePathCaseCorrect(std::wstring_view(path), correct);
	}
}
