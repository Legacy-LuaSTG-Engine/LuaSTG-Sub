#include "Core/Audio/Device_XAUDIO2.hpp"
#include "Core/i18n.hpp"

namespace Core::Audio
{
	template<typename T = IXAudio2Voice>
	inline void SAFE_RELEASE_VOICE(T*& ref_p) { if (ref_p) { ref_p->DestroyVoice(); ref_p = NULL; } }

	inline bool setOutputBalance(IXAudio2SourceVoice* p, float v)
	{
		XAUDIO2_VOICE_DETAILS detail = {};
		p->GetVoiceDetails(&detail);
		float output_matrix_2x2[4] = { 0 };
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
			spdlog::error("[core] 无法识别的的音频声道数量：{}，无法设置音量平衡", detail.InputChannels);
			return false;
		}
		// 这里第一个参数可以为 NULL 的原因是，我们只输出到一个混音节点
		HRESULT hr = gHR = p->SetOutputMatrix(NULL, detail.InputChannels, 2, output_matrix_2x2);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2SourceVoice::SetOutputMatrix ");
			return false;
		}
		return true;
	}

	Shared_XAUDIO2::Shared_XAUDIO2()
		: xa2_master(NULL)
		, xa2_soundeffect(NULL)
		, xa2_music(NULL)
	{
	}
	Shared_XAUDIO2::~Shared_XAUDIO2()
	{
		SAFE_RELEASE_VOICE(xa2_music);
		SAFE_RELEASE_VOICE(xa2_soundeffect);
		SAFE_RELEASE_VOICE(xa2_master);
		xa2_xaudio2.Reset();
	}

	void Device_XAUDIO2::setVolume(float v)
	{
		setMixChannelVolume(MixChannel::Direct, v);
	}
	float Device_XAUDIO2::getVolume()
	{
		return getMixChannelVolume(MixChannel::Direct);
	}
	void Device_XAUDIO2::setMixChannelVolume(MixChannel ch, float v)
	{
		if (!m_shared)
			return;
		IXAudio2Voice* p_voice = NULL;
		switch (ch)
		{
		case MixChannel::Direct: p_voice = m_shared->xa2_master; break;
		case MixChannel::SoundEffect: p_voice = m_shared->xa2_soundeffect; break;
		case MixChannel::Music: p_voice = m_shared->xa2_music; break;
		default: break;
		}
		assert(p_voice);
		if (p_voice)
		{
			HRESULT hr = gHR = p_voice->SetVolume(v);
			if (FAILED(hr))
				i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2Voice::SetVolume");
		}
	}
	float Device_XAUDIO2::getMixChannelVolume(MixChannel ch)
	{
		if (!m_shared)
			return 0.0f;
		IXAudio2Voice* p_voice = NULL;
		switch (ch)
		{
		case MixChannel::Direct: p_voice = m_shared->xa2_master; break;
		case MixChannel::SoundEffect: p_voice = m_shared->xa2_soundeffect; break;
		case MixChannel::Music: p_voice = m_shared->xa2_music; break;
		default: break;
		}
		assert(p_voice);
		float v = 0.0f;
		if (p_voice)
			p_voice->GetVolume(&v);
		return v;
	}

	bool Device_XAUDIO2::createAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player)
	{
		try
		{
			*pp_player = new AudioPlayer_XAUDIO2(this, p_decoder);
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
			*pp_player = nullptr;
			return false;
		}
	}
	bool Device_XAUDIO2::createLoopAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player)
	{
		try
		{
			*pp_player = new LoopAudioPlayer_XAUDIO2(this, p_decoder);
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
			*pp_player = nullptr;
			return false;
		}
	}
	bool Device_XAUDIO2::createStreamAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player)
	{
		try
		{
			*pp_player = new StreamAudioPlayer_XAUDIO2(this, p_decoder);
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
			*pp_player = nullptr;
			return false;
		}
	}

	Device_XAUDIO2::Device_XAUDIO2()
	{
		m_shared.attach(new Shared_XAUDIO2);

		HRESULT hr = 0;

		// 设备

		hr = gHR = XAudio2Create(&m_shared->xa2_xaudio2);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "XAudio2Create");
			throw std::runtime_error("Device_XAUDIO2::Device_XAUDIO2 (1)");
		}
	#ifdef _DEBUG
		XAUDIO2_DEBUG_CONFIGURATION debug_cfg = {
			.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS,
		};
		m_shared->xa2_xaudio2->SetDebugConfiguration(&debug_cfg);
	#endif

		hr = gHR = m_shared->xa2_xaudio2->CreateMasteringVoice(&m_shared->xa2_master);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2::CreateMasteringVoice");
			throw std::runtime_error("Device_XAUDIO2::Device_XAUDIO2 (2)");
		}

		// 混音通道

		XAUDIO2_VOICE_DETAILS voice_info = {};
		m_shared->xa2_master->GetVoiceDetails(&voice_info);

		hr = gHR = m_shared->xa2_xaudio2->CreateSubmixVoice(&m_shared->xa2_soundeffect, 2, voice_info.InputSampleRate); // 固定2声道
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2::CreateSubmixVoice -> #soundeffect");
			throw std::runtime_error("Device_XAUDIO2::Device_XAUDIO2 (3)");
		}

		hr = gHR = m_shared->xa2_xaudio2->CreateSubmixVoice(&m_shared->xa2_music, 2, voice_info.InputSampleRate); // 固定2声道
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2::CreateSubmixVoice -> #music");
			throw std::runtime_error("Device_XAUDIO2::Device_XAUDIO2 (4)");
		}

		// 组装

		XAUDIO2_SEND_DESCRIPTOR voice_send_master = {
			.Flags = 0,
			.pOutputVoice = m_shared->xa2_master,
		};
		XAUDIO2_VOICE_SENDS voice_send_list = {
			.SendCount = 1,
			.pSends = &voice_send_master
		};

		hr = gHR = m_shared->xa2_soundeffect->SetOutputVoices(&voice_send_list);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2SubmixVoice::SetOutputVoices #soundeffect -> #master");
			throw std::runtime_error("Device_XAUDIO2::Device_XAUDIO2 (5)");
		}

		hr = gHR = m_shared->xa2_music->SetOutputVoices(&voice_send_list);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2SubmixVoice::SetOutputVoices #music -> #master");
			throw std::runtime_error("Device_XAUDIO2::Device_XAUDIO2 (6)");
		}
	}
	Device_XAUDIO2::~Device_XAUDIO2()
	{
	}

	bool Device_XAUDIO2::create(Device_XAUDIO2** pp_audio)
	{
		try
		{
			*pp_audio = new Device_XAUDIO2;
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
			*pp_audio = nullptr;
			return false;
		}
	}
}

