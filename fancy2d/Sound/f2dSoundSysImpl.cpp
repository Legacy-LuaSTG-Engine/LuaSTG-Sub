#include "Sound/f2dSoundSysImpl.h"

#include "Sound/f2dAudioBufferStatic.h"
#include "Sound/f2dAudioBufferDynamic.h"

#include "Sound/f2dWaveDecoder.h"
#include "Sound/f2dVorbisDecoder.h"

#include "Engine/f2dEngineImpl.h"

#include "fcyException.h"

#include <ks.h>
#include <ksmedia.h>

////////////////////////////////////////////////////////////////////////////////

f2dSoundSysImpl::f2dSoundSysImpl(f2dEngineImpl* pEngine)
	: m_pEngine(pEngine)
{
	m_pXAudio2 = new f2dXAudio2Components;
	
	HRESULT hr = 0;
	
	hr = gHR = XAudio2Create(&m_pXAudio2->xa2_xaudio2);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] XAudio2Create 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "XAudio2Create Failed.", hr);
	}
	#ifdef _DEBUG
	XAUDIO2_DEBUG_CONFIGURATION debug_cfg = {
		.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS,
	};
	m_pXAudio2->xa2_xaudio2->SetDebugConfiguration(&debug_cfg);
	#endif

	hr = gHR = m_pXAudio2->xa2_xaudio2->CreateMasteringVoice(&m_pXAudio2->xa2_master);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateMasteringVoice 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2::CreateMasteringVoice Failed.", hr);
	}

	XAUDIO2_VOICE_DETAILS voice_info = {};
	m_pXAudio2->xa2_master->GetVoiceDetails(&voice_info);

	hr = gHR = m_pXAudio2->xa2_xaudio2->CreateSubmixVoice(&m_pXAudio2->xa2_soundeffect, 2, voice_info.InputSampleRate); // 固定2声道
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateSubmixVoice -> #audio_soundeffect 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2::CreateSubmixVoice Failed.", hr);
	}

	hr = gHR = m_pXAudio2->xa2_xaudio2->CreateSubmixVoice(&m_pXAudio2->xa2_music, 2, voice_info.InputSampleRate); // 固定2声道
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateSubmixVoice -> #audio_music 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2::CreateSubmixVoice Failed.", hr);
	}

	XAUDIO2_SEND_DESCRIPTOR voice_send_master = {
		.Flags = 0,
		.pOutputVoice = m_pXAudio2->xa2_master,
	};
	XAUDIO2_VOICE_SENDS voice_send_list = {
		.SendCount = 1,
		.pSends = &voice_send_master
	};

	hr = gHR = m_pXAudio2->xa2_soundeffect->SetOutputVoices(&voice_send_list);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] @audio_soundeffect -> IXAudio2SubmixVoice::SetOutputVoices -> #audio_master 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2SubmixVoice::SetOutputVoices Failed.", hr);
	}

	hr = gHR = m_pXAudio2->xa2_music->SetOutputVoices(&voice_send_list);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] @audio_music -> IXAudio2SubmixVoice::SetOutputVoices -> #audio_master 调用失败");
		throw fcyWin32COMException("f2dSoundSysImpl::f2dSoundSysImpl", "IXAudio2SubmixVoice::SetOutputVoices Failed.", hr);
	}
}
f2dSoundSysImpl::~f2dSoundSysImpl()
{
	FCYSAFEKILL(m_pXAudio2);
}

