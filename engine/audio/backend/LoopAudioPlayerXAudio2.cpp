#include "backend/LoopAudioPlayerXAudio2.hpp"
#include "core/Logger.hpp"
#include "win32/base.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	constexpr float s_empty_fft_data[1]{};
}

namespace core {
	// IAudioPlayer

	bool LoopAudioPlayerXAudio2::start() {
		m_is_playing = true;
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->Start(), "IXAudio2SourceVoice::Start"sv);
	}
	bool LoopAudioPlayerXAudio2::stop() {
		m_is_playing = false;
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->Stop(), "IXAudio2SourceVoice::Stop"sv);
	}
	bool LoopAudioPlayerXAudio2::reset() {
		m_is_playing = false;
		if (m_voice == nullptr) {
			return true;
		}
		if (!win32::check_hresult_as_boolean(m_voice->Stop(), "IXAudio2SourceVoice::Stop"sv)) {
			return false;
		}
		if (!win32::check_hresult_as_boolean(m_voice->FlushSourceBuffers(), "IXAudio2SourceVoice::FlushSourceBuffers"sv)) {
			return false;
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

	bool LoopAudioPlayerXAudio2::isPlaying() {
		if (m_voice == nullptr) {
			return m_is_playing;
		}
		XAUDIO2_VOICE_STATE state{};
		m_voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		return m_is_playing && state.BuffersQueued > 0;
	}

	double LoopAudioPlayerXAudio2::getTotalTime() { assert(false); return 0.0; }
	double LoopAudioPlayerXAudio2::getTime() { assert(false); return 0.0; }
	bool LoopAudioPlayerXAudio2::setTime(double const time) {
		m_start_time = time;
		auto const start_sample = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_start_time);
		assert(start_sample <= m_total_frame);
		return start_sample <= m_total_frame;
	}
	bool LoopAudioPlayerXAudio2::setLoop(bool const enable, double const start_pos, double const length) {
		m_loop = enable;
		m_loop_start = start_pos;
		m_loop_length = length;
		auto const loop_start_sample = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_loop_start);
		auto const loop_range_sample_count = static_cast<uint32_t>(static_cast<double>(m_sample_rate) * m_loop_length);
		assert((loop_start_sample + loop_range_sample_count) <= m_total_frame);
		return (loop_start_sample + loop_range_sample_count) <= m_total_frame;
	}

	float LoopAudioPlayerXAudio2::getVolume() {
		return m_volume;
	}
	bool LoopAudioPlayerXAudio2::setVolume(float const volume) {
		m_volume = std::clamp(volume, 0.0f, 1.0f);
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetVolume(m_volume), "IXAudio2SourceVoice::SetVolume"sv);
	}
	float LoopAudioPlayerXAudio2::getBalance() {
		return m_output_balance;
	}
	bool LoopAudioPlayerXAudio2::setBalance(float const v) {
		m_output_balance = std::clamp(v, -1.0f, 1.0f);
		if (m_voice == nullptr) {
			return true;
		}
		auto const result = setOutputBalance(m_voice, m_parent->getChannel(m_mixing_channel), m_output_balance);
		return win32::check_hresult_as_boolean(result, "IXAudio2SourceVoice::SetOutputMatrix"sv);
	}
	float LoopAudioPlayerXAudio2::getSpeed() {
		return m_speed;
	}
	bool LoopAudioPlayerXAudio2::setSpeed(float const speed) {
		m_speed = speed;
		if (m_voice == nullptr) {
			return true;
		}
		return win32::check_hresult_as_boolean(m_voice->SetFrequencyRatio(m_speed), "IXAudio2SourceVoice::SetFrequencyRatio"sv);
	}

	void LoopAudioPlayerXAudio2::updateFFT() { assert(false); }
	uint32_t LoopAudioPlayerXAudio2::getFFTSize() { assert(false); return 0; }
	float const* LoopAudioPlayerXAudio2::getFFT() { assert(false); return s_empty_fft_data; }

	// IXAudio2VoiceCallback

	void WINAPI LoopAudioPlayerXAudio2::OnVoiceError(void* const, HRESULT const error) noexcept {
		std::ignore = win32::check_hresult(error, "IXAudio2VoiceCallback::OnVoiceError"sv);
	}

	// IAudioEndpointEventListener

	void LoopAudioPlayerXAudio2::onAudioEndpointCreate() {
		create();
	}
	void LoopAudioPlayerXAudio2::onAudioEndpointDestroy() {
		destroy();
	}

	// LoopAudioPlayerXAudio2

	LoopAudioPlayerXAudio2::~LoopAudioPlayerXAudio2() {
		if (m_parent) {
			m_parent->removeEventListener(this);
		}
		destroy();
	}

	bool LoopAudioPlayerXAudio2::create() {
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
	bool LoopAudioPlayerXAudio2::create(AudioEndpointXAudio2* const parent, AudioMixingChannel const mixing_channel, IAudioDecoder* const decoder) {
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
	void LoopAudioPlayerXAudio2::destroy() {
		if (m_voice != nullptr) {
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
}
