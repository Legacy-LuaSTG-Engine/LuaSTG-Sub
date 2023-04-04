#include "Core/Audio/Device_XAUDIO2.hpp"
#include "Core/i18n.hpp"

static std::array<float, 1> s_empty_fft_data{};

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
			i18n_core_system_call_report_error("IXAudio2SourceVoice::SetOutputMatrix ");
			return false;
		}
		return true;
	}
	inline HRESULT SetOutputBalance(IXAudio2SourceVoice* p, float v)
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
			return E_INVALIDARG;
		}
		// 这里第一个参数可以为 NULL 的原因是，我们只输出到一个混音节点
		HRESULT hr = gHR = p->SetOutputMatrix(NULL, detail.InputChannels, 2, output_matrix_2x2);
		if (FAILED(hr))
		{
			i18n_core_system_call_report_error("IXAudio2SourceVoice::SetOutputMatrix ");
		}
		return hr;
	}

	Shared_XAUDIO2::Shared_XAUDIO2() = default;
	Shared_XAUDIO2::~Shared_XAUDIO2()
	{
		voice_sound_effect = {};
		voice_music = {};
		voice_master = {};
		xaudio2 = {};
	}

	void Device_XAUDIO2::addEventListener(IAudioDeviceEventListener* p_m_listener)
	{
		assert(!m_dispatch_event);
		if (!m_listener.contains(p_m_listener))
		{
			m_listener.insert(p_m_listener);
		}
	}
	void Device_XAUDIO2::removeEventListener(IAudioDeviceEventListener* p_m_listener)
	{
		assert(!m_dispatch_event);
		if (m_listener.contains(p_m_listener))
		{
			m_listener.erase(p_m_listener);
		}
	}
	void Device_XAUDIO2::dispatchEventAudioDeviceCreate()
	{
		m_dispatch_event = true;
		for (auto& v : m_listener)
		{
			v->onAudioDeviceCreate();
		}
		m_dispatch_event = false;
	}
	void Device_XAUDIO2::dispatchEventAudioDeviceDestroy()
	{
		m_dispatch_event = true;
		for (auto& v : m_listener)
		{
			v->onAudioDeviceDestroy();
		}
		m_dispatch_event = false;
	}

	uint32_t Device_XAUDIO2::getAudioDeviceCount(bool refresh)
	{
		if (refresh)
		{
			refreshAudioDeviceList();
		}
		return static_cast<uint32_t>(m_audio_device_list.size());
	}
	std::string_view Device_XAUDIO2::getAudioDeviceName(uint32_t index) const noexcept
	{
		if (!m_audio_device_list.empty() && index < m_audio_device_list.size())
		{
			return m_audio_device_list[index].name;
		}
		return "";
	}
	bool Device_XAUDIO2::setTargetAudioDevice(std::string_view const audio_device_name)
	{
		m_target_audio_device_name = audio_device_name;
		destroyResources();
		return createResources();
	}

	bool Device_XAUDIO2::createResources()
	{
		try
		{
			m_shared.attach(new Shared_XAUDIO2);

			winrt::check_hresult(m_shared->loader.Create(m_shared->xaudio2.put()));

		#ifndef NDEBUG
			XAUDIO2_DEBUG_CONFIGURATION xaudio2_debug{};
			xaudio2_debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
			xaudio2_debug.BreakMask = XAUDIO2_LOG_ERRORS;
			xaudio2_debug.LogThreadID = TRUE;
			xaudio2_debug.LogTiming = TRUE;
			m_shared->xaudio2->SetDebugConfiguration(&xaudio2_debug);
		#endif

			// output

			std::string_view device_id;
			std::string_view device_name;
			for (auto const& v : m_audio_device_list)
			{
				if (v.name == m_target_audio_device_name)
				{
					device_id = v.id;
					device_name = v.name;
					break;
				}
			}

			if (true)
			{
				// fake no device
				//return false;
			}

			if (device_id.empty())
			{
				winrt::check_hresult(m_shared->xaudio2->CreateMasteringVoice(m_shared->voice_master.put()));
			}
			else
			{
				winrt::check_hresult(m_shared->xaudio2->CreateMasteringVoice(
					m_shared->voice_master.put(),
					0U, 0U, 0U,
					winrt::to_hstring(device_id).c_str()));
			}

			// fixed, 2 channel, 44100hz sample rate

			winrt::check_hresult(m_shared->xaudio2->CreateSubmixVoice(m_shared->voice_sound_effect.put(), 2, 44100));
			winrt::check_hresult(m_shared->xaudio2->CreateSubmixVoice(m_shared->voice_music.put(), 2, 44100));
			
			// build graph

			XAUDIO2_SEND_DESCRIPTOR voice_send_master = {};
			voice_send_master.pOutputVoice = m_shared->voice_master.get();
			XAUDIO2_VOICE_SENDS voice_send_list{};
			voice_send_list.SendCount = 1;
			voice_send_list.pSends = &voice_send_master;

			winrt::check_hresult(m_shared->voice_sound_effect->SetOutputVoices(&voice_send_list));
			winrt::check_hresult(m_shared->voice_music->SetOutputVoices(&voice_send_list));

			// update volume
			winrt::check_hresult(m_shared->voice_master->SetVolume(std::clamp(m_volume_direct, 0.0f, 1.0f)));
			winrt::check_hresult(m_shared->voice_sound_effect->SetVolume(std::clamp(m_volume_sound_effect, 0.0f, 1.0f)));
			winrt::check_hresult(m_shared->voice_music->SetVolume(std::clamp(m_volume_music, 0.0f, 1.0f)));

			m_current_audio_device_name = device_name;
			dispatchEventAudioDeviceCreate();

			return true;
		}
		catch (winrt::hresult_error const& e)
		{
			spdlog::error("[core] <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] <std::exception> {}", e.what());
		}

		return false;
	}
	void Device_XAUDIO2::destroyResources()
	{
		dispatchEventAudioDeviceDestroy();
		m_current_audio_device_name.clear();
		m_shared.reset();
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
		switch (ch)
		{
		case MixChannel::Direct:
			m_volume_direct = v;
			break;
		case MixChannel::SoundEffect:
			m_volume_sound_effect = v;
			break;
		case MixChannel::Music:
			m_volume_music = v;
			break;
		default:
			assert(false);
			break;
		}

		if (!m_shared) return;

		IXAudio2Voice* p_voice = NULL;
		switch (ch)
		{
		case MixChannel::Direct:
			p_voice = m_shared->voice_master.get();
			break;
		case MixChannel::SoundEffect:
			p_voice = m_shared->voice_sound_effect.get();
			break;
		case MixChannel::Music:
			p_voice = m_shared->voice_music.get();
			break;
		default:
			assert(false);
			break;
		}
		if (!p_voice)  return;

		HRESULT hr = gHR = p_voice->SetVolume(std::clamp(v, 0.0f, 1.0f));
		if (FAILED(hr))
			i18n_core_system_call_report_error("IXAudio2Voice::SetVolume");
	}
	float Device_XAUDIO2::getMixChannelVolume(MixChannel ch)
	{
		switch (ch)
		{
		case MixChannel::Direct: return m_volume_direct;
		case MixChannel::SoundEffect: return m_volume_sound_effect;
		case MixChannel::Music: return m_volume_music;
		default: assert(false); return 1.0f;
		}
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
		if (!createResources())
		{
			i18n_core_system_call_report_error("Device_XAUDIO2::Device_XAUDIO2 (1)");
			// 无异常
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
	void WINAPI AudioPlayer_XAUDIO2::OnVoiceError(void*, HRESULT Error) noexcept
	{
		gHR = Error;
		spdlog::error("[core] @IXAudio2VoiceCallback::OnVoiceError");
	}

	void AudioPlayer_XAUDIO2::onAudioDeviceCreate()
	{
		createResources();
	}
	void AudioPlayer_XAUDIO2::onAudioDeviceDestroy()
	{
		destoryResources();
	}

	bool AudioPlayer_XAUDIO2::createResources()
	{
		try
		{
			if (!m_device->getShared()) return false; // 共享组件不存在
			if (!m_device->getShared()->xaudio2) return false; // 设备不存在
			if (!m_device->getShared()->voice_master) return false; // 输出设备不存在

			m_shared = m_device->getShared();

			winrt::check_hresult(m_shared->xaudio2->CreateSourceVoice(m_player.put(), &m_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this));

			XAUDIO2_SEND_DESCRIPTOR voice_send{};
			voice_send.pOutputVoice = m_shared->voice_sound_effect.get();
			XAUDIO2_VOICE_SENDS voice_send_list{};
			voice_send_list.SendCount = 1;
			voice_send_list.pSends = &voice_send;

			winrt::check_hresult(m_player->SetOutputVoices(&voice_send_list));

			winrt::check_hresult(m_player->SetVolume(std::clamp(m_volume, 0.0f, 1.0f)));
			winrt::check_hresult(SetOutputBalance(m_player.get(), m_output_balance));
			winrt::check_hresult(m_player->SetFrequencyRatio(m_speed));

			return true;
		}
		catch (winrt::hresult_error const& e)
		{
			spdlog::error("[core] <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] <std::exception> {}", e.what());
		}
		return false;
	}
	void AudioPlayer_XAUDIO2::destoryResources()
	{
		m_player = {};
		m_shared.reset();
	}

	bool AudioPlayer_XAUDIO2::start()
	{
		m_is_playing = true;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->Start();
		return SUCCEEDED(hr);
	}
	bool AudioPlayer_XAUDIO2::stop()
	{
		m_is_playing = false;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->Stop();
		return SUCCEEDED(hr);
	}
	bool AudioPlayer_XAUDIO2::reset()
	{
		m_is_playing = false;
		if (!m_player) return true; // 不可用，但不是错误

		HRESULT hr = S_OK;

		hr = gHR = m_player->Stop();
		if (FAILED(hr)) return false;
		hr = gHR = m_player->FlushSourceBuffers();
		if (FAILED(hr)) return false;

		XAUDIO2_VOICE_STATE state = {};
		while (true)
		{
			m_player->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			if (state.BuffersQueued < XAUDIO2_MAX_QUEUED_BUFFERS)
			{
				break;
			}
			spdlog::warn("[core] audio buffer queue is full");
		}
		
		hr = gHR = m_player->SubmitSourceBuffer(&m_player_buffer);
		if (FAILED(hr)) return false;

		return true;
	}

	bool AudioPlayer_XAUDIO2::isPlaying()
	{
		if (!m_player) return false; // 不可用，但不是错误
		XAUDIO2_VOICE_STATE state = {};
		m_player->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		return m_is_playing && state.BuffersQueued > 0;
	}

	double AudioPlayer_XAUDIO2::getTotalTime() { assert(false); return 0.0; }
	double AudioPlayer_XAUDIO2::getTime() { assert(false); return 0.0; }
	bool AudioPlayer_XAUDIO2::setTime(double) { assert(false); return true; }
	bool AudioPlayer_XAUDIO2::setLoop(bool, double, double) { assert(false); return true; }

	float AudioPlayer_XAUDIO2::getVolume()
	{
		return m_volume;
	}
	bool AudioPlayer_XAUDIO2::setVolume(float v)
	{
		m_volume = v;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->SetVolume(std::clamp(v, 0.0f, 1.0f));
		return SUCCEEDED(hr);
	}
	float AudioPlayer_XAUDIO2::getBalance()
	{
		return m_output_balance;
	}
	bool AudioPlayer_XAUDIO2::setBalance(float v)
	{
		m_output_balance = v;
		if (!m_player) return true; // 不可用，但不是错误
		return setOutputBalance(m_player.get(), v);
	}
	float AudioPlayer_XAUDIO2::getSpeed()
	{
		return m_speed;
	}
	bool AudioPlayer_XAUDIO2::setSpeed(float v)
	{
		m_speed = v;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->SetFrequencyRatio(v);
		return SUCCEEDED(hr);
	}

	void AudioPlayer_XAUDIO2::updateFFT() { assert(false); }
	uint32_t AudioPlayer_XAUDIO2::getFFTSize() { assert(false); return 0; }
	float* AudioPlayer_XAUDIO2::getFFT() { assert(false); return s_empty_fft_data.data(); }

	AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder)
		: m_device(p_device)
	#ifndef NDEBUG
		, m_decoder(p_decoder)
	#endif
	{
		// 填写格式

		m_format.wFormatTag = WAVE_FORMAT_PCM;
		m_format.nChannels = p_decoder->getChannelCount();
		m_format.nSamplesPerSec = p_decoder->getSampleRate();
		m_format.nAvgBytesPerSec = p_decoder->getByteRate();
		m_format.nBlockAlign = p_decoder->getFrameSize();
		m_format.wBitsPerSample = WORD(p_decoder->getSampleSize() * 8u);
		
		// 全部解码

		m_pcm_data.resize(p_decoder->getFrameCount() * (uint32_t)p_decoder->getFrameSize());
		uint32_t frames_read = 0;
		if (!p_decoder->read(p_decoder->getFrameCount(), m_pcm_data.data(), &frames_read))
		{
			i18n_core_system_call_report_error("IDecoder::read -> #ALL");
			throw std::runtime_error("AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2 (4)");
		}
		
		// 填写缓冲区描述符

		m_player_buffer.Flags = XAUDIO2_END_OF_STREAM;
		m_player_buffer.AudioBytes = frames_read * p_decoder->getFrameSize();
		m_player_buffer.pAudioData = m_pcm_data.data();

		// 创建音频

		if (createResources())
		{
			i18n_core_system_call_report_error("AudioPlayer_XAUDIO2::AudioPlayer_XAUDIO2 (5)");
			// 无异常
		}

		// 注册

		m_device->addEventListener(this);
	}
	AudioPlayer_XAUDIO2::~AudioPlayer_XAUDIO2()
	{
		m_device->removeEventListener(this);
		destoryResources();
	}
}