void f2dSoundSysImpl::AddXAudio2Ref() { if (m_pXAudio2) m_pXAudio2->AddRef(); }
void f2dSoundSysImpl::DecXAudio2Ref() { if (m_pXAudio2) m_pXAudio2->Release(); }
IXAudio2* f2dSoundSysImpl::GetXAudio2() { return m_pXAudio2 ? m_pXAudio2->xa2_xaudio2.Get() : nullptr; }
IXAudio2SubmixVoice* f2dSoundSysImpl::GetSoundEffectChannel() { return m_pXAudio2 ? m_pXAudio2->xa2_soundeffect : nullptr; }
IXAudio2SubmixVoice* f2dSoundSysImpl::GetMusicChannel() { return m_pXAudio2 ? m_pXAudio2->xa2_music : nullptr; }
fResult f2dSoundSysImpl::SetSoundEffectPan(IXAudio2SourceVoice* p, float v)
{
	XAUDIO2_VOICE_DETAILS detail = {};
	p->GetVoiceDetails(&detail);
	float output_matrix_2x2[4] = {0};
	float pan = std::clamp(v, -1.0f, 1.0f);
	switch (detail.InputChannels)
	{
	case 1:
		if (pan < 0.0f)
		{
			output_matrix_2x2[0] = 1.0f;
			output_matrix_2x2[1] = 1.0f + pan;
		}
		else
		{
			output_matrix_2x2[0] = 1.0f - pan;
			output_matrix_2x2[1] = 1.0f;
		}
	case 2:
		if (pan < 0.0f)
		{
			output_matrix_2x2[0] = 1.0f;
			output_matrix_2x2[3] = 1.0f + pan;
		}
		else
		{
			output_matrix_2x2[0] = 1.0f - pan;
			output_matrix_2x2[3] = 1.0f;
		}
		break;
	default:
		spdlog::error("[fancy2d] 无法识别的的音频声道数量：{}，无法设置音量平衡", detail.InputChannels);
		return FCYERR_NOTSUPPORT;
	}
	HRESULT hr = gHR = p->SetOutputMatrix(NULL, detail.InputChannels, 2, output_matrix_2x2);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2SourceVoice::SetOutputMatrix 调用失败");
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
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
	//if(!pOrg || !pOut)
	//	return FCYERR_INVAILDPARAM;
	//
	//*pOut = NULL;
	//
	//if(pOrg->IsDynamic())
	//	return FCYERR_ILLEGAL;
	//
	//IDirectSoundBuffer* tpOut = NULL;
	//HRESULT tHR = m_pDSound8->DuplicateSoundBuffer(((f2dSoundBufferStatic*)pOrg)->m_pBuffer, &tpOut);
	//if(FAILED(tHR))
	//{
	//	m_pEngine->ThrowException(fcyWin32COMException("f2dSoundSysImpl::CreateSharedStaticBuffer", "DuplicateSoundBuffer failed.", tHR));
	//	return FCYERR_INTERNALERR;
	//}
	//
	//f2dSoundBufferStatic* tRet = new f2dSoundBufferStatic(tpOut);
	//*pOut = tRet;
	//
	//return FCYERR_OK;
	return FCYERR_NOTIMPL;
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
	//if (!pDecoder || !pOut)
	//	return FCYERR_INVAILDPARAM;
	//
	//*pOut = NULL;
	//f2dSoundBuffer* pBuffer = NULL;
	//
	//try
	//{
	//	pBuffer = new f2dSoundBufferPull(m_pDSound8, pDecoder, iBufferSampleCount, bGlobalFocus);
	//}
	//catch (const fcyException& e)
	//{
	//	m_pEngine->ThrowException(e);
	//	return FCYERR_INTERNALERR;
	//}
	//
	//*pOut = pBuffer;
	//return FCYERR_OK;
	return FCYERR_NOTIMPL;
}
fResult f2dSoundSysImpl::CreateSoundSprite(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt iMaxCount, f2dSoundSprite** pOut)
{
	//if(!pDecoder || !pOut)
	//	return FCYERR_INVAILDPARAM;
	//
	//*pOut = NULL;
	//f2dSoundSprite* pBuffer = NULL;
	//
	//try
	//{
	//	pBuffer = new f2dSoundSpriteImpl(m_pDSound8, pDecoder, bGlobalFocus, iMaxCount);
	//}
	//catch(const fcyException& e)
	//{
	//	m_pEngine->ThrowException(e);
	//	return FCYERR_INTERNALERR;
	//}
	//
	//*pOut = pBuffer;
	//return FCYERR_OK;
	return FCYERR_NOTIMPL;
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
	HRESULT hr = gHR = m_pXAudio2->xa2_soundeffect->SetVolume(std::clamp(v, 0.0f, 1.0f));
	return SUCCEEDED(hr) ? FCYERR_OK : FCYERR_INTERNALERR;
}
fResult f2dSoundSysImpl::SetMusicChannelVolume(fFloat v)
{
	HRESULT hr = gHR = m_pXAudio2->xa2_music->SetVolume(std::clamp(v, 0.0f, 1.0f));
	return SUCCEEDED(hr) ? FCYERR_OK : FCYERR_INTERNALERR;
}
fFloat f2dSoundSysImpl::GetSoundEffectChannelVolume()
{
	float v = 0.0f;
	m_pXAudio2->xa2_soundeffect->GetVolume(&v);
	return v;
}
fFloat f2dSoundSysImpl::GetMusicChannelVolume()
{
	float v = 0.0f;
	m_pXAudio2->xa2_music->GetVolume(&v);
	return v;
}
