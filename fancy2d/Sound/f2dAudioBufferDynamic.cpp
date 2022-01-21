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
	action_queue.notifyBufferAvailable((size_t)pBufferContext);
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
	std::vector<uint8_t> raw_buffer(buffer_bytes * 2);
	uint8_t* buffer[2] = { raw_buffer.data(), raw_buffer.data() + buffer_bytes };

	XAUDIO2_BUFFER xa2_buffer[2] = {
		XAUDIO2_BUFFER {
			.Flags = 0,
			.AudioBytes = (UINT32)buffer_bytes,
			.pAudioData = buffer[0],
			.PlayBegin = 0,
			.PlayLength = 0,
			.LoopBegin = 0,
			.LoopLength = 0,
			.LoopCount = 0,
			.pContext = (void*)(size_t)0,
		},
		XAUDIO2_BUFFER {
			.Flags = 0,
			.AudioBytes = (UINT32)buffer_bytes,
			.pAudioData = buffer[1],
			.PlayBegin = 0,
			.PlayLength = 0,
			.LoopBegin = 0,
			.LoopLength = 0,
			.LoopCount = 0,
			.pContext = (void*)(size_t)1,
		},
	};
	fDouble buffer_add_time[2] = { 0.0, 0.0 };
	fDouble buffer_set_time[2] = { 0.0, 0.0 };

	bool is_running = true;
	Action action = {};
	fDouble start_time = 0.0;
	size_t buffer_index = 0;
	fLen bytes_pos = 0;
	fuInt bytes_read = 0;
	while (is_running)
	{
		self->action_queue.reciveAction(action);
		switch (action.type)
		{
		default:
		case ActionType::Exit:
			spdlog::debug("ActionType::Exit");
			is_running = false; // 该滚蛋了
			self->xa2_source->Stop();
			self->xa2_source->FlushSourceBuffers(); // 防止继续使用上面的局部 buffer 导致内存读取错误
			break;
		case ActionType::Stop:
			spdlog::debug("ActionType::Stop");
			self->xa2_source->Stop();
			break;
		case ActionType::Start:
			spdlog::debug("ActionType::Start");
			self->xa2_source->Start();
			break;
		case ActionType::Reset:
			spdlog::debug("ActionType::Reset [{}]", action.action_reset.play ? "X" : "");
			self->xa2_source->Stop();
			self->xa2_source->FlushSourceBuffers();
			self->total_time = 0.0;
			self->current_time = start_time;
			decoder->SetPosition(FCYSEEKORIGIN_BEG, (fuInt)((fDouble)decoder->GetSamplesPerSec() * start_time) * decoder->GetBlockAlign());
			buffer_add_time[0] = buffer_add_time[1] = 0.0;
			buffer_set_time[0] = buffer_set_time[1] = start_time;
			if (action.action_reset.play)
			{
				self->xa2_source->Start();
			}
			break;
		case ActionType::SetTime:
			spdlog::debug("ActionType::SetTime ({})", action.action_set_time.time);
			start_time = action.action_set_time.time;
			self->xa2_source->FlushSourceBuffers();
			self->total_time = 0.0;
			self->current_time = start_time;
			decoder->SetPosition(FCYSEEKORIGIN_BEG, (fuInt)((fDouble)decoder->GetSamplesPerSec() * start_time) * decoder->GetBlockAlign());
			buffer_add_time[0] = buffer_add_time[1] = 0.0;
			buffer_set_time[0] = buffer_set_time[1] = start_time;
			break;
		case ActionType::BufferAvailable:
			spdlog::debug("ActionType::BufferAvailable [{}]", action.action_buffer_available.index);
			buffer_index = action.action_buffer_available.index;
			self->total_time += buffer_add_time[buffer_index];
			self->current_time = buffer_set_time[buffer_index];
			decoder->Read(buffer[buffer_index], (fuInt)buffer_bytes, &bytes_read);
			bytes_pos = decoder->GetPosition();
			xa2_buffer[buffer_index].AudioBytes = bytes_read;
			self->xa2_source->SubmitSourceBuffer(&xa2_buffer[buffer_index]);
			buffer_add_time[buffer_index] = (double)bytes_read / (double)decoder->GetAvgBytesPerSec();
			buffer_set_time[buffer_index] = (double)bytes_pos / (double)decoder->GetAvgBytesPerSec();
			break;
		}
	}

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

	if (!action_queue.createObjects())
	{
		spdlog::error("[fancy2d] CreateSemaphoreExW 或 CreateEventExW 调用失败");
		throw fcyException("f2dAudioBufferDynamic::f2dAudioBufferDynamic", "CreateSemaphoreExW or CreateEventExW Failed.");
	}

	working_thread.Attach(CreateThread(NULL, 0, &WorkingThread, this, 0, NULL));

	action_queue.notifyBufferAvailable(0);
	action_queue.notifyBufferAvailable(1);
}

f2dAudioBufferDynamic::~f2dAudioBufferDynamic()
{
	action_queue.notifyExit();
	WaitForSingleObject(working_thread.Get(), INFINITE);
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
	Action action = {};
	action.type = ActionType::Start;
	action_queue.sendAction(action);
}

void f2dAudioBufferDynamic::Stop()
{
	source_state = State::Stop;
	Action action = {};
	action.type = ActionType::Reset;
	action.action_reset.play = false;
	action_queue.sendAction(action);
}

void f2dAudioBufferDynamic::Pause()
{
	source_state = State::Pause;
	Action action = {};
	action.type = ActionType::Stop;
	action_queue.sendAction(action);
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
	Action action = {};
	action.type = ActionType::SetTime;
	action.action_set_time.time = Time;
	action_queue.sendAction(action);
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
