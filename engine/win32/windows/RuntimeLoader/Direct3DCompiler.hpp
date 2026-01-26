#pragma once

namespace Platform::RuntimeLoader
{
	class Direct3DCompiler
	{
	private:
		HMODULE dll_d3dcompiler{};
		decltype(D3DCompile)* api_D3DCompile{};
		decltype(D3DReflect)* api_D3DReflect{};
	public:
		HRESULT Compile(LPCVOID pSrcData,
			SIZE_T SrcDataSize,
			LPCSTR pSourceName,
			CONST D3D_SHADER_MACRO* pDefines,
			ID3DInclude* pInclude,
			LPCSTR pEntrypoint,
			LPCSTR pTarget,
			UINT Flags1,
			UINT Flags2,
			ID3DBlob** ppCode,
			ID3DBlob** ppErrorMsgs);
		HRESULT Reflect(LPCVOID pSrcData,
				SIZE_T SrcDataSize,
				REFIID pInterface,
				void** ppReflector);
	public:
		Direct3DCompiler();
		~Direct3DCompiler();
	};
}
