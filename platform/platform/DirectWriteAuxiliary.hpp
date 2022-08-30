#pragma once
#ifndef DWRITE_H_INCLUDED
#include <dwrite.h>
#endif

interface DWRITE_DECLARE_INTERFACE("415fb37f-ac9f-4f4b-8dba-6fd11f4741a8") IDirectWriteAuxiliary : public IUnknown
{
	STDMETHOD_(IDWriteFontCollection*, GetCustomFontCollection)() PURE;
	STDMETHOD(AddSearchPath)(_In_z_ WCHAR const* filePath) PURE;
	STDMETHOD(Refresh)() PURE;
};

HRESULT WINAPI DirectWriteCreateAuxiliary(_In_ IDWriteFactory* pFactory, _COM_Outptr_ IDirectWriteAuxiliary** ppAuxiliary) WIN_NOEXCEPT;