namespace Core::Audio
{
	void WINAPI LoopAudioPlayer_XAUDIO2::OnVoiceError(void*, HRESULT Error) noexcept
	{
		gHR = Error;
		spdlog::error("[core] @IXAudio2VoiceCallback::OnVoiceError");
	}

	void LoopAudioPlayer_XAUDIO2::onAudioDeviceCreate()
	{
		createResources();
	}
	void LoopAudioPlayer_XAUDIO2::onAudioDeviceDestroy()
	{
		destoryResources();
	}

	bool LoopAudioPlayer_XAUDIO2::createResources()
	{
		try
		{
			if (!m_device->getShared()) return false; // 共享组件不存在
			if (!m_device->getShared()->xaudio2) return false; // 设备不存在
			if (!m_device->getShared()->voice_master) return false; // 输出设备不存在

			m_shared = m_device->getShared();

			winrt::check_hresult(m_shared->xaudio2->CreateSourceVoice(m_player.put(), &m_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this));

			XAUDIO2_SEND_DESCRIPTOR voice_send{};
			voice_send.pOutputVoice = m_shared->voice_music.get(); // 音乐通道
			XAUDIO2_VOICE_SENDS voice_send_list{};
			voice_send_list.SendCount = 1;
			voice_send_list.pSends = &voice_send;

			winrt::check_hresult(m_player->SetOutputVoices(&voice_send_list));

			winrt::check_hresult(m_player->SetVolume(std::clamp(m_volume, 0.0f, 1.0f)));
			winrt::check_hresult(SetOutputBalance(m_player.get(), m_output_balance));
			winrt::check_hresult(m_player->SetFrequencyRatio(m_speed));

			return true;
		}
		catch (winrt::hresult_error const& e)
		{
			spdlog::error("[core] <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] <std::exception> {}", e.what());
		}
		return false;
	}
	void LoopAudioPlayer_XAUDIO2::destoryResources()
	{
		m_player = {};
		m_shared.reset();
	}

