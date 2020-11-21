#pragma once
#include <Windows.h>

namespace LuaSTGPlus {
	inline bool CheckRuntime() {
		const wchar_t* dlls[] = {
			L"d3dx9_43.dll",
			L"D3DCompiler_43.dll",
		};
		bool flag = true;
		for (int idx = 0; idx < (sizeof(dlls) / sizeof(dlls[0])); idx++) {
			HMODULE dll = LoadLibraryW(dlls[idx]);
			if (dll != NULL) {
				FreeLibrary(dll);
			}
			else
			{
				flag = false;
			}
		}
		return flag;
	}
}
