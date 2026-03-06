#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSERVICE
#define NOMCX
#define NOIME
#include <Windows.h>
#include <bcrypt.h> // NTSTATUS
#include <d3dkmthk.h>

namespace Platform::RuntimeLoader
{
	class D3DKMT
	{
	private:
		HMODULE dll_gdi{};
		decltype(D3DKMTOpenAdapterFromLuid)* api_D3DKMTOpenAdapterFromLuid{};
		decltype(D3DKMTOpenAdapterFromGdiDisplayName)* api_D3DKMTOpenAdapterFromGdiDisplayName{};
		decltype(D3DKMTCloseAdapter)* api_D3DKMTCloseAdapter{};
		decltype(D3DKMTCreateDevice)* api_D3DKMTCreateDevice{};
		decltype(D3DKMTDestroyDevice)* api_D3DKMTDestroyDevice{};
		decltype(D3DKMTQueryAdapterInfo)* api_D3DKMTQueryAdapterInfo{};
		decltype(D3DKMTGetMultiPlaneOverlayCaps)* api_D3DKMTGetMultiPlaneOverlayCaps{};
	public:
		D3DKMT();
		~D3DKMT();
	public:
		NTSTATUS OpenAdapterFromLuid(D3DKMT_OPENADAPTERFROMLUID* arg);
		NTSTATUS OpenAdapterFromGdiDisplayName(D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME* arg);
		NTSTATUS CloseAdapter(CONST D3DKMT_CLOSEADAPTER* arg);
		NTSTATUS CreateDevice(D3DKMT_CREATEDEVICE* arg);
		NTSTATUS DestroyDevice(CONST D3DKMT_DESTROYDEVICE* arg);
		NTSTATUS QueryAdapterInfo(D3DKMT_QUERYADAPTERINFO* arg);
		NTSTATUS GetMultiPlaneOverlayCaps(D3DKMT_GET_MULTIPLANE_OVERLAY_CAPS* arg);
	};
}