	bool LoopAudioPlayer_XAUDIO2::start()
	{
		m_is_playing = true;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->Start();
		return SUCCEEDED(hr);
	}
	bool LoopAudioPlayer_XAUDIO2::stop()
	{
		m_is_playing = false;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->Stop();
		return SUCCEEDED(hr);
	}
	bool LoopAudioPlayer_XAUDIO2::reset()
	{
		m_is_playing = false;
		if (!m_player) return true; // 不可用，但不是错误

		// 重置状态和排队的缓冲区

		HRESULT hr = S_OK;

		hr = gHR = m_player->Stop();
		if (FAILED(hr)) return false;
		hr = gHR = m_player->FlushSourceBuffers();
		if (FAILED(hr)) return false;

		XAUDIO2_VOICE_STATE state = {};
		while (true)
		{
			m_player->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			if (state.BuffersQueued < (XAUDIO2_MAX_QUEUED_BUFFERS - 1)) // 可能会一次性提交两个缓冲区
			{
				break;
			}
			spdlog::warn("[core] audio buffer queue is full");
		}
		
		// 提交缓冲区

		if (m_is_loop)
		{
			// 开始播放的位置
			uint32_t const start_sample = (uint32_t)((double)m_sample_rate * m_start_time);
			// 计算循环区起始位置的采样
			uint32_t const loop_start_sample = (uint32_t)((double)m_sample_rate * m_loop_start);
			// 计算循环区长度
			uint32_t const loop_range_sample_count = (uint32_t)((double)m_sample_rate * m_loop_length);

			// 循环节缓冲区
			XAUDIO2_BUFFER xa2_buffer_loop = {
				.Flags = XAUDIO2_END_OF_STREAM,
				.AudioBytes = loop_range_sample_count * (uint32_t)m_frame_size,
				.pAudioData = m_pcm_data.data() + loop_start_sample * (uint32_t)m_frame_size,
				.PlayBegin = 0,
				.PlayLength = 0,
				.LoopBegin = 0,
				.LoopLength = 0,
				.LoopCount = XAUDIO2_LOOP_INFINITE,
				.pContext = NULL,
			};

			// 分情况来提交缓冲区
			bool const out_of_range = (start_sample >= m_total_frame); // 异常情况，超出范围，回落到循环节内
			bool const out_of_loop_range = (start_sample >= (loop_start_sample + loop_range_sample_count)); // 开始播放的位置在循环节后面，回落到循环节内
			bool const at_loop_range_start = (start_sample == loop_start_sample); // 开始播放的位置刚好在循环节上
			if (out_of_range || out_of_loop_range || at_loop_range_start)
			{
				hr = gHR = m_player->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}
			else if (start_sample > loop_start_sample)
			{
				// 开启播放的位置已经位于循环节内
				// 在循环节之内的部分
				XAUDIO2_BUFFER xa2_buffer_lead = {
					.Flags = 0,
					.AudioBytes = (loop_range_sample_count - (start_sample - loop_start_sample)) * (uint32_t)m_frame_size,
					.pAudioData = m_pcm_data.data() + start_sample * (uint32_t)m_frame_size,
					.PlayBegin = 0,
					.PlayLength = 0,
					.LoopBegin = 0,
					.LoopLength = 0,
					.LoopCount = 0,
					.pContext = NULL,
				};
				hr = gHR = m_player->SubmitSourceBuffer(&xa2_buffer_lead);
				if (FAILED(hr)) return false;
				hr = gHR = m_player->SubmitSourceBuffer(&xa2_buffer_loop);
				if (FAILED(hr)) return false;
			}
			else
			{
				// 开启播放的位置位于循环节之前
				// 在循环节之前的部分
				XAUDIO2_BUFFER xa2_buffer_lead = {
					.Flags = 0,
					.AudioBytes = (loop_start_sample - start_sample) * (uint32_t)m_frame_size,
					.pAudioData = m_pcm_data.data() + start_sample * (uint32_t)m_frame_size,
					.PlayBegin = 0,
					.PlayLength = 0,
					.LoopBegin = 0,
					.LoopLength = 0,
					.LoopCount = 0,
					.pContext = NULL,
				};
				hr = gHR = m_player->SubmitSourceBuffer(&xa2_buffer_lead);
				if (FAILED(hr)) return false;
				hr = gHR = m_player->SubmitSourceBuffer(&xa2_buffer_loop);
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
				.pAudioData = m_pcm_data.data() + start_sample * (uint32_t)m_frame_size,
				.PlayBegin = 0,
				.PlayLength = 0,
				.LoopBegin = 0,
				.LoopLength = 0,
				.LoopCount = 0,
				.pContext = NULL,
			};
			hr = gHR = m_player->SubmitSourceBuffer(&xa2_buffer);
			return SUCCEEDED(hr);
		}
	}

