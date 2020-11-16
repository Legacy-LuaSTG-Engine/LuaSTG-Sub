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
	
	bool makeApplicationRoamingAppDataDirectory(const std::wstring& company, const std::wstring& product, std::wstring& out)
	{
		out.clear();
		if (!getRoamingAppDataDirectory(out))
		{
			out.clear();
			return false;
		}
		BOOL ret = FALSE;
		out.push_back(L'\\');
		out.append(company);
		ret = CreateDirectoryW(out.c_str(), NULL);
		if ((ret == FALSE) && (GetLastError() != ERROR_ALREADY_EXISTS))
		{
			out.clear();
			return false;
		}
		out.push_back(L'\\');
		out.append(product);
		ret = CreateDirectoryW(out.c_str(), NULL);
		if ((ret == FALSE) && (GetLastError() != ERROR_ALREADY_EXISTS))
		{
			out.clear();
			return false;
		}
		return true;
	}
}
