#include "Sound/f2dAudioBufferStatic.h"
#include "Sound/f2dSoundSysImpl.h"
#include "spdlog/spdlog.h"

// Called just before this voice's processing pass begins.
void WINAPI f2dAudioBufferStatic::OnVoiceProcessingPassStart(UINT32 BytesRequired)
{
}
// Called just after this voice's processing pass ends.
void WINAPI f2dAudioBufferStatic::OnVoiceProcessingPassEnd()
{
}
// Called when this voice has just finished playing a buffer stream
// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
void WINAPI f2dAudioBufferStatic::OnStreamEnd()
{
	SetEvent(event_end.Get());
}
// Called when this voice is about to start processing a new buffer.
void WINAPI f2dAudioBufferStatic::OnBufferStart(void* pBufferContext)
{
}
// Called when this voice has just finished processing a buffer.
// The buffer can now be reused or destroyed.
void WINAPI f2dAudioBufferStatic::OnBufferEnd(void* pBufferContext)
{
}
// Called when this voice has just reached the end position of a loop.
void WINAPI f2dAudioBufferStatic::OnLoopEnd(void* pBufferContext)
{
}
// Called in the event of a critical error during voice processing,
// such as a failing xAPO or an error from the hardware XMA decoder.
// The voice may have to be destroyed and re-created to recover from
// the error.  The callback arguments report which buffer was being
// processed when the error occurred, and its HRESULT code.
void WINAPI f2dAudioBufferStatic::OnVoiceError(void* pBufferContext, HRESULT Error)
{
	HRESULT hr = gHR = Error;
	spdlog::error("[fancy2d] @IXAudio2VoiceCallback::OnVoiceError 播放音频流出错");
}

////////////////////////////////////////////////////////////////////////////////

f2dAudioBufferStatic::f2dAudioBufferStatic(f2dSoundSys* pSoundSys, f2dSoundDecoder* pDecoder, fBool bGlobalFocus)
	: m_pSoundSys(pSoundSys)
{
	if(!pDecoder)
		throw fcyException("f2dAudioBufferStatic::f2dAudioBufferStatic", "Invalid Pointer.");

	f2dSoundSysImpl* pSys = (f2dSoundSysImpl*)m_pSoundSys;
	pSys->AddXAudio2Ref();

	HRESULT hr = 0;

	WAVEFORMATEX fmt = {
		.wFormatTag = pDecoder->GetFormatTag(),
		.nChannels = pDecoder->GetChannelCount(),
		.nSamplesPerSec = pDecoder->GetSamplesPerSec(),
		.nAvgBytesPerSec = pDecoder->GetAvgBytesPerSec(),
		.nBlockAlign = pDecoder->GetBlockAlign(),
		.wBitsPerSample = pDecoder->GetBitsPerSample(),
		.cbSize = 0, // 我看还有谁TM写错成 sizeof(WAVEFORMATEX)
	};
	hr = gHR = pSys->GetXAudio2()->CreateSourceVoice(&xa2_source, &fmt, 0, 2.0f, this);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IXAudio2::CreateSourceVoice 调用失败");
		throw fcyException("f2dAudioBufferStatic::f2dAudioBufferStatic", "IXAudio2::CreateSourceVoice Failed.");
	}

	XAUDIO2_SEND_DESCRIPTOR voice_send_se = {
		.Flags = 0,
		.pOutputVoice = pSys->GetSoundEffectChannel(),
	};
	XAUDIO2_VOICE_SENDS voice_send_list = {
		.SendCount = 1,
		.pSends = &voice_send_se
	};
	hr = gHR = xa2_source->SetOutputVoices(&voice_send_list);
	if (FAILED(hr))
	{
		SAFE_RELEASE_VOICE(xa2_source);
		spdlog::error("[fancy2d] IXAudio2SourceVoice::SetOutputVoices -> #audio_soundeffect 调用失败");
		throw fcyException("f2dAudioBufferStatic::f2dAudioBufferStatic", "IXAudio2SourceVoice::SetOutputVoices Failed.");
	}

	pcm_data.resize(pDecoder->GetBufferSize());
	fuInt bytes_read = 0;
	if (FCYFAILED(pDecoder->Read(pcm_data.data(), pDecoder->GetBufferSize(), &bytes_read)))
	{
		spdlog::error("[fancy2d] f2dSoundDecoder::Read 调用失败");
		throw fcyException("f2dAudioBufferStatic::f2dAudioBufferStatic", "f2dSoundDecoder::Read Failed.");
	}
	xa2_buffer = XAUDIO2_BUFFER{
		.Flags = XAUDIO2_END_OF_STREAM,
		.AudioBytes = bytes_read,
		.pAudioData = pcm_data.data(),
		.PlayBegin = 0,
		.PlayLength = 0,
		.LoopBegin = 0,
		.LoopLength = 0,
		.LoopCount = 0,
		.pContext = NULL,
	};

	event_end.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));

	channel_cnt = pDecoder->GetChannelCount();
	sample_rate = pDecoder->GetSamplesPerSec();
}

