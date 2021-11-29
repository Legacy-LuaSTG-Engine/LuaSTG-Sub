#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <wrl.h>

#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wincodec.h>

#include <DirectXMath.h>

inline bool _hr_succeeded(HRESULT hr) { return SUCCEEDED(hr); }
inline bool _hr_failed(HRESULT hr) { return FAILED(hr); }
inline std::wstring _to_wide(const std::string& str)
{
	const int need = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
	if (need > 0)
	{
		std::wstring wstr(need, L'\0');
		const int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), wstr.data(), need);
		if (size > 0)
		{
			return std::move(wstr);
		}
	}
	return L"";
}