namespace Core::Audio
{
	void WINAPI AudioPlayer_XAUDIO2::OnVoiceProcessingPassStart(UINT32) {}
	void WINAPI AudioPlayer_XAUDIO2::OnVoiceProcessingPassEnd() {}
	void WINAPI AudioPlayer_XAUDIO2::OnStreamEnd()
	{
		SetEvent(event_end.Get());
	}
	void WINAPI AudioPlayer_XAUDIO2::OnBufferStart(void*) {}
	void WINAPI AudioPlayer_XAUDIO2::OnBufferEnd(void*) {}
	void WINAPI AudioPlayer_XAUDIO2::OnLoopEnd(void*) {}
	void WINAPI AudioPlayer_XAUDIO2::OnVoiceError(void*, HRESULT Error)
	{
		gHR = Error;
		spdlog::error("[core] @IXAudio2VoiceCallback::OnVoiceError");
	}

	bool AudioPlayer_XAUDIO2::start()
	{
		is_playing = true;
		ResetEvent(event_end.Get());
		HRESULT hr = gHR = xa2_source->Start();
		return SUCCEEDED(hr);
	}
	bool AudioPlayer_XAUDIO2::stop()
	{
		is_playing = false;
		HRESULT hr = gHR = xa2_source->Stop();
		return SUCCEEDED(hr);
	}
	bool AudioPlayer_XAUDIO2::reset()
	{
		is_playing = false;
		SetEvent(event_end.Get());
		gHR = xa2_source->Stop();
		gHR = xa2_source->FlushSourceBuffers();
		// TODO: 不应该这样做
		// BEGIN MAGIC
		XAUDIO2_VOICE_STATE state = {};
		do
		{
			xa2_source->GetState(&state);
		} while (state.BuffersQueued >= XAUDIO2_MAX_QUEUED_BUFFERS);
		// END MAGIC
		HRESULT hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer);
		return SUCCEEDED(hr);
	}

	bool AudioPlayer_XAUDIO2::isPlaying()
	{
		DWORD ret = WaitForSingleObjectEx(event_end.Get(), 0, FALSE);
		return is_playing && ret != WAIT_OBJECT_0;
	}

	float AudioPlayer_XAUDIO2::getVolume()
	{
		float v = 0.0;
		xa2_source->GetVolume(&v);
		return v;
	}
	bool AudioPlayer_XAUDIO2::setVolume(float v)
	{
		HRESULT hr = gHR = xa2_source->SetVolume(std::clamp(v, 0.0f, 1.0f));
		return SUCCEEDED(hr);
	}
	float AudioPlayer_XAUDIO2::getBalance()
	{
		return output_balance;
	}
	bool AudioPlayer_XAUDIO2::setBalance(float v)
	{
		return setOutputBalance(xa2_source, v);
	}
	float AudioPlayer_XAUDIO2::getSpeed()
	{
		float v = 0.0f;
		xa2_source->GetFrequencyRatio(&v);
		return v;
	}
	bool AudioPlayer_XAUDIO2::setSpeed(float v)
	{
		HRESULT hr = gHR = xa2_source->SetFrequencyRatio(v);
		return SUCCEEDED(hr);
	}

	AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder)
		: m_device(p_device)
	#ifdef _DEBUG
		, m_decoder(p_decoder)
	#endif
		, xa2_source(NULL)
	{
		auto* p_shared = m_device->GetShared();

		HRESULT hr = 0;

		// 创建音源

		WAVEFORMATEX fmt = {
			.wFormatTag = WAVE_FORMAT_PCM,
			.nChannels = p_decoder->getChannelCount(),
			.nSamplesPerSec = p_decoder->getSampleRate(),
			.nAvgBytesPerSec = p_decoder->getByteRate(),
			.nBlockAlign = p_decoder->getFrameSize(),
			.wBitsPerSample = WORD(p_decoder->getSampleSize() * 8u),
			.cbSize = 0, // 我看还有谁TM写错成 sizeof(WAVEFORMATEX)
		};
		hr = gHR = p_shared->xa2_xaudio2->CreateSourceVoice(&xa2_source, &fmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2::CreateSourceVoice");
			throw std::runtime_error("AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2 (1)");
		}

		event_end.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
		if (!event_end.IsValid())
		{
			gHRLastError;
			i18n_log_error_fmt("[core].system_call_failed_f", "CreateEventExW");
			throw std::runtime_error("AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2 (2)");
		}
		SetEvent(event_end.Get()); // 一开始确实是停止的

		// 绑定

		XAUDIO2_SEND_DESCRIPTOR voice_send_se = {
			.Flags = 0,
			.pOutputVoice = p_shared->xa2_soundeffect,
		};
		XAUDIO2_VOICE_SENDS voice_send_list = {
			.SendCount = 1,
			.pSends = &voice_send_se
		};
		hr = gHR = xa2_source->SetOutputVoices(&voice_send_list);
		if (FAILED(hr))
		{
			SAFE_RELEASE_VOICE(xa2_source);
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2SourceVoice::SetOutputVoices -> #soundeffect");
			throw std::runtime_error("AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2 (3)");
		}

		// 全部解码

		pcm_data.resize(p_decoder->getFrameCount() * (uint32_t)p_decoder->getFrameSize());
		uint32_t frames_read = 0;
		if (!p_decoder->read(p_decoder->getFrameCount(), pcm_data.data(), &frames_read))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDecoder::read -> #ALL");
			throw std::runtime_error("AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2 (4)");
		}
		
		// 填写缓冲区描述符

		xa2_buffer = XAUDIO2_BUFFER{
			.Flags = XAUDIO2_END_OF_STREAM,
			.AudioBytes = frames_read * p_decoder->getFrameSize(),
			.pAudioData = pcm_data.data(),
			.PlayBegin = 0,
			.PlayLength = 0,
			.LoopBegin = 0,
			.LoopLength = 0,
			.LoopCount = 0,
			.pContext = NULL,
		};
	}
	AudioPlayer_XAUDIO2::~AudioPlayer_XAUDIO2()
	{
		if (xa2_source) xa2_source->DestroyVoice(); xa2_source = NULL;
	}
}