f2dAudioBufferStatic::~f2dAudioBufferStatic()
{
	SAFE_RELEASE_VOICE(xa2_source);

	f2dSoundSysImpl* pSys = (f2dSoundSysImpl*)m_pSoundSys;
	pSys->DecXAudio2Ref();
}

fBool f2dAudioBufferStatic::IsDynamic()
{
	return false;
}

void f2dAudioBufferStatic::Play()
{
	is_playing = true;
	xa2_source->Start();
}

void f2dAudioBufferStatic::Stop()
{
	is_playing = false;
	xa2_source->Stop();
	xa2_source->FlushSourceBuffers();
	xa2_source->SubmitSourceBuffer(&xa2_buffer);
	ResetEvent(event_end.Get());
}

void f2dAudioBufferStatic::Pause()
{
	is_playing = false;
	xa2_source->Stop();
}

fBool f2dAudioBufferStatic::IsLoop()
{
	return false;
}

void f2dAudioBufferStatic::SetLoop(fBool bValue)
{
}

fBool f2dAudioBufferStatic::IsPlaying()
{
	DWORD ret = WaitForSingleObjectEx(event_end.Get(), 0, FALSE);
	return is_playing && ret != WAIT_OBJECT_0;
}

fDouble f2dAudioBufferStatic::GetTotalTime()
{
	return 0.0;
}

fDouble f2dAudioBufferStatic::GetTime()
{
	return 0.0;
}

fResult f2dAudioBufferStatic::SetTime(fDouble Time)
{
	return FCYERR_OK;
}

fFloat f2dAudioBufferStatic::GetVolume()
{
	float v = 0.0;
	xa2_source->GetVolume(&v);
	return v;
}

fResult f2dAudioBufferStatic::SetVolume(fFloat Value)
{
	HRESULT hr = gHR = xa2_source->SetVolume(std::clamp(Value, 0.0f, 1.0f));
	return FAILED(hr) ? FCYERR_INTERNALERR : FCYERR_OK;
}

fFloat f2dAudioBufferStatic::GetPan()
{
	float v[7] = {};
	xa2_source->GetChannelVolumes(channel_cnt, v);
	if (v[0] < 1.0f)
	{
		return 1.0f - v[0];
	}
	else if (v[1] < 1.0f)
	{
		return -(1.0f - v[1]);
	}
	else
	{
		return 0.0f;
	}
}

fResult f2dAudioBufferStatic::SetPan(fFloat Value)
{
	float v[7] = {};
	Value = std::clamp(Value, -1.0f, 1.0f);
	if (Value < 0.0f)
	{
		v[0] = 1.0f;
		v[1] = 1.0f + Value;
	}
	else
	{
		v[0] = 1.0f - Value;
		v[1] = 1.0f;
	}
	HRESULT hr = gHR = xa2_source->SetChannelVolumes(channel_cnt, v);
	return FAILED(hr) ? FCYERR_INTERNALERR : FCYERR_OK;
}

fuInt f2dAudioBufferStatic::GetFrequency()
{
	float v = 0.0f;
	xa2_source->GetFrequencyRatio(&v);
	return (fuInt)((float)sample_rate * v);
}

fResult f2dAudioBufferStatic::SetFrequency(fuInt Value)
{
	HRESULT hr = gHR = xa2_source->SetFrequencyRatio((float)Value / (float)sample_rate);
	return FAILED(hr) ? FCYERR_INTERNALERR : FCYERR_OK;
}
