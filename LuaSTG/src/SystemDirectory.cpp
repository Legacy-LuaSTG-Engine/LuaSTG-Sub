#include "SystemDirectory.hpp"
#include <Windows.h>
#include <Shobjidl.h>
#include <Knownfolders.h>

namespace app {
	bool getDirectory(const KNOWNFOLDERID& id, std::wstring& out) {
		bool ok = false;
		HRESULT hr;
		IKnownFolderManager* manager;
		hr = CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_INPROC_SERVER, IID_IKnownFolderManager, (LPVOID*)&manager);
		if (hr == S_OK) {
			IKnownFolder* folder;
			hr = manager->GetFolder(id, &folder);
			if (hr == S_OK) {
				LPWSTR str = NULL;
				hr = folder->GetPath(0, &str);
				if (hr == S_OK && str != NULL) {
					out = str;
					ok = true;
					CoTaskMemFree(str);
				}
				folder->Release();
			}
			manager->Release();
		}
		return ok;
	}

	bool getLocalAppDataDirectory(std::wstring& out) {
		return getDirectory(FOLDERID_LocalAppData, out);
	}

	bool getRoamingAppDataDirectory(std::wstring& out) {
		return getDirectory(FOLDERID_RoamingAppData, out);
	}
}