	bool LoopAudioPlayer_XAUDIO2::isPlaying()
	{
		if (!m_player) return m_is_playing; // 不可用，但不是错误
		XAUDIO2_VOICE_STATE state = {};
		m_player->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		return m_is_playing && state.BuffersQueued > 0;
	}

	double LoopAudioPlayer_XAUDIO2::getTotalTime() { assert(false); return 0.0; }
	double LoopAudioPlayer_XAUDIO2::getTime() { assert(false); return 0.0; }
	bool LoopAudioPlayer_XAUDIO2::setTime(double t)
	{
		m_start_time = t;
		uint32_t const start_sample = (uint32_t)((double)m_sample_rate * m_start_time);
		assert(start_sample <= m_total_frame);
		return start_sample <= m_total_frame;
	}
	bool LoopAudioPlayer_XAUDIO2::setLoop(bool enable, double start_pos, double length)
	{
		m_is_loop = enable;
		m_loop_start = start_pos;
		m_loop_length = length;
		uint32_t const loop_start_sample = (uint32_t)((double)m_sample_rate * m_loop_start);
		uint32_t const loop_range_sample_count = (uint32_t)((double)m_sample_rate * m_loop_length);
		assert((loop_start_sample + loop_range_sample_count) <= m_total_frame);
		return (loop_start_sample + loop_range_sample_count) <= m_total_frame;
	}