namespace Core::Audio
{
	void WINAPI LoopAudioPlayer_XAUDIO2::OnVoiceProcessingPassStart(UINT32) {}
	void WINAPI LoopAudioPlayer_XAUDIO2::OnVoiceProcessingPassEnd() {}
	void WINAPI LoopAudioPlayer_XAUDIO2::OnStreamEnd()
	{
		SetEvent(event_end.Get());
	}
	void WINAPI LoopAudioPlayer_XAUDIO2::OnBufferStart(void*) {}
	void WINAPI LoopAudioPlayer_XAUDIO2::OnBufferEnd(void*) {}
	void WINAPI LoopAudioPlayer_XAUDIO2::OnLoopEnd(void*) {}
	void WINAPI LoopAudioPlayer_XAUDIO2::OnVoiceError(void*, HRESULT Error)
	{
		gHR = Error;
		spdlog::error("[core] @IXAudio2VoiceCallback::OnVoiceError");
	}

	bool LoopAudioPlayer_XAUDIO2::start()
	{
		is_playing = true;
		ResetEvent(event_end.Get());
		HRESULT hr = gHR = xa2_source->Start();
		return SUCCEEDED(hr);
	}
	bool LoopAudioPlayer_XAUDIO2::stop()
	{
		is_playing = false;
		HRESULT hr = gHR = xa2_source->Stop();
		return SUCCEEDED(hr);
	}
	bool LoopAudioPlayer_XAUDIO2::reset()
	{
		// 重置状态和排队的缓冲区

		is_playing = false;
		SetEvent(event_end.Get());
		gHR = xa2_source->Stop();
		gHR = xa2_source->FlushSourceBuffers();
		// TODO: 不应该这样做
		// BEGIN MAGIC
		XAUDIO2_VOICE_STATE state = {};
		do
		{
			xa2_source->GetState(&state);
		} while (state.BuffersQueued >= XAUDIO2_MAX_QUEUED_BUFFERS);
		// END MAGIC
		
		// 提交缓冲区

		if (is_loop)
		{
			HRESULT hr = S_OK;

			// 开始播放的位置
			uint32_t const start_sample = (uint32_t)((double)m_sample_rate * m_start_time);
			// 计算循环区起始位置的采样
			uint32_t const loop_start_sample = (uint32_t)((double)m_sample_rate * loop_start);
			// 计算循环区长度
			uint32_t const loop_range_sample_count = (uint32_t)((double)m_sample_rate * loop_length);

			// 循环节缓冲区
			XAUDIO2_BUFFER xa2_buffer_loop = {
				.Flags = XAUDIO2_END_OF_STREAM,
				.AudioBytes = loop_range_sample_count * (uint32_t)m_frame_size,
				.pAudioData = pcm_data.data() + loop_start_sample * (uint32_t)m_frame_size,
				.PlayBegin = 0,
				.PlayLength = 0,
				.LoopBegin = 0,
				.LoopLength = 0,
				.LoopCount = XAUDIO2_LOOP_INFINITE,
				.pContext = NULL,
			};

			// 分情况来提交缓冲区
			if (start_sample >= m_total_frame)
			{
				// 异常情况，回落到循环节内
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}
			else if (start_sample >= (loop_start_sample + loop_range_sample_count))
			{
				// 开始播放的位置在循环节后面，回落到循环节内
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}
			else if (start_sample > loop_start_sample)
			{
				// 开启播放的位置已经位于循环节内
				// 在循环节之内的部分
				XAUDIO2_BUFFER xa2_buffer_lead = {
					.Flags = 0,
					.AudioBytes = (loop_range_sample_count - (start_sample - loop_start_sample)) * (uint32_t)m_frame_size,
					.pAudioData = pcm_data.data() + start_sample * (uint32_t)m_frame_size,
					.PlayBegin = 0,
					.PlayLength = 0,
					.LoopBegin = 0,
					.LoopLength = 0,
					.LoopCount = 0,
					.pContext = NULL,
				};
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_lead);
				if (FAILED(hr)) return false;
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}
			else if (start_sample == loop_start_sample)
			{
				// 开启播放的位置刚好在循环节上
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}
			else
			{
				// 开启播放的位置位于循环节之前
				// 在循环节之前的部分
				XAUDIO2_BUFFER xa2_buffer_lead = {
					.Flags = 0,
					.AudioBytes = (loop_start_sample - start_sample) * (uint32_t)m_frame_size,
					.pAudioData = pcm_data.data() + start_sample * (uint32_t)m_frame_size,
					.PlayBegin = 0,
					.PlayLength = 0,
					.LoopBegin = 0,
					.LoopLength = 0,
					.LoopCount = 0,
					.pContext = NULL,
				};
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_lead);
				if (FAILED(hr)) return false;
				hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}

