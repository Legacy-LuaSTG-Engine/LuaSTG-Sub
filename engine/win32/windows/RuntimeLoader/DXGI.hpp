#pragma once

namespace Platform::RuntimeLoader
{
	class DXGI
	{
	private:
		HMODULE dll_dxgi{};
		decltype(CreateDXGIFactory)* api_CreateDXGIFactory{};
		decltype(CreateDXGIFactory1)* api_CreateDXGIFactory1{};
		decltype(CreateDXGIFactory2)* api_CreateDXGIFactory2{};
	public:
		HRESULT CreateFactory(REFIID riid, void** ppFactory);
	public:
		DXGI();
		~DXGI();
	};
}