	float LoopAudioPlayer_XAUDIO2::getVolume()
	{
		return m_volume;
	}
	bool LoopAudioPlayer_XAUDIO2::setVolume(float v)
	{
		m_volume = v;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->SetVolume(std::clamp(v, 0.0f, 1.0f));
		return SUCCEEDED(hr);
	}
	float LoopAudioPlayer_XAUDIO2::getBalance()
	{
		return m_output_balance;
	}
	bool LoopAudioPlayer_XAUDIO2::setBalance(float v)
	{
		m_output_balance = v;
		if (!m_player) return true; // 不可用，但不是错误
		return setOutputBalance(m_player.get(), v);
	}
	float LoopAudioPlayer_XAUDIO2::getSpeed()
	{
		return m_speed;
	}
	bool LoopAudioPlayer_XAUDIO2::setSpeed(float v)
	{
		m_speed = v;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->SetFrequencyRatio(v);
		return SUCCEEDED(hr);
	}

	void LoopAudioPlayer_XAUDIO2::updateFFT() { assert(false); }
	uint32_t LoopAudioPlayer_XAUDIO2::getFFTSize() { assert(false); return 0; }
	float* LoopAudioPlayer_XAUDIO2::getFFT() { assert(false); return s_empty_fft_data.data(); }

	LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder)
		: m_device(p_device)
	#ifndef NDEBUG
		, m_decoder(p_decoder)
	#endif
	{
		// 填写格式

		m_format.wFormatTag = WAVE_FORMAT_PCM;
		m_format.nChannels = p_decoder->getChannelCount();
		m_format.nSamplesPerSec = p_decoder->getSampleRate();
		m_format.nAvgBytesPerSec = p_decoder->getByteRate();
		m_format.nBlockAlign = p_decoder->getFrameSize();
		m_format.wBitsPerSample = WORD(p_decoder->getSampleSize() * 8u);
		
		// 全部解码

		if (!p_decoder->seek(0))
		{
			i18n_core_system_call_report_error("IDecoder::seek -> #0");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (3)");
		}
		m_pcm_data.resize(p_decoder->getFrameCount() * p_decoder->getFrameSize());
		uint32_t frames_read = 0;
		if (!p_decoder->read(p_decoder->getFrameCount(), m_pcm_data.data(), &frames_read))
		{
			i18n_core_system_call_report_error("IDecoder::read -> #ALL");
			throw std::runtime_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (4)");
		}
		m_total_frame = frames_read;
		m_frame_size = p_decoder->getFrameSize();
		m_sample_rate = p_decoder->getSampleRate();

		// 创建音频

		if (createResources())
		{
			i18n_core_system_call_report_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (5)");
			// 无异常
		}

		// 注册

		m_device->addEventListener(this);
	}
	LoopAudioPlayer_XAUDIO2::~LoopAudioPlayer_XAUDIO2()
	{
		m_device->removeEventListener(this);
		destoryResources();
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
		DWORD const wait_result = WaitForSingleObject(semaphore_space, 1000); // 减少剩余空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行
		if (wait_result == WAIT_OBJECT_0)
		{
			data[writer_index] = v;
			writer_index = (writer_index + 1) % size;
			ReleaseSemaphore(semaphore_data, 1, NULL); // 增加已用空间的信号量
		}
		else if (wait_result == WAIT_TIMEOUT)
		{
			spdlog::warn("[core] audio decode thread is blocking");
		}
	}
	void StreamAudioPlayer_XAUDIO2::ActionQueue::reciveAction(Action& v)
	{
		HANDLE objects[4] = {
			event_exit,
			event_buffer[0],
			event_buffer[1],
			semaphore_data,
		};
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
			//WaitForSingleObject(semaphore_data, INFINITE); // 减少已用空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行，上面已经调用过
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

	void WINAPI StreamAudioPlayer_XAUDIO2::OnBufferEnd(void* pBufferContext) noexcept
	{
		action_queue.notifyBufferAvailable((size_t)pBufferContext);
	}
	void WINAPI StreamAudioPlayer_XAUDIO2::OnVoiceError(void*, HRESULT Error) noexcept
	{
		gHR = Error;
		spdlog::error("[core] @IXAudio2VoiceCallback::OnVoiceError");
	}

	void StreamAudioPlayer_XAUDIO2::onAudioDeviceCreate()
	{
		createResources();
	}
	void StreamAudioPlayer_XAUDIO2::onAudioDeviceDestroy()
	{
		destoryResources();
	}

	bool StreamAudioPlayer_XAUDIO2::createResources()
	{
		try
		{
			if (!m_device->getShared()) return false; // 共享组件不存在
			if (!m_device->getShared()->xaudio2) return false; // 设备不存在
			if (!m_device->getShared()->voice_master) return false; // 输出设备不存在

			m_shared = m_device->getShared();

			{
				auto lock_scope = m_player_lock.lock();

				winrt::check_hresult(m_shared->xaudio2->CreateSourceVoice(m_player.put(), &m_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this));

				XAUDIO2_SEND_DESCRIPTOR voice_send{};
				voice_send.pOutputVoice = m_shared->voice_music.get(); // 音乐通道
				XAUDIO2_VOICE_SENDS voice_send_list{};
				voice_send_list.SendCount = 1;
				voice_send_list.pSends = &voice_send;

				winrt::check_hresult(m_player->SetOutputVoices(&voice_send_list));

				winrt::check_hresult(m_player->SetVolume(std::clamp(m_volume, 0.0f, 1.0f)));
				winrt::check_hresult(SetOutputBalance(m_player.get(), m_output_balance));
				winrt::check_hresult(m_player->SetFrequencyRatio(m_speed));

				std::ignore = lock_scope;
			}

			// 让所有缓冲区可用

			action_queue.notifyBufferAvailable(0);
			action_queue.notifyBufferAvailable(1);
			if (source_state == State::Play)
			{
				start(); // 恢复播放
			}

			return true;
		}
		catch (winrt::hresult_error const& e)
		{
			spdlog::error("[core] <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] <std::exception> {}", e.what());
		}
		return false;
	}
	void StreamAudioPlayer_XAUDIO2::destoryResources()
	{
		{
			auto lock_scope = m_player_lock.lock();
			m_player = {};
			std::ignore = lock_scope;
		}
		m_shared.reset();
	}

	DWORD WINAPI StreamAudioPlayer_XAUDIO2::WorkingThread(LPVOID lpThreadParameter)
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
		auto* self = (StreamAudioPlayer_XAUDIO2*)lpThreadParameter;

		struct StreamBuffer
		{
			uint8_t* data{};
			uint32_t max_size{};
			uint32_t max_frame{};
			XAUDIO2_BUFFER info{};
			double add_time{};
			double set_time{};
		};

		std::array<StreamBuffer, 2> stream_buffer{};

		stream_buffer[0].data = self->raw_buffer.data();
		stream_buffer[0].max_size = static_cast<uint32_t>(self->raw_buffer.size()) / 2u; // 缓冲区一半
		stream_buffer[0].max_frame = stream_buffer[0].max_size / static_cast<uint32_t>(self->m_decoder->getFrameSize()); // 可以容纳这么多帧
		stream_buffer[0].info.pAudioData = stream_buffer[0].data;
		stream_buffer[0].info.pContext = reinterpret_cast<void*>(0);

		stream_buffer[1].data = self->raw_buffer.data() + stream_buffer[0].max_size; // 偏移到上一个缓冲区的尾部
		stream_buffer[1].max_size = stream_buffer[0].max_size; // 相同
		stream_buffer[1].max_frame = stream_buffer[0].max_frame; // 相同
		stream_buffer[1].info.pAudioData = stream_buffer[1].data;
		stream_buffer[1].info.pContext = reinterpret_cast<void*>(1);

	//#define PLAYER_DECODER_DEBUG

		double start_time = 0.0; // 用于 on_reset, on_set_time

		// 只有音频对象存在时才执行
		auto lock_player_and_do = [&self](std::function<void()> fun)
		{
			auto lock_scope = self->m_player_lock.lock();
			if (self->m_player)
			{
				fun();
			}
			std::ignore = lock_scope;
		};

		auto on_reset = [&] (bool const play)
		{
		#ifdef PLAYER_DECODER_DEBUG
			spdlog::debug("[Player] [ActionType::Reset] ({})", play ? "Start" : "");
		#endif

			// 先停下来，改好解码器起始位置再清空队列
			lock_player_and_do([&self] {
				self->m_player->Stop(); 
			});

			// 配置为上次设置时间的信息
			self->total_time = 0.0;
			self->current_time = start_time;
			self->m_decoder->seekByTime(start_time);
			stream_buffer[0].add_time = stream_buffer[1].add_time = 0.0;
			stream_buffer[0].set_time = stream_buffer[1].set_time = start_time;

			// 清空当前缓冲区队列
			lock_player_and_do([&] {
				self->m_player->FlushSourceBuffers();
				if (play)
				{
					self->m_player->Start();
				}
			});
		};

		auto on_set_time = [&] (double const time)
		{
		#ifdef PLAYER_DECODER_DEBUG
			spdlog::debug("[Player] [ActionType::SetTime] ({}s)", time);
		#endif

			// 配置信息
			start_time = time;
			self->total_time = 0.0; // 这里可以不关心线程访问冲突
			self->current_time = time; // 这里可以不关心线程访问冲突
			self->m_decoder->seekByTime(time);
			stream_buffer[0].add_time = stream_buffer[1].add_time = 0.0;
			stream_buffer[0].set_time = stream_buffer[1].set_time = time;

			// 清空当前缓冲区队列
			lock_player_and_do([&self] {
				self->m_player->FlushSourceBuffers();
			});
		};

		auto on_buffer_available = [&] (size_t const index)
		{
		#ifdef PLAYER_DECODER_DEBUG
			spdlog::debug("[Player] [ActionType::BufferAvailable] ({})", index);
		#endif

			StreamBuffer& buffer = stream_buffer[index];

			// 这个缓冲区播放完后，可以更新当前时间和总播放时间
			self->total_time += buffer.add_time; // 这里可以不关心线程访问冲突
			self->current_time = buffer.set_time; // 这里可以不关心线程访问冲突

			// 解码下一节到缓冲区
			uint32_t read_frames = 0;
			self->m_decoder->read(buffer.max_frame, buffer.data, &read_frames);
			self->audio_buffer_index = index; // 这里可以不关心线程访问冲突

			// 计算当前时间
			double time_pos = 0.0;
			self->m_decoder->tellAsTime(&time_pos);

			// 提交
			buffer.info.AudioBytes = read_frames * self->m_decoder->getFrameSize();
			lock_player_and_do([&] {
				self->m_player->SubmitSourceBuffer(&buffer.info);
			#ifdef PLAYER_DECODER_DEBUG
				spdlog::debug("[Player] [ActionType::BufferAvailable] Submit {} Samples", read_frames);
			#endif
			});

			// 这个缓冲区播放完后要更新的数据
			buffer.add_time = (double)read_frames / (double)self->m_decoder->getSampleRate();
			buffer.set_time = time_pos;
		};

		bool is_running = true;
		Action action = {};
		
		while (is_running)
		{
			self->action_queue.reciveAction(action);
			switch (action.type)
			{
			default:
			case ActionType::Exit:
			#ifdef PLAYER_DECODER_DEBUG
				spdlog::debug("[Player] [ActionType::Exit]");
			#endif
				is_running = false; // 该滚蛋了
				lock_player_and_do([&self] {
					// 先停下，然后清空缓冲区队列，防止继续使用上面的局部 buffer 导致内存读取错误
					self->m_player->Stop(); 
					self->m_player->FlushSourceBuffers();
				});
				break;
			case ActionType::Stop:
			#ifdef PLAYER_DECODER_DEBUG
				spdlog::debug("[Player] [ActionType::Stop]");
			#endif
				lock_player_and_do([&self] {
					self->m_player->Stop();
				});
				break;
			case ActionType::Start:
			#ifdef PLAYER_DECODER_DEBUG
				spdlog::debug("[Player] [ActionType::Start]");
			#endif
				lock_player_and_do([&self] {
					self->m_player->Start();
				});
				break;
			case ActionType::Reset:
				on_reset(action.action_reset.play);
				break;
			case ActionType::SetTime:
				on_set_time(action.action_set_time.time);
				break;
			case ActionType::BufferAvailable:
				on_buffer_available(action.action_buffer_available.index);
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
		return m_volume;
	}
	bool StreamAudioPlayer_XAUDIO2::setVolume(float v)
	{
		m_volume = v;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->SetVolume(std::clamp(v, 0.0f, 1.0f));
		return SUCCEEDED(hr);
	}
	float StreamAudioPlayer_XAUDIO2::getBalance()
	{
		return m_output_balance;
	}
	bool StreamAudioPlayer_XAUDIO2::setBalance(float v)
	{
		m_output_balance = v;
		if (!m_player) return true; // 不可用，但不是错误
		return setOutputBalance(m_player.get(), v);
	}
	float StreamAudioPlayer_XAUDIO2::getSpeed()
	{
		return m_speed;
	}
	bool StreamAudioPlayer_XAUDIO2::setSpeed(float v)
	{
		m_speed = v;
		if (!m_player) return true; // 不可用，但不是错误
		HRESULT hr = gHR = m_player->SetFrequencyRatio(v);
		return SUCCEEDED(hr);
	}

	StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder)
		: m_device(p_device)
		, m_decoder(p_decoder)
	{
		// 动作队列

		if (!action_queue.createObjects())
		{
			spdlog::error("[core] CreateSemaphoreExW 或 CreateEventExW 调用失败");
			throw std::runtime_error("StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2 (2)");
		}

		// 填写格式
		
		m_format.wFormatTag = WAVE_FORMAT_PCM;
		m_format.nChannels = p_decoder->getChannelCount();
		m_format.nSamplesPerSec = p_decoder->getSampleRate();
		m_format.nAvgBytesPerSec = p_decoder->getByteRate();
		m_format.nBlockAlign = p_decoder->getFrameSize();
		m_format.wBitsPerSample = WORD(p_decoder->getSampleSize() * 8u);
		
		// 计算解码数据

		size_t buffer_bytes = m_decoder->getFrameSize() * 2048;
		raw_buffer.resize(buffer_bytes * 2);
		p_audio_buffer[0] = raw_buffer.data();
		p_audio_buffer[1] = raw_buffer.data() + buffer_bytes;

		// 创建音频

		if (createResources())
		{
			i18n_core_system_call_report_error("LoopAudioPlayer_XAUDIO2::LoopAudioPlayer_XAUDIO2 (5)");
			// 无异常
		}

		// 注册

		m_device->addEventListener(this);

		// 启动解码线程

		working_thread.Attach(CreateThread(NULL, 0, &WorkingThread, this, 0, NULL));
		if (!working_thread.IsValid())
		{
			gHRLastError;
			i18n_core_system_call_report_error("CreateThread");
			throw std::runtime_error("StreamAudioPlayer_XAUDIO2::StreamAudioPlayer_XAUDIO2 (4)");
		}
	}
	StreamAudioPlayer_XAUDIO2::~StreamAudioPlayer_XAUDIO2()
	{
		// 先把解码线程停下来
		action_queue.notifyExit();
		WaitForSingleObject(working_thread.Get(), INFINITE);
		// 再销毁
		m_device->removeEventListener(this);
		destoryResources();
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
		std::memset(fft_wave_data.data(), 0, sizeof(float) * fft_wave_data.size());
		if constexpr (true)
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
		return static_cast<uint32_t>(fft_output.size());
	}
	float* StreamAudioPlayer_XAUDIO2::getFFT()
	{
		return fft_output.data();
	}
}
