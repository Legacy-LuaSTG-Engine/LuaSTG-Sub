#pragma once

namespace Platform::RuntimeLoader
{
	class XAudio2
	{
	private:
		typedef HRESULT(WINAPI* PFN_XAudio2CreateWithVersionInfo)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR, DWORD);
		typedef HRESULT(WINAPI* PFN_XAudio2Create)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR);
	private:
		HMODULE dll_xaudio2{};
		PFN_XAudio2CreateWithVersionInfo api_XAudio2CreateWithVersionInfo{};
		PFN_XAudio2Create api_XAudio2Create{};
	public:
		HRESULT Create(IXAudio2** ppXAudio2);
	public:
		XAudio2();
		~XAudio2();
	};
}
