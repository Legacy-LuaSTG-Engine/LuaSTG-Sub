#include "./D3DKMT.hpp"

namespace Platform::RuntimeLoader
{
	D3DKMT::D3DKMT()
	{
		dll_gdi = LoadLibraryW(L"gdi32.dll");
		if (dll_gdi) {
		#define load(x) api_##x = (decltype(api_##x))GetProcAddress(dll_gdi, #x);
			load(D3DKMTOpenAdapterFromLuid);
			load(D3DKMTOpenAdapterFromGdiDisplayName);
			load(D3DKMTCloseAdapter);
			load(D3DKMTCreateDevice);
			load(D3DKMTDestroyDevice);
			load(D3DKMTQueryAdapterInfo);
			load(D3DKMTGetMultiPlaneOverlayCaps);
		#undef load
		}
	}
	D3DKMT::~D3DKMT()
	{
		if (dll_gdi) {
			FreeLibrary(dll_gdi);
		}
		dll_gdi = NULL;
	#define unload(x) api_##x = NULL;
		unload(D3DKMTOpenAdapterFromLuid);
		unload(D3DKMTOpenAdapterFromGdiDisplayName);
		unload(D3DKMTCloseAdapter);
		unload(D3DKMTCreateDevice);
		unload(D3DKMTDestroyDevice);
		unload(D3DKMTQueryAdapterInfo);
		unload(D3DKMTGetMultiPlaneOverlayCaps);
	#undef unload
	}

	NTSTATUS D3DKMT::OpenAdapterFromLuid(D3DKMT_OPENADAPTERFROMLUID* arg)
	{
		if (!api_D3DKMTOpenAdapterFromLuid) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTOpenAdapterFromLuid(arg);
	}
	NTSTATUS D3DKMT::OpenAdapterFromGdiDisplayName(D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME* arg)
	{
		if (!api_D3DKMTOpenAdapterFromGdiDisplayName) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTOpenAdapterFromGdiDisplayName(arg);
	}
	NTSTATUS D3DKMT::CloseAdapter(CONST D3DKMT_CLOSEADAPTER* arg)
	{
		if (!api_D3DKMTCloseAdapter) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTCloseAdapter(arg);
	}
	NTSTATUS D3DKMT::CreateDevice(D3DKMT_CREATEDEVICE* arg)
	{
		if (!api_D3DKMTCreateDevice) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTCreateDevice(arg);
	}
	NTSTATUS D3DKMT::DestroyDevice(CONST D3DKMT_DESTROYDEVICE* arg)
	{
		if (!api_D3DKMTDestroyDevice) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTDestroyDevice(arg);
	}
	NTSTATUS D3DKMT::QueryAdapterInfo(D3DKMT_QUERYADAPTERINFO* arg)
	{
		if (!api_D3DKMTQueryAdapterInfo) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTQueryAdapterInfo(arg);
	}
	NTSTATUS D3DKMT::GetMultiPlaneOverlayCaps(D3DKMT_GET_MULTIPLANE_OVERLAY_CAPS* arg)
	{
		if (!api_D3DKMTGetMultiPlaneOverlayCaps) {
			return STATUS_DLL_NOT_FOUND;
		}
		return api_D3DKMTGetMultiPlaneOverlayCaps(arg);
	}
}
