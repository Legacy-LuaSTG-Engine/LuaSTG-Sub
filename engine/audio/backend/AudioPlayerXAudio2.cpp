#include "backend/AudioPlayerXAudio2.hpp"
#include "core/Logger.hpp"
#include "win32/base.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	constexpr float s_empty_fft_data[1]{};
}

namespace core {
	// IAudioPlayer

	bool AudioPlayerXAudio2::play(double const seconds) {
		if (static_cast<double>(m_sample_rate) * m_start_time > static_cast<double>(m_total_frame)) {
			m_start_time = m_total_seconds;
			if (!m_loop) {
				m_state = AudioPlayerState::stopped;
				return true; // not a fault
			}
		}
		else {
			m_start_time = seconds;
		}
		m_state = AudioPlayerState::playing;
		if (m_voice == nullptr) {
			return true;
		}
		if (!win32::check_hresult_as_boolean(m_voice->Stop(), "IXAudio2SourceVoice::Stop"sv)) {
			return false;
		}
		if (!win32::check_hresult_as_boolean(m_voice->FlushSourceBuffers(), "IXAudio2SourceVoice::FlushSourceBuffers"sv)) {
			return false;
		}
	#ifndef NDEBUG
		XAUDIO2_VOICE_STATE state = {};
		while (true) {
			m_voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			if (state.BuffersQueued < XAUDIO2_MAX_QUEUED_BUFFERS) {
				break;
			}
			Logger::warn("[core] audio buffer queue is full");
		}
	#endif
		if (!submitBuffer()) {
			return false;
		}
		return win32::check_hresult_as_boolean(m_voice->Start(), "IXAudio2SourceVoice::Start"sv);
	}
	bool AudioPlayerXAudio2::pause() {
		if (m_state != AudioPlayerState::playing) {
			return true; // not a fault
		}
		m_state = AudioPlayerState::paused;
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->Stop(), "IXAudio2SourceVoice::Stop"sv);
	}
	bool AudioPlayerXAudio2::resume() {
		if (m_state != AudioPlayerState::paused) {
			return true; // not a fault
		}
		m_state = AudioPlayerState::playing;
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->Start(), "IXAudio2SourceVoice::Start"sv);
	}
	bool AudioPlayerXAudio2::stop() {
		m_state = AudioPlayerState::stopped;
		if (m_voice == nullptr) {
			return true;
		}
		if (!win32::check_hresult_as_boolean(m_voice->Stop(), "IXAudio2SourceVoice::Stop"sv)) {
			return false;
		}
		if (!win32::check_hresult_as_boolean(m_voice->FlushSourceBuffers(), "IXAudio2SourceVoice::FlushSourceBuffers"sv)) {
			return false;
		}
		return true;
	}
	AudioPlayerState AudioPlayerXAudio2::getState() {
		return m_state;
	}

	double AudioPlayerXAudio2::getTotalTime() { assert(false); return 0.0; }
	double AudioPlayerXAudio2::getTime() { assert(false); return 0.0; }
	bool AudioPlayerXAudio2::setLoop(bool const enable, double const start_pos, double const length) {
		m_loop = enable;
		m_loop_start = start_pos;
		m_loop_length = length;
		auto const loop_start_sample = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_loop_start);
		auto const loop_range_sample_count = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_loop_length);
		assert((loop_start_sample + loop_range_sample_count) <= m_total_frame);
		return (loop_start_sample + loop_range_sample_count) <= m_total_frame;
	}

	float AudioPlayerXAudio2::getVolume() {
		return m_volume;
	}
	bool AudioPlayerXAudio2::setVolume(float const volume) {
		m_volume = std::clamp(volume, 0.0f, 1.0f);
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetVolume(m_volume), "IXAudio2SourceVoice::SetVolume"sv);
	}
	float AudioPlayerXAudio2::getBalance() {
		return m_output_balance;
	}
	bool AudioPlayerXAudio2::setBalance(float const v) {
		m_output_balance = std::clamp(v, -1.0f, 1.0f);
		if (m_voice == nullptr) {
			return true;
		}
		auto const result = setOutputBalance(m_voice, m_parent->getChannel(m_mixing_channel), m_output_balance);
		return win32::check_hresult_as_boolean(result, "IXAudio2SourceVoice::SetOutputMatrix"sv);
	}
	float AudioPlayerXAudio2::getSpeed() {
		return m_speed;
	}
	bool AudioPlayerXAudio2::setSpeed(float const speed) {
		m_speed = speed;
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetFrequencyRatio(m_speed), "IXAudio2SourceVoice::SetFrequencyRatio"sv);
	}

	void AudioPlayerXAudio2::updateFFT() { assert(false); }
	uint32_t AudioPlayerXAudio2::getFFTSize() { assert(false); return 0; }
	float const* AudioPlayerXAudio2::getFFT() { assert(false); return s_empty_fft_data; }

	// IXAudio2VoiceCallback

	void WINAPI AudioPlayerXAudio2::OnStreamEnd() noexcept {
		m_state = AudioPlayerState::stopped;
	}
	void WINAPI AudioPlayerXAudio2::OnVoiceError(void* const, HRESULT const error) noexcept {
		std::ignore = win32::check_hresult(error, "IXAudio2VoiceCallback::OnVoiceError"sv);
	}

	// IAudioEndpointEventListener

	void AudioPlayerXAudio2::onAudioEndpointCreate() {
		create();
	}
	void AudioPlayerXAudio2::onAudioEndpointDestroy() {
		destroy();
	}

	// AudioPlayerXAudio2

	AudioPlayerXAudio2::~AudioPlayerXAudio2() {
		if (m_parent) {
			m_parent->removeEventListener(this);
		}
		destroy();
	}

	bool AudioPlayerXAudio2::create() {
		if (m_parent->getDirectChannel() == nullptr) {
			return false;
		}

		HRESULT hr{};

		XAUDIO2_SEND_DESCRIPTOR voice_send{};
		voice_send.pOutputVoice = m_parent->getChannel(m_mixing_channel);
		XAUDIO2_VOICE_SENDS voice_send_list{};
		voice_send_list.SendCount = 1;
		voice_send_list.pSends = &voice_send;

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

		return true;
	}
	bool AudioPlayerXAudio2::create(AudioEndpointXAudio2* const parent, AudioMixingChannel const mixing_channel, IAudioDecoder* const decoder) {
		m_parent = parent;
		m_mixing_channel = mixing_channel;
		m_total_seconds = static_cast<double>(decoder->getFrameCount()) / static_cast<double>(decoder->getSampleRate());

		m_total_frame = decoder->getFrameCount();
		m_sample_rate = decoder->getSampleRate();
		m_frame_size = decoder->getFrameSize();

		m_format.wFormatTag = WAVE_FORMAT_PCM;
		m_format.nChannels = decoder->getChannelCount();
		m_format.nSamplesPerSec = decoder->getSampleRate();
		m_format.nAvgBytesPerSec = decoder->getByteRate();
		m_format.nBlockAlign = decoder->getFrameSize();
		m_format.wBitsPerSample = static_cast<WORD>(decoder->getSampleSize() * 8);

		m_pcm_data.resize(decoder->getFrameCount() * static_cast<uint32_t>(decoder->getFrameSize()));
		uint32_t frames_read = 0;
		if (!decoder->read(decoder->getFrameCount(), m_pcm_data.data(), &frames_read)) {
			return false;
		}

		m_voice_buffer.Flags = XAUDIO2_END_OF_STREAM;
		m_voice_buffer.AudioBytes = frames_read * static_cast<uint32_t>(decoder->getFrameSize());
		m_voice_buffer.pAudioData = m_pcm_data.data();

		std::ignore = create();
		m_parent->addEventListener(this);
		return true;
	}
	void AudioPlayerXAudio2::destroy() {
		if (m_voice != nullptr) {
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
	bool AudioPlayerXAudio2::submitBuffer() {
		if (m_voice == nullptr) {
			return true;
		}

		XAUDIO2_BUFFER buffer{};
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.AudioBytes = static_cast<uint32_t>(m_pcm_data.size());
		buffer.pAudioData = m_pcm_data.data();

		auto const start_sample = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_start_time);
		buffer.PlayBegin = start_sample;
		buffer.PlayLength = m_total_frame - start_sample;

		if (m_loop) {
			auto const loop_start_sample = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_loop_start);
			auto const loop_sample_count = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_loop_length);
			buffer.LoopBegin = loop_start_sample;
			buffer.LoopLength = loop_sample_count;
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		}

		return win32::check_hresult_as_boolean(m_voice->SubmitSourceBuffer(&buffer), "IXAudio2SourceVoice::SubmitSourceBuffer"sv);
	}
}
