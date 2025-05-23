#include "backend/SimpleAudioPlayerXAudio2.hpp"
#include "core/Logger.hpp"
#include "winrt/base.h"

namespace {
	float s_empty_fft_data[1]{};
}

namespace core {
	// IAudioPlayer

	bool SimpleAudioPlayerXAudio2::start() {
		m_is_playing = true;
		if (m_voice == nullptr) {
			return true;
		}
		try {
			winrt::check_hresult(m_voice->Start());
			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] IXAudio2SourceVoice::Start failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] IXAudio2SourceVoice::Start failed: <std::exception> {}", e.what());
		}
		return false;
	}
	bool SimpleAudioPlayerXAudio2::stop() {
		m_is_playing = false;
		if (m_voice == nullptr) {
			return true;
		}
		try {
			winrt::check_hresult(m_voice->Stop());
			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] IXAudio2SourceVoice::Start failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] IXAudio2SourceVoice::Start failed: <std::exception> {}", e.what());
		}
		return false;
	}
	bool SimpleAudioPlayerXAudio2::reset() {
		m_is_playing = false;
		if (m_voice == nullptr) {
			return true;
		}
		try {
			winrt::check_hresult(m_voice->Stop());
			winrt::check_hresult(m_voice->FlushSourceBuffers());

			XAUDIO2_VOICE_STATE state = {};
			while (true) {
				m_voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
				if (state.BuffersQueued < XAUDIO2_MAX_QUEUED_BUFFERS) {
					break;
				}
				Logger::warn("[core] audio buffer queue is full");
			}

			winrt::check_hresult(m_voice->SubmitSourceBuffer(&m_voice_buffer));

			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] SimpleAudioPlayerXAudio2::reset failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] SimpleAudioPlayerXAudio2::reset failed: <std::exception> {}", e.what());
		}
		return false;
	}

	bool SimpleAudioPlayerXAudio2::isPlaying() {
		if (m_voice == nullptr) {
			return m_is_playing;
		}
		XAUDIO2_VOICE_STATE state = {};
		m_voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		return m_is_playing && state.BuffersQueued > 0;
	}

	double SimpleAudioPlayerXAudio2::getTotalTime() {
		return m_total_seconds;
	}
	double SimpleAudioPlayerXAudio2::getTime() { assert(false); return 0.0; }
	bool SimpleAudioPlayerXAudio2::setTime(double) { assert(false); return true; }
	bool SimpleAudioPlayerXAudio2::setLoop(bool, double, double) { assert(false); return true; }

	float SimpleAudioPlayerXAudio2::getVolume() {
		return m_volume;
	}
	bool SimpleAudioPlayerXAudio2::setVolume(float const volume) {
		m_volume = std::clamp(volume, 0.0f, 1.0f);
		if (m_voice == nullptr) {
			return true;
		}
		try {
			winrt::check_hresult(m_voice->SetVolume(m_volume));
			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] IXAudio2SourceVoice::SetVolume failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] IXAudio2SourceVoice::SetVolume failed: <std::exception> {}", e.what());
		}
		return false;
	}
	float SimpleAudioPlayerXAudio2::getBalance() {
		return m_output_balance;
	}
	bool SimpleAudioPlayerXAudio2::setBalance(float const v) {
		m_output_balance = std::clamp(v, -1.0f, 1.0f);
		if (m_voice == nullptr) {
			return true;
		}
		try {
			winrt::check_hresult(setOutputBalance(m_voice, m_parent->getChannel(m_mixing_channel), m_output_balance));
			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] SimpleAudioPlayerXAudio2::setBalance failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] SimpleAudioPlayerXAudio2::setBalance failed: <std::exception> {}", e.what());
		}
		return false;
	}
	float SimpleAudioPlayerXAudio2::getSpeed() {
		return m_speed;
	}
	bool SimpleAudioPlayerXAudio2::setSpeed(float const speed) {
		m_speed = speed;
		if (m_voice == nullptr) {
			return true;
		}
		try {
			winrt::check_hresult(m_voice->SetFrequencyRatio(m_speed));
			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] IXAudio2SourceVoice::SetFrequencyRatio failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] IXAudio2SourceVoice::SetFrequencyRatio failed: <std::exception> {}", e.what());
		}
		return false;
	}

	void SimpleAudioPlayerXAudio2::updateFFT() { assert(false); }
	uint32_t SimpleAudioPlayerXAudio2::getFFTSize() { assert(false); return 0; }
	float* SimpleAudioPlayerXAudio2::getFFT() { assert(false); return s_empty_fft_data; }

	// IXAudio2VoiceCallback

	void WINAPI SimpleAudioPlayerXAudio2::OnVoiceError(void* const, HRESULT const error) noexcept {
		try {
			winrt::throw_hresult(error);
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] IXAudio2SourceVoice error: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] IXAudio2SourceVoice error: <std::exception> {}", e.what());
		}
	}

	// IAudioEndpointEventListener

	void SimpleAudioPlayerXAudio2::onAudioEndpointCreate() {
		create();
	}
	void SimpleAudioPlayerXAudio2::onAudioEndpointDestroy() {
		destroy();
	}

	// SimpleAudioPlayerXAudio2

	SimpleAudioPlayerXAudio2::~SimpleAudioPlayerXAudio2() {
		if (m_parent) {
			m_parent->removeEventListener(this);
		}
		destroy();
	}

	bool SimpleAudioPlayerXAudio2::create() {
		try {
			if (m_parent->getDirectChannel() == nullptr) {
				return false;
			}

			XAUDIO2_SEND_DESCRIPTOR voice_send{};
			voice_send.pOutputVoice = m_parent->getChannel(m_mixing_channel);
			XAUDIO2_VOICE_SENDS voice_send_list{};
			voice_send_list.SendCount = 1;
			voice_send_list.pSends = &voice_send;

			winrt::check_hresult(m_parent->getFactory()->CreateSourceVoice(&m_voice, &m_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this, &voice_send_list));
			winrt::check_hresult(m_voice->SetVolume(m_volume));
			winrt::check_hresult(m_voice->SetFrequencyRatio(m_speed));
			if (!setBalance(m_output_balance)) {
				return false;
			}

			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] SimpleAudioPlayerXAudio2::create failed: <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] SimpleAudioPlayerXAudio2::create failed: <std::exception> {}", e.what());
		}
		return false;
	}
	bool SimpleAudioPlayerXAudio2::create(AudioEndpointXAudio2* const parent, AudioMixingChannel const mixing_channel, IAudioDecoder* const decoder) {
		m_parent = parent;
		m_mixing_channel = mixing_channel;
		m_total_seconds = static_cast<double>(decoder->getFrameCount()) / static_cast<double>(decoder->getSampleRate());
	
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
	void SimpleAudioPlayerXAudio2::destroy() {
		if (m_voice != nullptr) {
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
}
