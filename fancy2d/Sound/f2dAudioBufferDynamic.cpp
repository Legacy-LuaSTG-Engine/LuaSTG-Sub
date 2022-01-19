#include "Sound/f2dAudioBufferDynamic.h"
#include "Sound/f2dSoundSysImpl.h"
#include "spdlog/spdlog.h"

// Called just before this voice's processing pass begins.
void WINAPI f2dAudioBufferDynamic::OnVoiceProcessingPassStart(UINT32 BytesRequired)
{
}
// Called just after this voice's processing pass ends.
void WINAPI f2dAudioBufferDynamic::OnVoiceProcessingPassEnd()
{
}
// Called when this voice has just finished playing a buffer stream
// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
void WINAPI f2dAudioBufferDynamic::OnStreamEnd()
{
}
// Called when this voice is about to start processing a new buffer.
void WINAPI f2dAudioBufferDynamic::OnBufferStart(void* pBufferContext)
{
}
// Called when this voice has just finished processing a buffer.
// The buffer can now be reused or destroyed.
void WINAPI f2dAudioBufferDynamic::OnBufferEnd(void* pBufferContext)
{
	SetEvent((HANDLE)pBufferContext);
}
// Called when this voice has just reached the end position of a loop.
void WINAPI f2dAudioBufferDynamic::OnLoopEnd(void* pBufferContext)
{
}
// Called in the event of a critical error during voice processing,
// such as a failing xAPO or an error from the hardware XMA decoder.
// The voice may have to be destroyed and re-created to recover from
// the error.  The callback arguments report which buffer was being
// processed when the error occurred, and its HRESULT code.
void WINAPI f2dAudioBufferDynamic::OnVoiceError(void* pBufferContext, HRESULT Error)
{
	HRESULT hr = gHR = Error;
	spdlog::error("[fancy2d] @IXAudio2VoiceCallback::OnVoiceError 播放音频流出错");
}

