#pragma once

namespace Platform::RuntimeLoader
{
	class DirectWrite
	{
	private:
		HMODULE dll_dwrite{};
		decltype(DWriteCreateFactory)* api_DWriteCreateFactory{};
	public:
		HRESULT CreateFactory(
			DWRITE_FACTORY_TYPE factoryType,
			REFIID iid,
			void** factory);
	public:
		DirectWrite();
		~DirectWrite();
	};
}
