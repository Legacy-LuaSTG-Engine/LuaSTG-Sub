#include "Renderer/f2dRendererImpl.h"

#include "RenderDev/f2dRenderDeviceImpl.h"
#include "RenderDev/f2dRenderDevice11.h"

#include "Renderer/f2dSpriteImpl.h"
#include "Renderer/f2dGeometryRendererImpl.h"
#include "Renderer/f2dFontRendererImpl.h"
#include "Renderer/f2dFontFileProvider.h"
#include "Renderer/f2dFontTexProvider.h"
#include "Renderer/f2dTrueTypeFontProvider.h"
#include "Renderer/f2dSpriteAnimationImpl.h"
#include "Renderer/f2dParticle.h"

#include "Engine/f2dEngineImpl.h"
#include "f2dConfig.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

void f2dRendererImpl::DefaultListener::OnRenderDeviceLost()
{
	m_pEngine->SendMsg(F2DMSG_RENDER_ONDEVLOST);
}

void f2dRendererImpl::DefaultListener::OnRenderDeviceReset()
{
	m_pEngine->SendMsg(F2DMSG_RENDER_ONDEVRESET);
}

////////////////////////////////////////////////////////////////////////////////

f2dRendererImpl::f2dRendererImpl(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)
	: m_pEngine(pEngine), m_pDev(NULL), m_DefaultListener(pEngine)
{
#ifdef F2D_GRAPHIC_API_D3D11
	m_pDev = new f2dRenderDevice11(pEngine, BackBufferWidth, BackBufferHeight, Windowed, VSync, AALevel);
#else
	m_pDev = new f2dRenderDeviceImpl(pEngine, BackBufferWidth, BackBufferHeight, Windowed, VSync, AALevel);
#endif
	m_pDev->AttachListener(&m_DefaultListener);
}

f2dRendererImpl::~f2dRendererImpl()
{
	if (m_pDev)
	{
		m_pDev->RemoveListener(&m_DefaultListener);
	#ifdef F2D_GRAPHIC_API_D3D11
		((f2dRenderDevice11*)m_pDev)->Release();
	#else
		((f2dRenderDeviceImpl*)m_pDev)->Release();
	#endif
		m_pDev = nullptr;
	}
}

f2dRenderDevice* f2dRendererImpl::GetDevice()
{
	return m_pDev;
}

