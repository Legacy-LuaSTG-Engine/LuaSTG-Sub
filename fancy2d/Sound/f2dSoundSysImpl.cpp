#include "Sound/f2dSoundSysImpl.h"

#include "Sound/f2dSoundBufferStatic.h"
#include "Sound/f2dSoundBufferDynamic.h"
#include "Sound/f2dSoundBufferPull.h"
#include "Sound/f2dSoundSpriteImpl.h"
#include "Sound/f2dAudioBufferStatic.h"
#include "Sound/f2dAudioBufferDynamic.h"

#include "Sound/f2dWaveDecoder.h"
#include "Sound/f2dVorbisDecoder.h"

#include "Engine/f2dEngineImpl.h"

#include "fcyException.h"
#include "spdlog/spdlog.h"

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

	HRESULT hr = 0;

	hr = gHR = XAudio2Create(&xa2_xaudio2);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] XAudio2Create 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "XAudio2Create Failed.", hr);
	}

	hr = gHR = xa2_xaudio2->CreateMasteringVoice(&xa2_master);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateMasteringVoice 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2::CreateMasteringVoice Failed.", hr);
	}

	XAUDIO2_VOICE_DETAILS voice_info = {};
	xa2_master->GetVoiceDetails(&voice_info);

	hr = gHR = xa2_xaudio2->CreateSubmixVoice(&xa2_soundeffect, voice_info.InputChannels, voice_info.InputSampleRate);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateSubmixVoice -> #audio_soundeffect 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2::CreateSubmixVoice Failed.", hr);
	}

	hr = gHR = xa2_xaudio2->CreateSubmixVoice(&xa2_music, voice_info.InputChannels, voice_info.InputSampleRate);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateSubmixVoice -> #audio_music 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2::CreateSubmixVoice Failed.", hr);
	}

	XAUDIO2_SEND_DESCRIPTOR voice_send_master = {
		.Flags = 0,
		.pOutputVoice = xa2_master,
	};
	XAUDIO2_VOICE_SENDS voice_send_list = {
		.SendCount = 1,
		.pSends = &voice_send_master
	};

	hr = gHR = xa2_soundeffect->SetOutputVoices(&voice_send_list);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] @audio_soundeffect -> IXAudio2SubmixVoice::SetOutputVoices -> #audio_master 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2SubmixVoice::SetOutputVoices Failed.", hr);
	}

	hr = gHR = xa2_music->SetOutputVoices(&voice_send_list);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] @audio_music -> IXAudio2SubmixVoice::SetOutputVoices -> #audio_master 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2SubmixVoice::SetOutputVoices Failed.", hr);
	}
}

f2dSoundSysImpl::~f2dSoundSysImpl()
{
	FCYSAFEKILL(m_pDSound8);
	SAFE_RELEASE_VOICE(xa2_soundeffect);
	SAFE_RELEASE_VOICE(xa2_music);
	SAFE_RELEASE_VOICE(xa2_master);
}

void f2dSoundSysImpl::AddXAudio2Ref()
{
	xa2_xaudio2->AddRef();
}
void f2dSoundSysImpl::DecXAudio2Ref()
{
	xa2_xaudio2->Release();
}

fResult f2dSoundSysImpl::CreateStaticBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut)
{
	if(!pDecoder || !pOut)
		return FCYERR_INVAILDPARAM;

	*pOut = NULL;
	f2dSoundBuffer* pBuffer = NULL;

	try
	{
		//pBuffer = new f2dSoundBufferStatic(m_pDSound8, pDecoder, bGlobalFocus);
		pBuffer = new f2dAudioBufferStatic(this, pDecoder, bGlobalFocus);
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
		//pBuffer = new f2dSoundBufferDynamic(m_pDSound8, pDecoder, bGlobalFocus);
		pBuffer = new f2dAudioBufferDynamic(this, pDecoder, bGlobalFocus);
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

fResult f2dSoundSysImpl::SetSoundEffectChannelVolume(fFloat v)
{
	HRESULT hr = gHR = xa2_soundeffect->SetVolume(std::clamp(v, 0.0f, 1.0f));
	return SUCCEEDED(hr) ? FCYERR_OK : FCYERR_INTERNALERR;
}
fResult f2dSoundSysImpl::SetMusicChannelVolume(fFloat v)
{
	HRESULT hr = gHR = xa2_music->SetVolume(std::clamp(v, 0.0f, 1.0f));
	return SUCCEEDED(hr) ? FCYERR_OK : FCYERR_INTERNALERR;
}
fFloat f2dSoundSysImpl::GetSoundEffectChannelVolume()
{
	float v = 0.0f;
	xa2_soundeffect->GetVolume(&v);
	return v;
}
fFloat f2dSoundSysImpl::GetMusicChannelVolume()
{
	float v = 0.0f;
	xa2_music->GetVolume(&v);
	return v;
}