////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI f2dAudioBufferDynamic::WorkingThread(LPVOID lpThreadParameter)
{
	f2dAudioBufferDynamic* self = (f2dAudioBufferDynamic*)lpThreadParameter;

	f2dSoundDecoder* decoder = self->m_pDecoder;
	size_t buffer_bytes = decoder->GetAvgBytesPerSec();
	std::vector<uint8_t> buffer(buffer_bytes * 2);
	uint8_t* buffer1 = buffer.data();
	uint8_t* buffer2 = buffer.data() + buffer_bytes;

	IXAudio2SourceVoice* xa2_source = self->xa2_source;
	XAUDIO2_BUFFER xa2_buffer1 = {
		.Flags = 0,
		.AudioBytes = (UINT32)buffer_bytes,
		.pAudioData = buffer1,
		.PlayBegin = 0,
		.PlayLength = 0,
		.LoopBegin = 0,
		.LoopLength = 0,
		.LoopCount = 0,
		.pContext = self->event_buf1.Get(),
	};
	XAUDIO2_BUFFER xa2_buffer2 = {
		.Flags = 0,
		.AudioBytes = (UINT32)buffer_bytes,
		.pAudioData = buffer2,
		.PlayBegin = 0,
		.PlayLength = 0,
		.LoopBegin = 0,
		.LoopLength = 0,
		.LoopCount = 0,
		.pContext = self->event_buf2.Get(),
	};
	fDouble total_time = 0.0;
	fDouble buffer1_set_time = 0.0;
	fDouble buffer2_set_time = 0.0;
	fDouble buffer1_add_time = 0.0;
	fDouble buffer2_add_time = 0.0;

	HANDLE events[6] = {
		self->event_start.Get(),
		self->event_stop.Get(),
		self->event_reset.Get(),
		self->event_buf1.Get(),
		self->event_buf2.Get(),
		self->event_exit.Get(),
	};

	bool is_running = true;
	fuInt bytes_read = 0;
	fLen bytes_pos = 0;
	fDouble start_time = 0.0;
	while (is_running)
	{
		fuInt bytes_read = 0;
		DWORD wret = WaitForMultipleObjectsEx(6, events, FALSE, INFINITE, FALSE);
		switch (wret)
		{
		case WAIT_OBJECT_0:
			xa2_source->Start();
			ResetEvent(events[0]);
			break;
		case WAIT_OBJECT_0 + 1:
			xa2_source->Stop();
			ResetEvent(events[1]);
			break;
		case WAIT_OBJECT_0 + 2:
			EnterCriticalSection(&self->start_time_lock);
			start_time = self->start_time;
			LeaveCriticalSection(&self->start_time_lock);
			xa2_source->Stop();
			xa2_source->FlushSourceBuffers();
			self->total_time = 0.0;
			self->current_time = self->start_time;
			decoder->SetPosition(FCYSEEKORIGIN_BEG, (fuInt)((double)decoder->GetSamplesPerSec() * start_time) * decoder->GetBlockAlign());
			ResetEvent(events[2]);
			SetEvent(events[3]);
			SetEvent(events[4]);
			break;
		case WAIT_OBJECT_0 + 3:
			self->total_time += buffer1_add_time;
			self->current_time = buffer1_set_time;
			decoder->Read(buffer1, (fuInt)buffer_bytes, &bytes_read);
			bytes_pos = decoder->GetPosition();
			xa2_buffer1.AudioBytes = bytes_read;
			xa2_source->SubmitSourceBuffer(&xa2_buffer1);
			buffer1_set_time = (double)bytes_pos / (double)decoder->GetAvgBytesPerSec();
			buffer1_add_time = (double)bytes_read / (double)decoder->GetAvgBytesPerSec();
			ResetEvent(events[3]);
			break;
		case WAIT_OBJECT_0 + 4:
			self->total_time += buffer2_add_time;
			self->current_time = buffer2_set_time;
			decoder->Read(buffer2, (fuInt)buffer_bytes, &bytes_read);
			bytes_pos = decoder->GetPosition();
			xa2_buffer2.AudioBytes = bytes_read;
			xa2_source->SubmitSourceBuffer(&xa2_buffer2);
			buffer2_set_time = (double)bytes_pos / (double)decoder->GetAvgBytesPerSec();
			buffer2_add_time = (double)bytes_read / (double)decoder->GetAvgBytesPerSec();
			ResetEvent(events[4]);
			break;
		case WAIT_OBJECT_0 + 5:
		default:
			is_running = false; // 需要退出或者出错了
			// ResetEvent(events[5]); // 让它一直亮着
			break;
		}
	}

	xa2_source->Stop();
	xa2_source->FlushSourceBuffers(); // 防止继续使用上面的局部 buffer 导致内存读取错误

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

f2dAudioBufferDynamic::f2dAudioBufferDynamic(f2dSoundSys* pSoundSys, f2dSoundDecoder* pDecoder, fBool bGlobalFocus)
	: m_pSoundSys(pSoundSys)
	, m_pDecoder(pDecoder)
{
	if(!m_pDecoder)
		throw fcyException("f2dAudioBufferDynamic::f2dAudioBufferDynamic", "Invalid Pointer.");
	m_pDecoder->AddRef();
	
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
		throw fcyException("f2dAudioBufferDynamic::f2dAudioBufferDynamic", "IXAudio2::CreateSourceVoice Failed.");
	}

	XAUDIO2_SEND_DESCRIPTOR voice_send_music = {
		.Flags = 0,
		.pOutputVoice = pSys->GetMusicChannel(),
	};
	XAUDIO2_VOICE_SENDS voice_send_list = {
		.SendCount = 1,
		.pSends = &voice_send_music
	};
	hr = gHR = xa2_source->SetOutputVoices(&voice_send_list);
	if (FAILED(hr))
	{
		SAFE_RELEASE_VOICE(xa2_source);
		spdlog::error("[fancy2d] IXAudio2SourceVoice::SetOutputVoices -> #audio_music 调用失败");
		throw fcyException("f2dAudioBufferDynamic::f2dAudioBufferDynamic", "IXAudio2SourceVoice::SetOutputVoices Failed.");
	}

	event_start.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
	event_stop.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
	event_reset.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
	event_buf1.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET | CREATE_EVENT_INITIAL_SET, EVENT_ALL_ACCESS));
	event_buf2.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET | CREATE_EVENT_INITIAL_SET, EVENT_ALL_ACCESS));
	event_exit.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));

	InitializeCriticalSection(&start_time_lock);
	working_thread.Attach(CreateThread(NULL, 0, &WorkingThread, this, 0, NULL));
}

