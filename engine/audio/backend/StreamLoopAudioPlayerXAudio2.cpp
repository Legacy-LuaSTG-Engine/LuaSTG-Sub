#include "backend/StreamLoopAudioPlayerXAudio2.hpp"
#include "core/Logger.hpp"
#include "win32/base.hpp"

using std::string_view_literals::operator""sv;

namespace core {
	void StreamLoopAudioPlayerXAudio2::ActionQueue::notifyExit() {
		m_event_exit.store(true);
	}
	void StreamLoopAudioPlayerXAudio2::ActionQueue::notifyBufferAvailable(size_t const i) {
		m_buffer_available_mask.fetch_or(1 << i);
	}
	void StreamLoopAudioPlayerXAudio2::ActionQueue::sendAction(Action const& v) {
		if (!m_semaphore_space.try_acquire_for(std::chrono::seconds(1))) {
			Logger::warn("[core] audio decode thread is blocking");
			return;
		}
		m_data[m_writer_index.fetch_add(1) % m_data.size()] = v;
		m_semaphore_data.release();
	}
	void StreamLoopAudioPlayerXAudio2::ActionQueue::receiveAction(Action& v) {
		for (;;) {
			if (m_event_exit.load()) {
				v.type = ActionType::Exit;
				return;
			}
			if (auto const mask = m_buffer_available_mask.fetch_add(0x2); (mask & 0x1) == 0x1) {
				v.type = ActionType::BufferAvailable;
				v.action_buffer_available.index = 0;
				return;
			}
			if (auto const mask = m_buffer_available_mask.fetch_add(0x1); (mask & 0x2) == 0x2) {
				v.type = ActionType::BufferAvailable;
				v.action_buffer_available.index = 1;
				return;
			}
			if (m_semaphore_data.try_acquire_for(std::chrono::seconds(1))) {
				v = m_data[m_reader_index.fetch_add(1) % m_data.size()];
				m_semaphore_space.release();
				return;
			}
		}
	}
}

namespace core {
	// IAudioPlayer

	bool StreamLoopAudioPlayerXAudio2::start() {
		source_state = State::Play;
		Action action = {};
		action.type = ActionType::Start;
		action_queue.sendAction(action);
		return true;
	}
	bool StreamLoopAudioPlayerXAudio2::stop() {
		source_state = State::Pause;
		Action action = {};
		action.type = ActionType::Stop;
		action_queue.sendAction(action);
		return true;
	}
	bool StreamLoopAudioPlayerXAudio2::reset() {
		source_state = State::Stop;
		Action action = {};
		action.type = ActionType::Reset;
		action.action_reset.play = false;
		action_queue.sendAction(action);
		return true;
	}

	bool StreamLoopAudioPlayerXAudio2::isPlaying() {
		return source_state == State::Play;
	}

	double StreamLoopAudioPlayerXAudio2::getTotalTime() {
		return total_time;
	}
	double StreamLoopAudioPlayerXAudio2::getTime() {
		return current_time;
	}
	bool StreamLoopAudioPlayerXAudio2::setTime(double const time) {
		Action action = {};
		action.type = ActionType::SetTime;
		action.action_set_time.time = time;
		action_queue.sendAction(action);
		return true;
	}

