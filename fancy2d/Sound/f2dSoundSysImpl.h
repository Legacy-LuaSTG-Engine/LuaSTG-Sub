////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSysImpl.h
/// @brief fancy2D音频系统接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"
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
	class f2dXAudio2Components : public fcyRefObjImpl<f2dInterface>
	{
	public:
		Microsoft::WRL::ComPtr<IXAudio2> xa2_xaudio2;
		IXAudio2MasteringVoice* xa2_master = NULL;
		IXAudio2SubmixVoice* xa2_soundeffect = NULL;
		IXAudio2SubmixVoice* xa2_music = NULL;
	public:
		~f2dXAudio2Components()
		{
			SAFE_RELEASE_VOICE(xa2_soundeffect);
			SAFE_RELEASE_VOICE(xa2_music);
			SAFE_RELEASE_VOICE(xa2_master);
			xa2_xaudio2.Reset();
		}
	};
	f2dXAudio2Components* m_pXAudio2 = nullptr;
public:
	// 内部公开方法
	void AddXAudio2Ref();
	void DecXAudio2Ref();
	IXAudio2* GetXAudio2();
	IXAudio2SubmixVoice* GetSoundEffectChannel();
	IXAudio2SubmixVoice* GetMusicChannel();
	fResult SetSoundEffectPan(IXAudio2SourceVoice* p, float v);
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