f2dAudioBufferDynamic::~f2dAudioBufferDynamic()
{
	SetEvent(event_exit.Get());
	WaitForSingleObject(working_thread.Get(), INFINITE);
	DeleteCriticalSection(&start_time_lock);
	SAFE_RELEASE_VOICE(xa2_source);
	FCYSAFEKILL(m_pDecoder);

	f2dSoundSysImpl* pSys = (f2dSoundSysImpl*)m_pSoundSys;
	pSys->DecXAudio2Ref();
}

////////////////////////////////////////////////////////////////////////////////

fBool f2dAudioBufferDynamic::IsDynamic()
{
	return true;
}

void f2dAudioBufferDynamic::Play()
{
	source_state = State::Play;
	ResetEvent(event_stop.Get());
	ResetEvent(event_reset.Get());
	SetEvent(event_start.Get());
}

void f2dAudioBufferDynamic::Stop()
{
	source_state = State::Stop;
	ResetEvent(event_start.Get());
	ResetEvent(event_stop.Get());
	SetEvent(event_reset.Get());
}

void f2dAudioBufferDynamic::Pause()
{
	source_state = State::Pause;
	ResetEvent(event_start.Get());
	ResetEvent(event_reset.Get());
	SetEvent(event_stop.Get());
}

fBool f2dAudioBufferDynamic::IsPlaying()
{
	return source_state == State::Play;
}

fDouble f2dAudioBufferDynamic::GetTotalTime()
{
	return total_time;
}

fDouble f2dAudioBufferDynamic::GetTime()
{
	return current_time;
}

fResult f2dAudioBufferDynamic::SetTime(fDouble Time)
{
	EnterCriticalSection(&start_time_lock);
	start_time = Time;
	LeaveCriticalSection(&start_time_lock);
	SetEvent(event_reset.Get());
	return FCYERR_OK;
}

fFloat f2dAudioBufferDynamic::GetVolume()
{
	float v = 0.0f;
	xa2_source->GetVolume(&v);
	return v;
}

fResult f2dAudioBufferDynamic::SetVolume(fFloat Value)
{
	HRESULT hr = gHR = xa2_source->SetVolume(std::clamp(Value, 0.0f, 1.0f));
	return FAILED(hr) ? FCYERR_INTERNALERR : FCYERR_OK;
}

fFloat f2dAudioBufferDynamic::GetPan()
{
	return output_pan;
}

fResult f2dAudioBufferDynamic::SetPan(fFloat Value)
{
	output_pan = std::clamp(Value, -1.0f, 1.0f);
	f2dSoundSysImpl* pSys = (f2dSoundSysImpl*)m_pSoundSys;
	return pSys->SetSoundEffectPan(xa2_source, Value);
}

fuInt f2dAudioBufferDynamic::GetFrequency()
{
	float v = 0.0f;
	xa2_source->GetFrequencyRatio(&v);
	fuInt samp = m_pDecoder->GetSamplesPerSec();
	return (fuInt)((float)samp * v);
}

fResult f2dAudioBufferDynamic::SetFrequency(fuInt Value)
{
	fuInt samp = m_pDecoder->GetSamplesPerSec();
	HRESULT hr = gHR = xa2_source->SetFrequencyRatio((float)Value / (float)samp);
	return FAILED(hr) ? FCYERR_INTERNALERR : FCYERR_OK;
}

// 应该废弃的方法

fBool f2dAudioBufferDynamic::IsLoop()
{
	return false;
}
void f2dAudioBufferDynamic::SetLoop(fBool bValue)
{
}