			return true;
		}
		else
		{
			// 开始播放的位置
			uint32_t const start_sample = (uint32_t)((double)m_sample_rate * m_start_time);

			XAUDIO2_BUFFER xa2_buffer = {
				.Flags = XAUDIO2_END_OF_STREAM,
				.AudioBytes = (m_total_frame - start_sample) * (uint32_t)m_frame_size,
				.pAudioData = pcm_data.data() + start_sample * (uint32_t)m_frame_size,
				.PlayBegin = 0,
				.PlayLength = 0,
				.LoopBegin = 0,
				.LoopLength = 0,
				.LoopCount = 0,
				.pContext = NULL,
			};
			HRESULT hr = gHR = xa2_source->SubmitSourceBuffer(&xa2_buffer);
			return SUCCEEDED(hr);
		}
	}

	bool LoopAudioPlayer_XAUDIO2::isPlaying()
	{
		DWORD ret = WaitForSingleObjectEx(event_end.Get(), 0, FALSE);
		return is_playing && ret != WAIT_OBJECT_0;
	}

	bool LoopAudioPlayer_XAUDIO2::setTime(double t)
	{
		m_start_time = t;
		uint32_t const start_sample = (uint32_t)((double)m_sample_rate * m_start_time);
		assert(start_sample <= m_total_frame);
		return start_sample <= m_total_frame;
	}
	bool LoopAudioPlayer_XAUDIO2::setLoop(bool enable, double start_pos, double length)
	{
		is_loop = enable;
		loop_start = start_pos;
		loop_length = length;
		uint32_t const loop_start_sample = (uint32_t)((double)m_sample_rate * loop_start);
		uint32_t const loop_range_sample_count = (uint32_t)((double)m_sample_rate * loop_length);
		assert((loop_start_sample + loop_range_sample_count) <= m_total_frame);
		return (loop_start_sample + loop_range_sample_count) <= m_total_frame;
	}

	float LoopAudioPlayer_XAUDIO2::getVolume()
	{
		float v = 0.0;
		xa2_source->GetVolume(&v);
		return v;
	}
	bool LoopAudioPlayer_XAUDIO2::setVolume(float v)
	{
		HRESULT hr = gHR = xa2_source->SetVolume(std::clamp(v, 0.0f, 1.0f));
		return SUCCEEDED(hr);
	}
	float LoopAudioPlayer_XAUDIO2::getBalance()
	{
		return output_balance;
	}
	bool LoopAudioPlayer_XAUDIO2::setBalance(float v)
	{
		return setOutputBalance(xa2_source, v);
	}
	float LoopAudioPlayer_XAUDIO2::getSpeed()
	{
		float v = 0.0f;
		xa2_source->GetFrequencyRatio(&v);
		return v;
	}
	bool LoopAudioPlayer_XAUDIO2::setSpeed(float v)
	{
		HRESULT hr = gHR = xa2_source->SetFrequencyRatio(v);
		return SUCCEEDED(hr);
	}

	LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder)
		: m_device(p_device)
	#ifdef _DEBUG
		, m_decoder(p_decoder)
	#endif
		, xa2_source(NULL)
	{
		auto* p_shared = m_device->GetShared();

		HRESULT hr = 0;

		// 创建音源

		WAVEFORMATEX fmt = {
			.wFormatTag = WAVE_FORMAT_PCM,
			.nChannels = p_decoder->getChannelCount(),
			.nSamplesPerSec = p_decoder->getSampleRate(),
			.nAvgBytesPerSec = p_decoder->getByteRate(),
			.nBlockAlign = p_decoder->getFrameSize(),
			.wBitsPerSample = WORD(p_decoder->getSampleSize() * 8u),
			.cbSize = 0, // 我看还有谁TM写错成 sizeof(WAVEFORMATEX)
		};
		hr = gHR = p_shared->xa2_xaudio2->CreateSourceVoice(&xa2_source, &fmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2::CreateSourceVoice");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (1)");
		}

		event_end.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
		if (!event_end.IsValid())
		{
			gHRLastError;
			i18n_log_error_fmt("[core].system_call_failed_f", "CreateEventExW");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (2)");
		}
		SetEvent(event_end.Get()); // 一开始确实是停止的

		// 绑定

		XAUDIO2_SEND_DESCRIPTOR voice_send_se = {
			.Flags = 0,
			.pOutputVoice = p_shared->xa2_music,
		};
		XAUDIO2_VOICE_SENDS voice_send_list = {
			.SendCount = 1,
			.pSends = &voice_send_se
		};
		hr = gHR = xa2_source->SetOutputVoices(&voice_send_list);
		if (FAILED(hr))
		{
			SAFE_RELEASE_VOICE(xa2_source);
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2SourceVoice::SetOutputVoices -> #soundeffect");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (3)");
		}

		// 全部解码

		if (!p_decoder->seek(0))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDecoder::seek -> #0");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (5)");
		}
		pcm_data.resize(p_decoder->getFrameCount() * p_decoder->getFrameSize());
		uint32_t frames_read = 0;
		if (!p_decoder->read(p_decoder->getFrameCount(), pcm_data.data(), &frames_read))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IDecoder::read -> #ALL");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (4)");
		}
		m_total_frame = frames_read;
		m_frame_size = p_decoder->getFrameSize();
		m_sample_rate = p_decoder->getSampleRate();
	}
	LoopAudioPlayer_XAUDIO2::~LoopAudioPlayer_XAUDIO2()
	{
		if (xa2_source) xa2_source->DestroyVoice(); xa2_source = NULL;
	}
}

