#include "Sound/f2dSoundSysImpl.h"

#include "Sound/f2dSoundBufferStatic.h"
#include "Sound/f2dSoundBufferDynamic.h"
#include "Sound/f2dSoundBufferPull.h"
#include "Sound/f2dSoundSpriteImpl.h"

#include "Sound/f2dWaveDecoder.h"
#include "Sound/f2dVorbisDecoder.h"

#include "Engine/f2dEngineImpl.h"

#include "fcyException.h"

////////////////////////////////////////////////////////////////////////////////

f2dSoundSysImpl::f2dSoundSysImpl(f2dEngineImpl* pEngine)
	: m_pEngine(pEngine)
{
	// 使用默认回放设备创建DSound
	HRESULT tHR;
	tHR = m_API.DLLEntry_DirectSoundCreate8(NULL, &m_pDSound8, NULL);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "DirectSoundCreate8 Failed", tHR);

	// 设置设备协作性
	tHR = m_pDSound8->SetCooperativeLevel((HWND)pEngine->GetMainWindow()->GetHandle(), DSSCL_PRIORITY);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "SetCooperativeLevel Failed.", tHR);
}

f2dSoundSysImpl::~f2dSoundSysImpl()
{
	FCYSAFEKILL(m_pDSound8);
}

fResult f2dSoundSysImpl::CreateStaticBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut)
{
	if(!pDecoder || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundBuffer* pBuffer = NULL;

	try
	{
		pBuffer = new f2dSoundBufferStatic(m_pDSound8, pDecoder, bGlobalFocus);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pBuffer;
	return FCYERR_OK;
}

fResult f2dSoundSysImpl::CreateSharedStaticBuffer(f2dSoundBuffer* pOrg, f2dSoundBuffer** pOut)
{
	if(!pOrg || !pOut)
		return FCYERR_INVAILDPARAM;
	
	*pOut = NULL;

	if(pOrg->IsDynamic())
		return FCYERR_ILLEGAL;

	IDirectSoundBuffer* tpOut = NULL;
	HRESULT tHR = m_pDSound8->DuplicateSoundBuffer(((f2dSoundBufferStatic*)pOrg)->m_pBuffer, &tpOut);
	if(FAILED(tHR))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dSoundSysImpl::CreateSharedStaticBuffer", "DuplicateSoundBuffer failed.", tHR));
		return FCYERR_INTERNALERR;
	}

	f2dSoundBufferStatic* tRet = new f2dSoundBufferStatic(tpOut);
	*pOut = tRet;

	return FCYERR_OK;
}

fResult f2dSoundSysImpl::CreateDynamicBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut)
{
	if(!pDecoder || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundBuffer* pBuffer = NULL;

	try
	{
		pBuffer = new f2dSoundBufferDynamic(m_pDSound8, pDecoder, bGlobalFocus);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pBuffer;
	return FCYERR_OK;
}

fResult f2dSoundSysImpl::CreatePullBuffer(f2dSoundDecoder* pDecoder, fuInt iBufferSampleCount, fBool bGlobalFocus, f2dSoundBuffer** pOut)
{
	if (!pDecoder || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundBuffer* pBuffer = NULL;

	try
	{
		pBuffer = new f2dSoundBufferPull(m_pDSound8, pDecoder, iBufferSampleCount, bGlobalFocus);
	}
	catch (const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pBuffer;
	return FCYERR_OK;
}

fResult f2dSoundSysImpl::CreateSoundSprite(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt iMaxCount, f2dSoundSprite** pOut)
{
	if(!pDecoder || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundSprite* pBuffer = NULL;

	try
	{
		pBuffer = new f2dSoundSpriteImpl(m_pDSound8, pDecoder, bGlobalFocus, iMaxCount);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pBuffer;
	return FCYERR_OK;
}

fResult f2dSoundSysImpl::CreateWaveDecoder(f2dStream* pStream, f2dSoundDecoder** pOut)
{
	if(!pStream || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundDecoder* pBuffer = NULL;

	try
	{
		pBuffer = new f2dWaveDecoder(pStream);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pBuffer;
	return FCYERR_OK;
}

fResult f2dSoundSysImpl::CreateOGGVorbisDecoder(f2dStream* pStream, f2dSoundDecoder** pOut)
{
	if(!pStream || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundDecoder* pBuffer = NULL;

	try
	{
		pBuffer = new f2dVorbisDecoder(pStream);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pBuffer;
	return FCYERR_OK;
}