	float StreamLoopAudioPlayerXAudio2::getVolume() {
		return m_volume;
	}
	bool StreamLoopAudioPlayerXAudio2::setVolume(float const volume) {
		m_volume = std::clamp(volume, 0.0f, 1.0f);
		std::shared_lock lock(m_player_lock);
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetVolume(m_volume), "IXAudio2SourceVoice::SetVolume"sv);
	}
	float StreamLoopAudioPlayerXAudio2::getBalance() {
		return m_output_balance;
	}
	bool StreamLoopAudioPlayerXAudio2::setBalance(float const v) {
		m_output_balance = std::clamp(v, -1.0f, 1.0f);
		std::shared_lock lock(m_player_lock);
		if (m_voice == nullptr) {
			return true;
		}
		auto const result = setOutputBalance(m_voice, m_parent->getChannel(m_mixing_channel), m_output_balance);
		return win32::check_hresult_as_boolean(result, "IXAudio2SourceVoice::SetOutputMatrix"sv);
	}
	float StreamLoopAudioPlayerXAudio2::getSpeed() {
		return m_speed;
	}
	bool StreamLoopAudioPlayerXAudio2::setSpeed(float const speed) {
		m_speed = speed;
		std::shared_lock lock(m_player_lock);
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetFrequencyRatio(m_speed), "IXAudio2SourceVoice::SetFrequencyRatio"sv);
	}

	// IXAudio2VoiceCallback

	void WINAPI StreamLoopAudioPlayerXAudio2::OnBufferEnd(void* const buffer_context) noexcept {
		action_queue.notifyBufferAvailable(reinterpret_cast<size_t>(buffer_context));
	}
	void WINAPI StreamLoopAudioPlayerXAudio2::OnVoiceError(void* const, HRESULT const error) noexcept {
		std::ignore = win32::check_hresult(error, "IXAudio2VoiceCallback::OnVoiceError"sv);
	}

	// IAudioEndpointEventListener

	void StreamLoopAudioPlayerXAudio2::onAudioEndpointCreate() {
		create();
	}
	void StreamLoopAudioPlayerXAudio2::onAudioEndpointDestroy() {
		destroy();
	}

	// StreamLoopAudioPlayerXAudio2

	StreamLoopAudioPlayerXAudio2::~StreamLoopAudioPlayerXAudio2() {
		if (m_parent) {
			m_parent->removeEventListener(this);
		}
		action_queue.notifyExit();
		if (m_working_thread.joinable()) {
			m_working_thread.join();
		}
		destroy();
	}

	bool StreamLoopAudioPlayerXAudio2::create() {
		if (m_parent->getDirectChannel() == nullptr) {
			return false;
		}

		HRESULT hr{};

		XAUDIO2_SEND_DESCRIPTOR voice_send{};
		voice_send.pOutputVoice = m_parent->getChannel(m_mixing_channel);
		XAUDIO2_VOICE_SENDS voice_send_list{};
		voice_send_list.SendCount = 1;
		voice_send_list.pSends = &voice_send;

		// 创建

		std::unique_lock lock(m_player_lock); // mutex

		hr = m_parent->getFactory()->CreateSourceVoice(&m_voice, &m_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this, &voice_send_list);
		if (!win32::check_hresult_as_boolean(hr, "XAudio2::CreateSourceVoice"sv)) {
			return false;
		}

		hr = m_voice->SetVolume(m_volume);
		if (!win32::check_hresult_as_boolean(hr, "IXAudio2SourceVoice::SetVolume"sv)) {
			return false;
		}

		hr = m_voice->SetFrequencyRatio(m_speed);
		if (!win32::check_hresult_as_boolean(hr, "IXAudio2SourceVoice::SetFrequencyRatio"sv)) {
			return false;
		}

		if (!setBalance(m_output_balance)) {
			return false;
		}

		// 恢复

		action_queue.notifyBufferAvailable(0);
		action_queue.notifyBufferAvailable(1);
		if (source_state == State::Play) {
			start(); // 恢复播放
		}

		return true;
	}
	bool StreamLoopAudioPlayerXAudio2::create(AudioEndpointXAudio2* const parent, AudioMixingChannel const mixing_channel, IAudioDecoder* const decoder) {
		m_parent = parent;
		m_mixing_channel = mixing_channel;
		m_decoder = decoder;

		// 填写格式

		m_format.wFormatTag = WAVE_FORMAT_PCM;
		m_format.nChannels = m_decoder->getChannelCount();
		m_format.nSamplesPerSec = m_decoder->getSampleRate();
		m_format.nAvgBytesPerSec = m_decoder->getByteRate();
		m_format.nBlockAlign = m_decoder->getFrameSize();
		m_format.wBitsPerSample = static_cast<uint16_t>(m_decoder->getSampleSize() * 8u);

		// 准备解码缓冲区

		auto const buffer_bytes = static_cast<size_t>(m_decoder->getFrameSize()) * 2048u;
		raw_buffer.resize(buffer_bytes * 2u);
		p_audio_buffer[0] = raw_buffer.data();
		p_audio_buffer[1] = raw_buffer.data() + buffer_bytes;

		// 启动解码线程

		m_working_thread = std::thread(&StreamLoopAudioPlayerXAudio2::worker, this);

		// 创建资源

		std::ignore = create();
		m_parent->addEventListener(this);
		return true;
	}
	void StreamLoopAudioPlayerXAudio2::destroy() {
		std::unique_lock lock(m_player_lock);
		if (m_voice != nullptr) {
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
	void StreamLoopAudioPlayerXAudio2::worker() {
		struct StreamBuffer {
			uint8_t* data{};
			uint32_t max_size{};
			uint32_t max_frame{};
			XAUDIO2_BUFFER info{};
			double add_time{};
			double set_time{};
		};

		std::array<StreamBuffer, 2> stream_buffer{};

		stream_buffer[0].data = raw_buffer.data();
		stream_buffer[0].max_size = static_cast<uint32_t>(raw_buffer.size()) / 2u; // 缓冲区一半
		stream_buffer[0].max_frame = stream_buffer[0].max_size / static_cast<uint32_t>(m_decoder->getFrameSize()); // 可以容纳这么多帧
		stream_buffer[0].info.pAudioData = stream_buffer[0].data;
		stream_buffer[0].info.pContext = reinterpret_cast<void*>(0);

		stream_buffer[1].data = raw_buffer.data() + stream_buffer[0].max_size; // 偏移到上一个缓冲区的尾部
		stream_buffer[1].max_size = stream_buffer[0].max_size; // 相同
		stream_buffer[1].max_frame = stream_buffer[0].max_frame; // 相同
		stream_buffer[1].info.pAudioData = stream_buffer[1].data;
		stream_buffer[1].info.pContext = reinterpret_cast<void*>(1);

		//#define PLAYER_DECODER_DEBUG

		double start_time = 0.0; // 用于 on_reset, on_set_time

		// 只有音频对象存在时才执行
		auto lock_player_and_do = [this](std::function<void()> const& fun) {
			std::shared_lock lock(m_player_lock);
			if (m_voice != nullptr) {
				fun();
			}
		};

		auto on_reset = [&](bool const play) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::debug("[Player] [ActionType::Reset] ({})", play ? "Start" : "");
		#endif

			// 先停下来，改好解码器起始位置再清空队列
			lock_player_and_do([this] {
				m_voice->Stop();
			});

			// 配置为上次设置时间的信息
			total_time = 0.0;
			current_time = start_time;
			m_decoder->seekByTime(start_time);
			stream_buffer[0].add_time = stream_buffer[1].add_time = 0.0;
			stream_buffer[0].set_time = stream_buffer[1].set_time = start_time;

			// 清空当前缓冲区队列
			lock_player_and_do([&] {
				m_voice->FlushSourceBuffers();
				if (play) {
					m_voice->Start();
				}
			});
		};

		auto on_set_time = [&](double const time) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::debug("[Player] [ActionType::SetTime] ({}s)", time);
		#endif

			// 配置信息
			start_time = time;
			total_time = 0.0; // 这里可以不关心线程访问冲突
			current_time = time; // 这里可以不关心线程访问冲突
			m_decoder->seekByTime(time);
			stream_buffer[0].add_time = stream_buffer[1].add_time = 0.0;
			stream_buffer[0].set_time = stream_buffer[1].set_time = time;

			// 清空当前缓冲区队列
			lock_player_and_do([this] {
				m_voice->FlushSourceBuffers();
			});
		};

		auto on_buffer_available = [&](size_t const index) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::debug("[Player] [ActionType::BufferAvailable] ({})", index);
		#endif

			StreamBuffer& buffer = stream_buffer[index];

			// 这个缓冲区播放完后，可以更新当前时间和总播放时间
			total_time += buffer.add_time; // 这里可以不关心线程访问冲突
			current_time = buffer.set_time; // 这里可以不关心线程访问冲突

			// 解码下一节到缓冲区
			uint32_t read_frames = 0;
			m_decoder->read(buffer.max_frame, buffer.data, &read_frames);
			audio_buffer_index = index; // 这里可以不关心线程访问冲突

			// 计算当前时间
			double time_pos = 0.0;
			m_decoder->tellAsTime(&time_pos);

			// 提交
			buffer.info.AudioBytes = read_frames * m_decoder->getFrameSize();
			lock_player_and_do([&] {
				m_voice->SubmitSourceBuffer(&buffer.info);
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::BufferAvailable] Submit {} Samples", read_frames);
			#endif
			});

			// 这个缓冲区播放完后要更新的数据
			buffer.add_time = static_cast<double>(read_frames) / static_cast<double>(m_decoder->getSampleRate());
			buffer.set_time = time_pos;
		};

		bool is_running = true;
		Action action = {};

		while (is_running) {
			action_queue.receiveAction(action);
			switch (action.type) {
			default:
			case ActionType::Exit:
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::Exit]");
			#endif
				is_running = false; // 该滚蛋了
				lock_player_and_do([this] {
					// 先停下，然后清空缓冲区队列，防止继续使用上面的局部 buffer 导致内存读取错误
					m_voice->Stop();
					m_voice->FlushSourceBuffers();
				});
				break;
			case ActionType::Stop:
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::Stop]");
			#endif
				lock_player_and_do([this] {
					m_voice->Stop();
				});
				break;
			case ActionType::Start:
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::Start]");
			#endif
				lock_player_and_do([this] {
					m_voice->Start();
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
	}
}

