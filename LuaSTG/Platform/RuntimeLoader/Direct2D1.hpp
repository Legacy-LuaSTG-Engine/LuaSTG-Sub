#pragma once

namespace Platform::RuntimeLoader
{
	class Direct2D1
	{
	private:
		// FUCK Microsoft
		typedef HRESULT(WINAPI* PFN_D2D1CreateFactory)(
			D2D1_FACTORY_TYPE factoryType,
			REFIID riid,
			CONST D2D1_FACTORY_OPTIONS* pFactoryOptions,
			void** ppIFactory);
		typedef HRESULT(WINAPI* PFN_D2D1CreateDevice)(
			IDXGIDevice* dxgiDevice,
			CONST D2D1_CREATION_PROPERTIES* creationProperties,
			ID2D1Device** d2dDevice);
	private:
		HMODULE dll_d2d1{};
		PFN_D2D1CreateFactory api_D2D1CreateFactory{};
		PFN_D2D1CreateDevice  api_D2D1CreateDevice{};
	public:
		HRESULT CreateFactory(
			D2D1_FACTORY_TYPE factoryType,
			REFIID riid,
			void** ppIFactory);
		HRESULT CreateDevice(
			IDXGIDevice* dxgiDevice,
			BOOL multiThread,
			ID2D1Device** d2dDevice);
		HRESULT CreateDevice(
			IDXGIDevice* dxgiDevice,
			BOOL multiThread,
			ID2D1Device** d2dDevice,
			ID2D1DeviceContext** d2dDeviceContext);
	public:
		Direct2D1();
		~Direct2D1();
	};
}
