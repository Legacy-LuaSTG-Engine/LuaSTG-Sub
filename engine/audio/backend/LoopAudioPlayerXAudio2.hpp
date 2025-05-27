#pragma once
#include "core/AudioPlayer.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "backend/CommonAudioPlayerXAudio2.hpp"
#include "backend/AudioEndpointXAudio2.hpp"

namespace core {
	class LoopAudioPlayerXAudio2 final
		: public implement::ReferenceCounted<IAudioPlayer>
		, public XAudio2VoiceCallbackHelper
		, public IAudioEndpointEventListener {
	public:
		// IAudioPlayer

		bool start() override;
		bool stop() override;
		bool reset() override;

		bool isPlaying() override;

		double getTotalTime() override;
		double getTime() override;
		bool setTime(double time) override;
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

		void WINAPI OnVoiceError(void*, HRESULT error) noexcept override;

		// IAudioEndpointEventListener

		void onAudioEndpointCreate() override;
		void onAudioEndpointDestroy() override;

		// LoopAudioPlayerXAudio2

		LoopAudioPlayerXAudio2() = default;
		LoopAudioPlayerXAudio2(LoopAudioPlayerXAudio2 const&) = delete;
		LoopAudioPlayerXAudio2(LoopAudioPlayerXAudio2&&) = delete;
		~LoopAudioPlayerXAudio2() override;

		LoopAudioPlayerXAudio2& operator=(LoopAudioPlayerXAudio2 const&) = delete;
		LoopAudioPlayerXAudio2& operator=(LoopAudioPlayerXAudio2&&) = delete;

		bool create();
		bool create(AudioEndpointXAudio2* parent, AudioMixingChannel mixing_channel, IAudioDecoder* decoder);
		void destroy();

	private:
		SmartReference<AudioEndpointXAudio2> m_parent;
		IXAudio2SourceVoice* m_voice{};
		WAVEFORMATEX m_format{};
		XAUDIO2_BUFFER m_voice_buffer = {};
		std::vector<BYTE> m_pcm_data;
		AudioMixingChannel m_mixing_channel;
		float m_volume = 1.0f;
		float m_output_balance = 0.0f;
		float m_speed = 1.0f;
		double m_total_seconds{};
		bool m_is_playing{};

		double m_start_time{};
		bool m_loop{};
		double m_loop_start{};
		double m_loop_length{};
		uint32_t m_total_frame{};
		uint32_t m_sample_rate{};
		uint16_t m_frame_size{};
	};
}