#include "xmath/XFFT.h"

namespace core {
	void StreamLoopAudioPlayerXAudio2::updateFFT() {
		constexpr size_t sample_count = 512;
		// 第一步，填充音频数据
		if (fft_wave_data.size() != sample_count) {
			fft_wave_data.resize(sample_count);
		}
		std::memset(fft_wave_data.data(), 0, sizeof(float) * fft_wave_data.size());
		if constexpr (true) {
			uint8_t* p_data = p_audio_buffer[(audio_buffer_index + 1) % 2];
			if (m_decoder->getSampleSize() == 2) {
				int16_t* p_pcm = (int16_t*)p_data;
				size_t pitch = m_decoder->getChannelCount();
				if (!(pitch == 1 || pitch == 2)) {
					return; // 没法处理的声道数
				}
				for (size_t i = 0; i < sample_count; i += 1) {
					fft_wave_data[i] = (float)(*p_pcm) / (float)(-(INT16_MIN));
					p_pcm += pitch;
				}
			}
			else if (m_decoder->getSampleSize() == 1) {
				int8_t* p_pcm = (int8_t*)p_data;
				size_t pitch = m_decoder->getChannelCount();
				if (!(pitch == 1 || pitch == 2)) {
					return; // 没法处理的声道数
				}
				for (size_t i = 0; i < sample_count; i += 1) {
					fft_wave_data[i] = (float)(*p_pcm) / (float)(-(INT8_MIN));
					p_pcm += pitch;
				}
			}
			else {
				return; // 没法处理的位深度
			}
		}
		// 第二步，获得采样窗
		if (fft_window.size() != sample_count) {
			fft_window.resize(sample_count);
			xmath::fft::getWindow(fft_window.size(), fft_window.data());
		}
		// 第三步，应用采样窗
		for (size_t i = 0; i < sample_count; i += 1) {
			fft_wave_data[i] *= fft_window[i];
		}
		// 第四步，申请 FFT 计算空间
		const size_t fft_data_size = xmath::fft::getNeededWorksetSize(fft_wave_data.size());
		const size_t fft_data_float_size = (fft_data_size / sizeof(float)) + 1;
		if (fft_data.size() != fft_data_float_size) {
			fft_data.resize(fft_data_float_size);
		}
		if (fft_complex_output.size() != (fft_wave_data.size() * 2)) {
			fft_complex_output.resize(fft_wave_data.size() * 2);
		}
		if (fft_output.size() != (sample_count / 2)) {
			fft_output.resize(sample_count / 2);
		}
		// 第五步，可以计算 FFT 了
		xmath::fft::fft(fft_wave_data.size(), fft_data.data(), fft_wave_data.data(), fft_complex_output.data(), fft_output.data());
		// 我先打个断点在这
		std::ignore = nullptr;
	}
	uint32_t StreamLoopAudioPlayerXAudio2::getFFTSize() {
		return static_cast<uint32_t>(fft_output.size());
	}
	float const* StreamLoopAudioPlayerXAudio2::getFFT() {
		return fft_output.data();
	}
}
