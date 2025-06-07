#pragma once
#include "core/AudioPlayer.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "backend/CommonAudioPlayerXAudio2.hpp"
#include "backend/AudioEngineXAudio2.hpp"
#include <atomic>

namespace core {
	class AudioPlayerXAudio2 final
		: public implement::ReferenceCounted<IAudioPlayer>
		, public XAudio2VoiceCallbackHelper
		, public IAudioEngineEventListener {
	public:
		// IAudioPlayer

		bool play(double seconds) override;
		bool pause() override;
		bool resume() override;
		bool stop() override;
		AudioPlayerState getState() override;

		double getTotalTime() override;
		double getTime() override;
		bool setLoop(bool enable, double start_pos, double length) override;

		float getVolume() override;
		bool setVolume(float volume) override;
		float getBalance() override;
		bool setBalance(float v) override;
		float getSpeed() override;
		bool setSpeed(float speed) override;

		void updateFFT() override;
		uint32_t getFFTSize() override;
		float const* getFFT() override;

		// IXAudio2VoiceCallback

		void WINAPI OnStreamEnd() noexcept override;
		void WINAPI OnVoiceError(void*, HRESULT error) noexcept override;

		// IAudioEngineEventListener

		void onAudioEngineCreate() override;
		void onAudioEngineDestroy() override;

		// AudioPlayerXAudio2

		AudioPlayerXAudio2() = default;
		AudioPlayerXAudio2(AudioPlayerXAudio2 const&) = delete;
		AudioPlayerXAudio2(AudioPlayerXAudio2&&) = delete;
		~AudioPlayerXAudio2() override;

		AudioPlayerXAudio2& operator=(AudioPlayerXAudio2 const&) = delete;
		AudioPlayerXAudio2& operator=(AudioPlayerXAudio2&&) = delete;

		bool create();
		bool create(AudioEngineXAudio2* parent, AudioMixingChannel mixing_channel, IAudioDecoder* decoder);
		void destroy();
		bool submitBuffer();

	private:
		SmartReference<AudioEngineXAudio2> m_parent;
		IXAudio2SourceVoice* m_voice{};
		WAVEFORMATEX m_format{};
		XAUDIO2_BUFFER m_voice_buffer = {};
		std::vector<BYTE> m_pcm_data;
		AudioMixingChannel m_mixing_channel;
		float m_volume = 1.0f;
		float m_output_balance = 0.0f;
		float m_speed = 1.0f;
		double m_total_seconds{};
		AudioPlayerState m_state{ AudioPlayerState::stopped };

		double m_start_time{};
		bool m_loop{};
		double m_loop_start{};
		double m_loop_length{};
		uint32_t m_total_frame{};
		uint32_t m_sample_rate{};
		uint16_t m_frame_size{};
	};
}
