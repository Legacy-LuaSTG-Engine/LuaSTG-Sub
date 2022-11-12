#pragma once

namespace RuntimeLoader
{
	struct DirectComposition
	{
		HMODULE dll_dcomp{ NULL };
		decltype(DCompositionCreateDevice)* api_DCompositionCreateDevice{ NULL };
		decltype(DCompositionCreateDevice2)* api_DCompositionCreateDevice2{ NULL };
		decltype(DCompositionCreateDevice3)* api_DCompositionCreateDevice3{ NULL };

		HRESULT CreateDevice(IUnknown* renderingDevice, REFIID iid, void** dcompositionDevice)
		{
			if (api_DCompositionCreateDevice3)
			{
				return api_DCompositionCreateDevice3(renderingDevice, iid, dcompositionDevice);
			}
			else if (api_DCompositionCreateDevice2)
			{
				return api_DCompositionCreateDevice2(renderingDevice, iid, dcompositionDevice);
			}
			else if (api_DCompositionCreateDevice)
			{
				HRESULT hr = S_OK;
				IDXGIDevice* dxgiDevice = NULL;
				hr = renderingDevice->QueryInterface(&dxgiDevice);
				if (FAILED(hr)) return hr;
				hr = api_DCompositionCreateDevice(dxgiDevice, iid, dcompositionDevice);
				dxgiDevice->Release();
				return hr;
			}
			else
			{
				return E_NOTIMPL;
			}
		}

		DirectComposition()
		{
			dll_dcomp = LoadLibraryExW(L"dcomp.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
			if (dll_dcomp)
			{
				api_DCompositionCreateDevice = (decltype(api_DCompositionCreateDevice))
					GetProcAddress(dll_dcomp, "DCompositionCreateDevice");
				api_DCompositionCreateDevice2 = (decltype(api_DCompositionCreateDevice2))
					GetProcAddress(dll_dcomp, "DCompositionCreateDevice2");
				api_DCompositionCreateDevice3 = (decltype(api_DCompositionCreateDevice3))
					GetProcAddress(dll_dcomp, "DCompositionCreateDevice3");
			}
		}
		~DirectComposition()
		{
			if (dll_dcomp)
			{
				FreeLibrary(dll_dcomp);
			}
			dll_dcomp = NULL;
			api_DCompositionCreateDevice = NULL;
			api_DCompositionCreateDevice2 = NULL;
			api_DCompositionCreateDevice3 = NULL;
		}
	};
}

using DCompositionLoader = RuntimeLoader::DirectComposition;
