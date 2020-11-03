////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSysImpl.h
/// @brief fancy2D音频系统接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"

#include "Sound/f2dSoundSysAPI.h"

#include "f2dSoundSys.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 声音系统实现
////////////////////////////////////////////////////////////////////////////////
class f2dSoundSysImpl :
	public fcyRefObjImpl<f2dSoundSys>
{
private:
	f2dEngineImpl* m_pEngine;

	f2dSoundSysAPI m_API;

	IDirectSound8* m_pDSound8;
public:
	fResult CreateStaticBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut);
	fResult CreateSharedStaticBuffer(f2dSoundBuffer* pOrg, f2dSoundBuffer** pOut);
	fResult CreateDynamicBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut);
	fResult CreatePullBuffer(f2dSoundDecoder* pDecoder, fuInt iBufferSampleCount, fBool bGlobalFocus, f2dSoundBuffer** pOut);
	fResult CreateSoundSprite(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt iMaxCount, f2dSoundSprite** pOut);

	fResult CreateWaveDecoder(f2dStream* pStream, f2dSoundDecoder** pOut);
	fResult CreateOGGVorbisDecoder(f2dStream* pStream, f2dSoundDecoder** pOut);
public:
	f2dSoundSysImpl(f2dEngineImpl* pEngine);
protected:
	~f2dSoundSysImpl();
};
