#pragma once
#include <Windows.h>

namespace LuaSTGPlus {
	inline bool CheckRuntime() {
		const wchar_t* dlls[] = {
			L"D3D9.dll",
			L"D3DX9_43.dll",
			L"XINPUT1_3.dll",
			L"XINPUT9_1_0.dll",
			L"DInput8.dll",
			L"DSound.dll",
		};
		for (int idx = 0; idx < 6; idx++) {
			HMODULE dll = LoadLibraryW(dlls[idx]);
			if (dll == NULL) {
				return false;
			}
			FreeLibrary(dll);
		}
		return true;
	}
}
