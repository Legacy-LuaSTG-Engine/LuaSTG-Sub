#include "backend/StreamAudioPlayerXAudio2.hpp"
#include "core/Logger.hpp"
#include "win32/base.hpp"
#include <ranges>
#include <algorithm>

using std::string_view_literals::operator""sv;

namespace core {
	void StreamAudioPlayerXAudio2::ActionQueue::notifyExit() {
		m_event_exit.store(true);
	}
	void StreamAudioPlayerXAudio2::ActionQueue::notifyBufferAvailable(size_t const i) {
		Action action{};
		action.type = ActionType::buffer_available;
		action.buffer_available_index = static_cast<uint8_t>(i);
		sendAction(action);
	}
	void StreamAudioPlayerXAudio2::ActionQueue::sendAction(Action const& v) {
		if (!m_semaphore_space.try_acquire_for(std::chrono::seconds(1))) {
			Logger::warn("[core] audio decode thread is blocking");
			return;
		}
		m_data[m_writer_index.fetch_add(1) % m_data.size()] = v;
		m_semaphore_data.release();
	}
	void StreamAudioPlayerXAudio2::ActionQueue::receiveAction(Action& v) {
		for (;;) {
			if (m_event_exit.load()) {
				v.type = ActionType::exit;
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

	bool StreamAudioPlayerXAudio2::play(double const seconds) {
		m_state = AudioPlayerState::playing;
		Action action{};
		action.type = ActionType::play;
		action.start = seconds;
		m_action_queue.sendAction(action);
		return true;
	}
	bool StreamAudioPlayerXAudio2::pause() {
		if (m_state != AudioPlayerState::playing) {
			return true; // not a fault
		}
		m_state = AudioPlayerState::paused;
		Action action{};
		action.type = ActionType::pause;
		m_action_queue.sendAction(action);
		return true;
	}
	bool StreamAudioPlayerXAudio2::resume() {
		if (m_state != AudioPlayerState::paused) {
			return true; // not a fault
		}
		m_state = AudioPlayerState::playing;
		Action action{};
		action.type = ActionType::resume;
		m_action_queue.sendAction(action);
		return true;
	}
	bool StreamAudioPlayerXAudio2::stop() {
		m_state = AudioPlayerState::stopped;
		Action action{};
		action.type = ActionType::stop;
		m_action_queue.sendAction(action);
		return true;
	}
	AudioPlayerState StreamAudioPlayerXAudio2::getState() {
		return m_state;
	}

	double StreamAudioPlayerXAudio2::getTotalTime() {
		return m_played_seconds;
	}
	double StreamAudioPlayerXAudio2::getTime() {
		return m_current_seconds;
	}
	bool StreamAudioPlayerXAudio2::setLoop(bool const enable, double const start_pos, double const length) {
		Action action{};
		action.type = ActionType::update_loop;
		action.buffer_available_index = enable;
		action.start = start_pos;
		action.length = length;
		m_action_queue.sendAction(action);
		return true;
	}

	float StreamAudioPlayerXAudio2::getVolume() {
		return m_volume;
	}
	bool StreamAudioPlayerXAudio2::setVolume(float const volume) {
		m_volume = std::clamp(volume, 0.0f, 1.0f);
		std::unique_lock lock(m_voice_lock);
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetVolume(m_volume), "IXAudio2SourceVoice::SetVolume"sv);
	}
	float StreamAudioPlayerXAudio2::getBalance() {
		return m_output_balance;
	}
	bool StreamAudioPlayerXAudio2::setBalance(float const v) {
		m_output_balance = std::clamp(v, -1.0f, 1.0f);
		std::unique_lock lock(m_voice_lock);
		if (m_voice == nullptr) {
			return true;
		}
		auto const result = setOutputBalance(m_voice, m_parent->getChannel(m_mixing_channel), m_output_balance);
		return win32::check_hresult_as_boolean(result, "IXAudio2SourceVoice::SetOutputMatrix"sv);
	}
	float StreamAudioPlayerXAudio2::getSpeed() {
		return m_speed;
	}
	bool StreamAudioPlayerXAudio2::setSpeed(float const speed) {
		m_speed = speed;
		std::unique_lock lock(m_voice_lock);
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetFrequencyRatio(m_speed), "IXAudio2SourceVoice::SetFrequencyRatio"sv);
	}

	// IXAudio2VoiceCallback

	void WINAPI StreamAudioPlayerXAudio2::OnBufferEnd(void* const buffer_context) noexcept {
		m_action_queue.notifyBufferAvailable(reinterpret_cast<size_t>(buffer_context));
	}
	void WINAPI StreamAudioPlayerXAudio2::OnVoiceError(void* const, HRESULT const error) noexcept {
		std::ignore = win32::check_hresult(error, "IXAudio2VoiceCallback::OnVoiceError"sv);
	}

	// IAudioEndpointEventListener

	void StreamAudioPlayerXAudio2::onAudioEndpointCreate() {
		create();
	}
	void StreamAudioPlayerXAudio2::onAudioEndpointDestroy() {
		destroy();
	}

	// StreamAudioPlayerXAudio2

	StreamAudioPlayerXAudio2::~StreamAudioPlayerXAudio2() {
		if (m_parent) {
			m_parent->removeEventListener(this);
		}
		m_action_queue.notifyExit();
		if (m_working_thread.joinable()) {
			m_working_thread.join();
		}
		destroy();
	}

	bool StreamAudioPlayerXAudio2::create() {
		if (m_parent->getDirectChannel() == nullptr) {
			return false;
		}

		XAUDIO2_SEND_DESCRIPTOR voice_send{};
		voice_send.pOutputVoice = m_parent->getChannel(m_mixing_channel);
		XAUDIO2_VOICE_SENDS voice_send_list{};
		voice_send_list.SendCount = 1;
		voice_send_list.pSends = &voice_send;

		// 创建

		{
			HRESULT hr{};
			std::unique_lock lock(m_voice_lock);

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
		}

		// 恢复

		if (m_state == AudioPlayerState::playing) {
			m_action_queue.notifyBufferAvailable(0);
			m_action_queue.notifyBufferAvailable(1);
		}

		return true;
	}
	bool StreamAudioPlayerXAudio2::create(AudioEndpointXAudio2* const parent, AudioMixingChannel const mixing_channel, IAudioDecoder* const decoder) {
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

		m_working_thread = std::thread(&StreamAudioPlayerXAudio2::worker, this);

		// 创建资源

		std::ignore = create();
		m_parent->addEventListener(this);
		return true;
	}
	void StreamAudioPlayerXAudio2::destroy() {
		std::unique_lock lock(m_voice_lock);
		if (m_voice != nullptr) {
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
	void StreamAudioPlayerXAudio2::worker() {
		struct StreamBuffer {
			uint8_t* data{};
			uint32_t size{};
			uint32_t max_sample_count{};
			XAUDIO2_BUFFER info{};
			double seconds{};
			double current_seconds{};
		};

		std::array<StreamBuffer, 2> stream_buffer{};

		stream_buffer[0].data = raw_buffer.data();
		stream_buffer[0].size = static_cast<uint32_t>(raw_buffer.size()) / 2u; // 缓冲区一半
		stream_buffer[0].max_sample_count = stream_buffer[0].size / static_cast<uint32_t>(m_decoder->getFrameSize()); // 可以容纳这么多帧
		stream_buffer[0].info.pAudioData = stream_buffer[0].data;
		stream_buffer[0].info.pContext = reinterpret_cast<void*>(0);

		stream_buffer[1].data = raw_buffer.data() + stream_buffer[0].size; // 偏移到上一个缓冲区的尾部
		stream_buffer[1].size = stream_buffer[0].size; // 相同
		stream_buffer[1].max_sample_count = stream_buffer[0].max_sample_count; // 相同
		stream_buffer[1].info.pAudioData = stream_buffer[1].data;
		stream_buffer[1].info.pContext = reinterpret_cast<void*>(1);

	#ifndef NDEBUG
	#define PLAYER_DECODER_DEBUG
	#endif

		bool running{ true };
		Action action{};

		auto lock_voice = [this](std::function<void()> const& fun) {
			std::unique_lock lock(m_voice_lock);
			if (m_voice != nullptr) {
				fun();
			}
		};

		auto on_exit = [&]() -> void {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::info("[StreamAudioPlayer] exit");
		#endif

			running = false;

			lock_voice([this] {
				win32::check_hresult(m_voice->Stop());
				win32::check_hresult(m_voice->FlushSourceBuffers());
			});
		};

		auto on_buffer_available = [&](size_t const index) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::info("[StreamAudioPlayer] buffer_available({})", index);
		#endif

			if (!running) {
			#ifdef PLAYER_DECODER_DEBUG
				Logger::info("[StreamAudioPlayer] buffer_available({}) but skip because of exit", index);
			#endif
				return;
			}

			auto& buffer = stream_buffer[index];

			// update played seconds and current position in seconds
			m_played_seconds += buffer.seconds;
			m_current_seconds = buffer.current_seconds;

			// prepare
			auto const loop_sample_end = m_loop_sample_start + m_loop_sample_count;
			uint32_t sample_count{};
			if (m_loop_enabled) {
				if (m_current_sample >= loop_sample_end) {
					m_current_sample = m_loop_sample_start;
					sample_count = std::min(m_loop_sample_count, buffer.max_sample_count);
				}
				else {
					sample_count = std::min(loop_sample_end - m_current_sample, buffer.max_sample_count);
				}
			}
			else {
				sample_count = std::min(m_decoder->getFrameCount() - m_current_sample, buffer.max_sample_count);
			}

			// decode
			if (!m_decoder->seek(m_current_sample)) {
				Logger::error("[StreamAudioPlayer] AudioDecoder seek failed");
				return;
			}
			uint32_t read_sample_count{};
			if (!m_decoder->read(sample_count, buffer.data, &read_sample_count)) {
				Logger::error("[StreamAudioPlayer] AudioDecoder read failed");
				return;
			}
			m_current_sample += read_sample_count;
			buffer.seconds = static_cast<double>(read_sample_count) / static_cast<double>(m_decoder->getSampleRate());
			buffer.current_seconds = static_cast<double>(m_current_sample) / static_cast<double>(m_decoder->getSampleRate());
			buffer.info.AudioBytes = read_sample_count * m_decoder->getFrameSize();
			buffer.info.Flags = 0;
			m_fft_buffer_index = index;

			// add end mark
			if (!m_loop_enabled && m_current_sample >= loop_sample_end) {
				buffer.info.Flags = XAUDIO2_END_OF_STREAM;
			}

			// submit
			lock_voice([&] {
				[[maybe_unused]] auto const result = win32::check_hresult_as_boolean(m_voice->SubmitSourceBuffer(&buffer.info));
			#ifdef PLAYER_DECODER_DEBUG
				Logger::info("[StreamAudioPlayer] submit {} samples: {}", read_sample_count, result);
			#endif
			});
		};

		auto on_play = [&](double const seconds) -> void {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::info("[StreamAudioPlayer] play({})", seconds);
		#endif

			lock_voice([this] {
				win32::check_hresult(m_voice->Stop());
			});

			m_current_sample = static_cast<uint32_t>(static_cast<double>(m_decoder->getSampleRate()) * seconds);
			if (!m_decoder->seek(m_current_sample)) {
				Logger::error("[StreamAudioPlayer] AudioDecoder seek failed");
				return;
			}

			m_played_seconds = 0.0;
			m_current_seconds = seconds;
			stream_buffer[0].seconds = stream_buffer[1].seconds = 0.0;
			stream_buffer[0].current_seconds = stream_buffer[1].current_seconds = seconds;

			lock_voice([this] {
				win32::check_hresult(m_voice->FlushSourceBuffers());
				win32::check_hresult(m_voice->Start());
			});

			m_action_queue.notifyBufferAvailable(0);
			m_action_queue.notifyBufferAvailable(1);
		};

		auto on_pause = [&]() -> void {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::info("[StreamAudioPlayer] pause");
		#endif

			lock_voice([this] {
				win32::check_hresult(m_voice->Stop());
			});
		};

		auto on_resume = [&]() -> void {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::info("[StreamAudioPlayer] resume");
		#endif

			lock_voice([this] {
				win32::check_hresult(m_voice->Start());
			});
		};

		auto on_stop = [&]() -> void {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::info("[StreamAudioPlayer] stop");
		#endif

			lock_voice([this] {
				win32::check_hresult(m_voice->Stop());
			});
		};

		auto on_update_loop = [&](bool const enabled, double const start, double const length) -> void {
			if (enabled) {
			#ifdef PLAYER_DECODER_DEBUG
				Logger::info("[StreamAudioPlayer] loop enabled: start {} length {}", start, length);
			#endif

				m_loop_enabled = true;
				m_loop_sample_start = static_cast<uint32_t>(static_cast<double>(m_decoder->getSampleRate()) * start);
				m_loop_sample_start = std::min(m_loop_sample_start, m_decoder->getFrameCount());
				m_loop_sample_count = static_cast<uint32_t>(static_cast<double>(m_decoder->getSampleRate()) * length);
				m_loop_sample_count = std::min(m_loop_sample_count, m_decoder->getFrameCount() - m_loop_sample_start);
			}
			else {
			#ifdef PLAYER_DECODER_DEBUG
				Logger::info("[StreamAudioPlayer] loop disabled");
			#endif

				m_loop_enabled = false;
				m_loop_sample_start = 0;
				m_loop_sample_count = m_decoder->getFrameCount();
			}
		};

		while (running) {
			m_action_queue.receiveAction(action);
			switch (action.type) {
			case ActionType::exit:
				on_exit();
				break;
			case ActionType::buffer_available:
				on_buffer_available(action.buffer_available_index);
				break;
			case ActionType::play:
				on_play(action.start);
				break;
			case ActionType::pause:
				on_pause();
				break;
			case ActionType::resume:
				on_resume();
				break;
			case ActionType::stop:
				on_stop();
				break;
			case ActionType::update_loop:
				on_update_loop(action.buffer_available_index, action.start, action.length);
				break;
			}
		}
	}
}

#include "xmath/XFFT.h"

namespace core {
	void StreamAudioPlayerXAudio2::updateFFT() {
		constexpr size_t sample_count = 512;
		// 第一步，填充音频数据
		if (m_fft_input.size() != sample_count) {
			m_fft_input.resize(sample_count);
		}
		std::ranges::fill(m_fft_input, 0.0f);
		if constexpr (true) {
			uint8_t* p_data = p_audio_buffer[(m_fft_buffer_index + 1) % 2];
			if (m_decoder->getSampleSize() == 2) {
				int16_t* p_pcm = (int16_t*)p_data;
				size_t pitch = m_decoder->getChannelCount();
				if (!(pitch == 1 || pitch == 2)) {
					return; // 没法处理的声道数
				}
				for (size_t i = 0; i < sample_count; i += 1) {
					m_fft_input[i] = (float)(*p_pcm) / (float)(-(INT16_MIN));
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
					m_fft_input[i] = (float)(*p_pcm) / (float)(-(INT8_MIN));
					p_pcm += pitch;
				}
			}
			else {
				return; // 没法处理的位深度
			}
		}
		// 第二步，获得采样窗
		if (m_fft_window.size() != sample_count) {
			m_fft_window.resize(sample_count);
			xmath::fft::getWindow(m_fft_window.size(), m_fft_window.data());
		}
		// 第三步，应用采样窗
		for (size_t i = 0; i < sample_count; i += 1) {
			m_fft_input[i] *= m_fft_window[i];
		}
		// 第四步，申请 FFT 计算空间
		const size_t fft_data_size = xmath::fft::getNeededWorksetSize(m_fft_input.size());
		const size_t fft_data_float_size = (fft_data_size / sizeof(float)) + 1;
		if (m_fft_data.size() != fft_data_float_size) {
			m_fft_data.resize(fft_data_float_size);
		}
		if (m_fft_complex_result.size() != (m_fft_input.size() * 2)) {
			m_fft_complex_result.resize(m_fft_input.size() * 2);
		}
		if (m_fft_result.size() != (sample_count / 2)) {
			m_fft_result.resize(sample_count / 2);
		}
		// 第五步，可以计算 FFT 了
		xmath::fft::fft(m_fft_input.size(), m_fft_data.data(), m_fft_input.data(), m_fft_complex_result.data(), m_fft_result.data());
		// 我先打个断点在这
		std::ignore = nullptr;
	}
	uint32_t StreamAudioPlayerXAudio2::getFFTSize() {
		return static_cast<uint32_t>(m_fft_result.size());
	}
	float const* StreamAudioPlayerXAudio2::getFFT() {
		return m_fft_result.data();
	}
}
