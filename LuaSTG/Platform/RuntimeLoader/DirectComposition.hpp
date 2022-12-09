#pragma once

namespace Platform::RuntimeLoader
{
	class DirectComposition
	{
	private:
		HMODULE dll_dcomp{};
		decltype(DCompositionCreateDevice)* api_DCompositionCreateDevice{};
		decltype(DCompositionCreateDevice2)* api_DCompositionCreateDevice2{};
		decltype(DCompositionCreateDevice3)* api_DCompositionCreateDevice3{};
	public:
		HRESULT CreateDevice(IUnknown* renderingDevice, REFIID iid, void** dcompositionDevice);
	public:
		DirectComposition();
		~DirectComposition();
	};
}