fResult f2dRendererImpl::CreateSprite2D(f2dTexture2D* pTex, f2dSprite** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pTex)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dSpriteImpl(pTex);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateSprite2D(f2dTexture2D* pTex, const fcyRect& Org, f2dSprite** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pTex)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dSpriteImpl(pTex, Org);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateSprite2D(f2dTexture2D* pTex, const fcyRect& Org, const fcyVec2& HotSopt, f2dSprite** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pTex)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dSpriteImpl(pTex, Org, HotSopt);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateGeometryRenderer(f2dGeometryRenderer** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dGeometryRendererImpl();
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateFontRenderer(f2dFontProvider* pProvider, f2dFontRenderer** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dFontRendererImpl(pProvider);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateFontFromFile(
	f2dStream* pStream, fuInt FaceIndex, const fcyVec2& FontSize, const fcyVec2& BBoxSize, F2DFONTFLAG Flag, f2dFontProvider** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pStream)
		return FCYERR_INVAILDPARAM;

	try
	{
		//*pOut = new f2dFontFileProvider(m_pDev, pStream, FontSize, BBoxSize, FaceIndex, Flag);
		f2dTrueTypeFontParam param = {};
		param.font_file = pStream;
		param.font_face = FaceIndex;
		param.font_size = FontSize;
		*pOut = new f2dTrueTypeFontProvider(m_pDev, &param, 1);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateFontFromMemory(
	fcyMemStream* pStream, fuInt FaceIndex, const fcyVec2& FontSize, const fcyVec2& BBoxSize, F2DFONTFLAG Flag, f2dFontProvider** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pStream)
		return FCYERR_INVAILDPARAM;

	try
	{
		//*pOut = new f2dFontFileProvider(m_pDev, pStream, FontSize, BBoxSize, FaceIndex, Flag);
		f2dTrueTypeFontParam param = {};
		param.font_source = pStream;
		param.font_face = FaceIndex;
		param.font_size = FontSize;
		*pOut = new f2dTrueTypeFontProvider(m_pDev, &param, 1);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateFontFromMemory(
		f2dFontProviderParam param, f2dTrueTypeFontParam* fonts, fuInt count, f2dFontProvider** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!fonts || count == 0)
		return FCYERR_INVAILDPARAM;
	
	try
	{
		//*pOut = new f2dFontFileProvider(m_pDev, param, fonts, count);
		*pOut = new f2dTrueTypeFontProvider(m_pDev, fonts, count);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateFontFromParam(
	f2dTrueTypeFontParam* fonts, fuInt count, f2dFontProvider** pOut)
{
	if (pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if (!fonts || count == 0)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dTrueTypeFontProvider(m_pDev, fonts, count);
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateSystemFont(fcStrW FaceName, fuInt FaceIndex, const fcyVec2& FontSize, F2DFONTFLAG Flag, f2dFontProvider** pOut)
{
	fcyRefPointer<fcyFileStream> pStream;
	
	try
	{
		pStream.DirectSet(enumSystemFont(FaceName));
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return CreateFontFromFile(pStream, FaceIndex, FontSize, fcyVec2(0.0f, 0.0f), Flag, pOut);
}

fResult f2dRendererImpl::CreateFontFromTex(f2dStream* pDefineFile, f2dTexture2D* pTex, f2dFontProvider** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pDefineFile)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dFontTexProvider(pDefineFile, pTex);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateFontFromTex(fcStrW pDefineText, f2dTexture2D* pTex, f2dFontProvider** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pDefineText)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dFontTexProvider(pDefineText, pTex);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateSpriteAnimation(f2dSpriteAnimation** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dSpriteAnimationImpl();
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRendererImpl::CreateParticlePool(f2dParticlePool** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dParticlePoolImpl();
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

#include <ShlObj.h>

fcyFileStream* f2dRendererImpl::enumSystemFont(fcStrW FontName)
{
	// 打开注册表 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
	// 枚举符合要求的字体
	HKEY tKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &tKey) == ERROR_SUCCESS)
	{
		// 枚举子键
		int tIndex = 0;
		fCharW tKeyName[MAX_PATH];
		DWORD tKeyNameLen = MAX_PATH;
		DWORD tKeyType = 0;
		BYTE tKeyData[MAX_PATH];
		DWORD tKeyDataLen = MAX_PATH;

		while (RegEnumValue(tKey, tIndex, tKeyName, &tKeyNameLen, NULL, &tKeyType, tKeyData, &tKeyDataLen) == ERROR_SUCCESS)
		{
			// 检查是否为相应字体
			if (tKeyType == REG_SZ)
			{
				fCharW tFontName[MAX_PATH];
				fCharW tFontType[MAX_PATH];
				if (2 == swscanf_s(tKeyName, L"%[^()] (%[^()])", tFontName, MAX_PATH, tFontType, MAX_PATH))
				{
					int tLen = wcslen(tFontName);

					// 去除scanf匹配的空格
					if (!tLen)
						continue;
					else
						if (tFontName[tLen - 1] == L' ')
							tFontName[tLen - 1] = L'\0';

					// 是否为需要的字体
					if (wcscmp(tFontName, FontName) == 0)
					{
						RegCloseKey(tKey);

						wchar_t tTextDir[MAX_PATH];
						SHGetSpecialFolderPath(GetDesktopWindow(), tTextDir, CSIDL_FONTS, 0);
						wstring tPath = tTextDir;
						tPath += L'\\';
						tPath += (wchar_t*)tKeyData;

						// 打开流
						fcyFileStream* pStream = new fcyFileStream(tPath.c_str(), false);
						return pStream;
					}
				}
			}

			// 继续枚举
			tKeyNameLen = MAX_PATH;
			tKeyType = 0;
			tKeyDataLen = MAX_PATH;
			tIndex++;
		}
	}
	else
	{
		throw fcyWin32Exception("f2dRendererImpl::enumSystemFont", "RegOpenKeyEx failed.");
	}

	RegCloseKey(tKey);

	throw fcyException("f2dRendererImpl::enumSystemFont", "Font not exist.");
}
