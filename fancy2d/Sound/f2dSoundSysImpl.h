////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSysImpl.h
/// @brief fancy2D音频系统接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"

#include "Sound/f2dSoundSysAPI.h"
#include "Common/f2dStandardCommon.hpp"
#include "Common/f2dWindowsCommon.h"

#include "f2dSoundSys.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 声音系统实现
////////////////////////////////////////////////////////////////////////////////
class f2dSoundSysImpl :
	public fcyRefObjImpl<f2dSoundSys>
{
private:
	f2dEngineImpl* m_pEngine = nullptr;

	f2dSoundSysAPI m_API;

	IDirectSound8* m_pDSound8;
	Microsoft::WRL::ComPtr<IXAudio2> xa2_xaudio2;
	IXAudio2MasteringVoice* xa2_master = NULL;
	IXAudio2SubmixVoice* xa2_soundeffect = NULL;
	IXAudio2SubmixVoice* xa2_music = NULL;
public:
	// 内部公开方法
	void AddXAudio2Ref();
	void DecXAudio2Ref();
	IXAudio2* GetXAudio2() { return xa2_xaudio2.Get(); }
	IXAudio2SubmixVoice* GetSoundEffectChannel() { return xa2_soundeffect; }
	IXAudio2SubmixVoice* GetMusicChannel() { return xa2_music; }
public:
	fResult CreateStaticBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut);
	fResult CreateSharedStaticBuffer(f2dSoundBuffer* pOrg, f2dSoundBuffer** pOut);
	fResult CreateDynamicBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut);
	fResult CreatePullBuffer(f2dSoundDecoder* pDecoder, fuInt iBufferSampleCount, fBool bGlobalFocus, f2dSoundBuffer** pOut);
	fResult CreateSoundSprite(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt iMaxCount, f2dSoundSprite** pOut);

	fResult CreateWaveDecoder(f2dStream* pStream, f2dSoundDecoder** pOut);
	fResult CreateOGGVorbisDecoder(f2dStream* pStream, f2dSoundDecoder** pOut);

	fResult SetSoundEffectChannelVolume(fFloat v);
	fResult SetMusicChannelVolume(fFloat v);
	fFloat GetSoundEffectChannelVolume();
	fFloat GetMusicChannelVolume();
public:
	f2dSoundSysImpl(f2dEngineImpl* pEngine);
protected:
	~f2dSoundSysImpl();
};