namespace Core::Audio
{
	bool StreamAudioPlayer_XAUDIO2::ActionQueue::createObjects()
	{
		semaphore_space = CreateSemaphoreExW(NULL, 64, 64, NULL, 0, SEMAPHORE_ALL_ACCESS);
		semaphore_data = CreateSemaphoreExW(NULL, 0, 64, NULL, 0, SEMAPHORE_ALL_ACCESS);
		if (semaphore_space == NULL || semaphore_data == NULL) return false;
		event_exit = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		event_buffer[0] = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		event_buffer[1] = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		if (event_exit == NULL || event_buffer[0] == NULL || event_buffer[1] == NULL) return false;
		return true;
	}

	void StreamAudioPlayer_XAUDIO2::ActionQueue::notifyExit()
	{
		SetEvent(event_exit);
	}
	void StreamAudioPlayer_XAUDIO2::ActionQueue::notifyBufferAvailable(size_t i)
	{
		SetEvent(event_buffer[i]);
	}
	void StreamAudioPlayer_XAUDIO2::ActionQueue::sendAction(Action const& v)
	{
		WaitForSingleObject(semaphore_space, INFINITE); // 减少剩余空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行
		data[writer_index] = v;
		writer_index = (writer_index + 1) % size;
		ReleaseSemaphore(semaphore_data, 1, NULL); // 增加已用空间的信号量
	}
	void StreamAudioPlayer_XAUDIO2::ActionQueue::reciveAction(Action& v)
	{
		HANDLE objects[4] = { event_exit, event_buffer[0], event_buffer[1], semaphore_data };
		switch (WaitForMultipleObjects(4, objects, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0 + 0:
			v.type = ActionType::Exit; // 特殊处理，且不重置 event
			break;
		case WAIT_OBJECT_0 + 1:
			ResetEvent(event_buffer[0]);
			v.type = ActionType::BufferAvailable; // 特殊处理
			v.action_buffer_available.index = 0;
			break;
		case WAIT_OBJECT_0 + 2:
			ResetEvent(event_buffer[1]);
			v.type = ActionType::BufferAvailable; // 特殊处理
			v.action_buffer_available.index = 1;
			break;
		case WAIT_OBJECT_0 + 3:
			//WaitForSingleObject(semaphore_data, INFINITE); // 减少已用空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行
			v = data[reader_index];
			reader_index = (reader_index + 1) % size;
			ReleaseSemaphore(semaphore_space, 1, NULL); // 增加剩余空间的信号量
			break;
		default:
			v.type = ActionType::Exit; // 出错了，快给爷退出
			break;
		}
	}

	StreamAudioPlayer_XAUDIO2::ActionQueue::ActionQueue()
	{
	}
	StreamAudioPlayer_XAUDIO2::ActionQueue::~ActionQueue()
	{
		writer_index = 0;
		reader_index = 0;
		if (semaphore_space) CloseHandle(semaphore_space); semaphore_space = NULL;
		if (semaphore_data) CloseHandle(semaphore_data); semaphore_data = NULL;
		if (event_exit) CloseHandle(event_exit); event_exit = NULL;
		if (event_buffer[0]) CloseHandle(event_buffer[0]); event_buffer[0] = NULL;
		if (event_buffer[1]) CloseHandle(event_buffer[1]); event_buffer[1] = NULL;
	}

	void WINAPI StreamAudioPlayer_XAUDIO2::OnVoiceProcessingPassStart(UINT32)
	{
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnVoiceProcessingPassEnd()
	{
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnStreamEnd()
	{
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnBufferStart(void*)
	{
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnBufferEnd(void* pBufferContext)
	{
		action_queue.notifyBufferAvailable((size_t)pBufferContext);
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnLoopEnd(void*)
	{
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnVoiceError(void*, HRESULT Error)
	{
		gHR = Error;
		spdlog::error("[core] @IXAudio2VoiceCallback::OnVoiceError");
	}

	DWORD WINAPI StreamAudioPlayer_XAUDIO2::WorkingThread(LPVOID lpThreadParameter)
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
		StreamAudioPlayer_XAUDIO2* self = (StreamAudioPlayer_XAUDIO2*)lpThreadParameter;

		IDecoder* decoder = self->m_decoder.get();
		uint32_t const buffer_bytes = (uint32_t)self->raw_buffer.size() / 2;
		uint32_t const buffer_frames = buffer_bytes / decoder->getFrameSize();
		uint8_t* buffer[2] = { self->raw_buffer.data(), self->raw_buffer.data() + buffer_bytes };

		XAUDIO2_BUFFER xa2_buffer[2] = {
			XAUDIO2_BUFFER {
				.Flags = 0,
				.AudioBytes = buffer_bytes,
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
				.AudioBytes = buffer_bytes,
				.pAudioData = buffer[1],
				.PlayBegin = 0,
				.PlayLength = 0,
				.LoopBegin = 0,
				.LoopLength = 0,
				.LoopCount = 0,
				.pContext = (void*)(size_t)1,
			},
		};
		double buffer_add_time[2] = { 0.0, 0.0 };
		double buffer_set_time[2] = { 0.0, 0.0 };

		bool is_running = true;
		Action action = {};
		double start_time = 0.0;
		size_t buffer_index = 0;
		double time_pos = 0.0;
		uint32_t read_frames = 0;
		while (is_running)
		{
			self->action_queue.reciveAction(action);
			switch (action.type)
			{
			default:
			case ActionType::Exit:
				//spdlog::debug("ActionType::Exit");
				is_running = false; // 该滚蛋了
				self->xa2_source->Stop();
				self->xa2_source->FlushSourceBuffers(); // 防止继续使用上面的局部 buffer 导致内存读取错误
				break;
			case ActionType::Stop:
				//spdlog::debug("ActionType::Stop");
				self->xa2_source->Stop();
				break;
			case ActionType::Start:
				//spdlog::debug("ActionType::Start");
				self->xa2_source->Start();
				break;
			case ActionType::Reset:
				//spdlog::debug("ActionType::Reset [{}]", action.action_reset.play ? "X" : "");
				self->xa2_source->Stop();
				self->xa2_source->FlushSourceBuffers();
				self->total_time = 0.0;
				self->current_time = start_time;
				decoder->seekByTime(start_time);
				buffer_add_time[0] = buffer_add_time[1] = 0.0;
				buffer_set_time[0] = buffer_set_time[1] = start_time;
				if (action.action_reset.play)
				{
					self->xa2_source->Start();
				}
				break;
			case ActionType::SetTime:
				//spdlog::debug("ActionType::SetTime ({})", action.action_set_time.time);
				start_time = action.action_set_time.time;
				self->xa2_source->FlushSourceBuffers();
				self->total_time = 0.0;
				self->current_time = start_time;
				decoder->seekByTime(start_time);
				buffer_add_time[0] = buffer_add_time[1] = 0.0;
				buffer_set_time[0] = buffer_set_time[1] = start_time;
				break;
			case ActionType::BufferAvailable:
				//spdlog::debug("[Player] BufferAvailable [{}]", action.action_buffer_available.index);
				buffer_index = action.action_buffer_available.index;
				self->total_time += buffer_add_time[buffer_index];
				self->current_time = buffer_set_time[buffer_index];
				decoder->read(buffer_frames, buffer[buffer_index], &read_frames);
				decoder->tellAsTime(&time_pos);
				xa2_buffer[buffer_index].AudioBytes = read_frames * decoder->getFrameSize();
				self->xa2_source->SubmitSourceBuffer(&xa2_buffer[buffer_index]);
				buffer_add_time[buffer_index] = (double)read_frames / (double)decoder->getSampleRate();
				buffer_set_time[buffer_index] = time_pos;
				self->audio_buffer_index = buffer_index;
				//spdlog::debug("[Player] Commit {} samples", read_frames);
				break;
			}
		}

		return 0;
	}

	bool StreamAudioPlayer_XAUDIO2::start()
	{
		source_state = State::Play;
		Action action = {};
		action.type = ActionType::Start;
		action_queue.sendAction(action);
		return true;
	}
	bool StreamAudioPlayer_XAUDIO2::stop()
	{
		source_state = State::Pause;
		Action action = {};
		action.type = ActionType::Stop;
		action_queue.sendAction(action);
		return true;
	}
	bool StreamAudioPlayer_XAUDIO2::reset()
	{
		source_state = State::Stop;
		Action action = {};
		action.type = ActionType::Reset;
		action.action_reset.play = false;
		action_queue.sendAction(action);
		return true;
	}

	bool StreamAudioPlayer_XAUDIO2::isPlaying()
	{
		return source_state == State::Play;
	}

	double StreamAudioPlayer_XAUDIO2::getTotalTime()
	{
		return total_time;
	}
	double StreamAudioPlayer_XAUDIO2::getTime()
	{
		return current_time;
	}
	bool StreamAudioPlayer_XAUDIO2::setTime(double t)
	{
		Action action = {};
		action.type = ActionType::SetTime;
		action.action_set_time.time = t;
		action_queue.sendAction(action);
		return true;
	}

	float StreamAudioPlayer_XAUDIO2::getVolume()
	{
		float v = 0.0f;
		xa2_source->GetVolume(&v);
		return v;
	}
	bool StreamAudioPlayer_XAUDIO2::setVolume(float v)
	{
		HRESULT hr = gHR = xa2_source->SetVolume(std::clamp(v, 0.0f, 1.0f));
		return SUCCEEDED(hr);
	}
	float StreamAudioPlayer_XAUDIO2::getBalance()
	{
		return output_balance;
	}
	bool StreamAudioPlayer_XAUDIO2::setBalance(float v)
	{
		return setOutputBalance(xa2_source, v);
	}
	float StreamAudioPlayer_XAUDIO2::getSpeed()
	{
		float v = 0.0f;
		xa2_source->GetFrequencyRatio(&v);
		return v;
	}
	bool StreamAudioPlayer_XAUDIO2::setSpeed(float v)
	{
		HRESULT hr = gHR = xa2_source->SetFrequencyRatio(v);
		return SUCCEEDED(hr);
	}

	StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder)
		: m_device(p_device)
		, m_decoder(p_decoder)
		, xa2_source(NULL)
	{
		auto* p_shared = m_device->GetShared();

		HRESULT hr = 0;

		// 创建音源

		WAVEFORMATEX fmt = {
			.wFormatTag = WAVE_FORMAT_PCM,
			.nChannels = p_decoder->getChannelCount(),
			.nSamplesPerSec = p_decoder->getSampleRate(),
			.nAvgBytesPerSec = p_decoder->getByteRate(),
			.nBlockAlign = p_decoder->getFrameSize(),
			.wBitsPerSample = WORD(p_decoder->getSampleSize() * 8u),
			.cbSize = 0, // 我看还有谁TM写错成 sizeof(WAVEFORMATEX)
		};
		hr = gHR = p_shared->xa2_xaudio2->CreateSourceVoice(&xa2_source, &fmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
		if (FAILED(hr))
		{
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2::CreateSourceVoice");
			throw std::runtime_error("StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2 (1)");
		}

		if (!action_queue.createObjects())
		{
			spdlog::error("[fancy2d] CreateSemaphoreExW 或 CreateEventExW 调用失败");
			throw std::runtime_error("StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2 (2)");
		}

		// 绑定

		XAUDIO2_SEND_DESCRIPTOR voice_send_music = {
			.Flags = 0,
			.pOutputVoice = p_shared->xa2_music,
		};
		XAUDIO2_VOICE_SENDS voice_send_list = {
			.SendCount = 1,
			.pSends = &voice_send_music
		};
		hr = gHR = xa2_source->SetOutputVoices(&voice_send_list);
		if (FAILED(hr))
		{
			SAFE_RELEASE_VOICE(xa2_source);
			i18n_log_error_fmt("[core].system_call_failed_f", "IXAudio2SourceVoice::SetOutputVoices -> #soundeffect");
			throw std::runtime_error("StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2 (3)");
		}

		// 计算解码数据

		size_t buffer_bytes = m_decoder->getFrameSize() * 2048;
		raw_buffer.resize(buffer_bytes * 2);
		p_audio_buffer[0] = raw_buffer.data();
		p_audio_buffer[1] = raw_buffer.data() + buffer_bytes;
		working_thread.Attach(CreateThread(NULL, 0, &WorkingThread, this, 0, NULL));
		if (!working_thread.IsValid())
		{
			gHRLastError;
			i18n_log_error_fmt("[core].system_call_failed_f", "CreateThread");
			throw std::runtime_error("StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2 (4)");
		}

		// 预先填充

		action_queue.notifyBufferAvailable(0);
		action_queue.notifyBufferAvailable(1);
	}
	StreamAudioPlayer_XAUDIO2::~StreamAudioPlayer_XAUDIO2()
	{
		action_queue.notifyExit();
		WaitForSingleObject(working_thread.Get(), INFINITE);
		SAFE_RELEASE_VOICE(xa2_source);
	}
}

// 高科技
#include "xmath/XFFT.h"

namespace Core::Audio
{
	void StreamAudioPlayer_XAUDIO2::updateFFT()
	{
		constexpr size_t sample_count = 512;
		// 第一步，填充音频数据
		if (fft_wave_data.size() != sample_count)
		{
			fft_wave_data.resize(sample_count);
		}
		std::memset(fft_wave_data.data(), 0, fft_wave_data.size() * sizeof(float));
		if (true)
		{
			uint8_t* p_data = p_audio_buffer[(audio_buffer_index + 1) % 2];
			if (m_decoder->getSampleSize() == 2)
			{
				int16_t* p_pcm = (int16_t*)p_data;
				size_t pitch = m_decoder->getChannelCount();
				if (!(pitch == 1 || pitch == 2))
				{
					return; // 没法处理的声道数
				}
				for (size_t i = 0; i < sample_count; i += 1)
				{
					fft_wave_data[i] = (float)(*p_pcm) / (float)(-(INT16_MIN));
					p_pcm += pitch;
				}
			}
			else if (m_decoder->getSampleSize() == 1)
			{
				int8_t* p_pcm = (int8_t*)p_data;
				size_t pitch = m_decoder->getChannelCount();
				if (!(pitch == 1 || pitch == 2))
				{
					return; // 没法处理的声道数
				}
				for (size_t i = 0; i < sample_count; i += 1)
				{
					fft_wave_data[i] = (float)(*p_pcm) / (float)(-(INT8_MIN));
					p_pcm += pitch;
				}
			}
			else
			{
				return; // 没法处理的位深度
			}
		}
		// 第二步，获得采样窗
		if (fft_window.size() != sample_count)
		{
			fft_window.resize(sample_count);
			xmath::fft::getWindow(fft_window.size(), fft_window.data());
		}
		// 第三步，应用采样窗
		for (size_t i = 0; i < sample_count; i += 1)
		{
			fft_wave_data[i] *= fft_window[i];
		}
		// 第四步，申请 FFT 计算空间
		const size_t fft_data_size = xmath::fft::getNeededWorksetSize(fft_wave_data.size());
		const size_t fft_data_float_size = (fft_data_size / sizeof(float)) + 1;
		if (fft_data.size() != fft_data_float_size)
		{
			fft_data.resize(fft_data_float_size);
		}
		if (fft_complex_output.size() != (fft_wave_data.size() * 2))
		{
			fft_complex_output.resize(fft_wave_data.size() * 2);
		}
		if (fft_output.size() != (sample_count / 2))
		{
			fft_output.resize(sample_count / 2);
		}
		// 第五步，可以计算 FFT 了
		xmath::fft::fft(fft_wave_data.size(), fft_data.data(), fft_wave_data.data(), fft_complex_output.data(), fft_output.data());
		// 我先打个断点在这
		std::ignore = nullptr;
	}
	uint32_t StreamAudioPlayer_XAUDIO2::getFFTSize()
	{
		return (uint32_t)fft_output.size();
	}
	float* StreamAudioPlayer_XAUDIO2::getFFT()
	{
		return fft_output.data();
	}
}
