#pragma once
#include "core/AudioPlayer.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "backend/CommonAudioPlayerXAudio2.hpp"
#include "backend/AudioEndpointXAudio2.hpp"
#include <thread>
#include <shared_mutex>
#include <semaphore>
#include <atomic>

namespace core {
	class StreamLoopAudioPlayerXAudio2 final
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
		bool setLoop(bool, double, double) override { return true; }

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

		void WINAPI OnBufferEnd(void* buffer_context) noexcept override;
		void WINAPI OnVoiceError(void*, HRESULT error) noexcept override;

		// IAudioEndpointEventListener

		void onAudioEndpointCreate() override;
		void onAudioEndpointDestroy() override;

		// StreamLoopAudioPlayerXAudio2

		StreamLoopAudioPlayerXAudio2() = default;
		StreamLoopAudioPlayerXAudio2(StreamLoopAudioPlayerXAudio2 const&) = delete;
		StreamLoopAudioPlayerXAudio2(StreamLoopAudioPlayerXAudio2&&) = delete;
		~StreamLoopAudioPlayerXAudio2() override;

		StreamLoopAudioPlayerXAudio2& operator=(StreamLoopAudioPlayerXAudio2 const&) = delete;
		StreamLoopAudioPlayerXAudio2& operator=(StreamLoopAudioPlayerXAudio2&&) = delete;

		bool create();
		bool create(AudioEndpointXAudio2* parent, AudioMixingChannel mixing_channel, IAudioDecoder* decoder);
		void destroy();
		void worker();

	private:
		enum class ActionType : uint8_t {
			Exit,
			Stop,
			Start,
			Reset,
			SetTime,
			BufferAvailable, // 注意，严禁通过 sendAction 发出，请使用 notifyBufferAvailable
		};

		struct Action {
			ActionType type;
			union {
				struct ActionExit {
					uint8_t _dummy;
				} action_exit;
				struct ActionStop {
					uint8_t _dummy;
				} action_stop;
				struct ActionStart {
					uint8_t _dummy;
				} action_start;
				struct ActionReset {
					bool play;
				} action_reset;
				struct ActionSetTime {
					double time;
				} action_set_time;
				struct ActionBufferAvailable {
					size_t index;
				} action_buffer_available;
			};
		};

		class ActionQueue {
		public:
			// 线程：任意线程
			// 通知工作线程应该退出
			void notifyExit();

			// 线程：任意线程
			// 通知指定的缓冲区已经处于可用状态，可用于解码并储存 PCM 数据
			void notifyBufferAvailable(size_t i);

			// 线程：仅限引擎更新线程
			// 向工作线程发送动作
			void sendAction(Action const& v);

			// 线程：仅限解码线程
			// 工作线程接收动作
			void receiveAction(Action& v);

		private:
			std::array<Action, 64> m_data{};
			std::atomic_size_t m_writer_index{0};
			std::atomic_size_t m_reader_index{0};
			std::counting_semaphore<255> m_semaphore_space{64};
			std::counting_semaphore<255> m_semaphore_data{0};
			std::atomic_bool m_event_exit{false};
			std::atomic_int m_buffer_available_mask{0x0};
		};

		enum class State : uint8_t {
			Stop,
			Pause,
			Play,
		};

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

		double total_time = 0.0;
		double current_time = 0.0;

		SmartReference<IAudioDecoder> m_decoder;
		ActionQueue action_queue;
		State source_state = State::Stop;

		std::vector<uint8_t> raw_buffer;
		uint8_t* p_audio_buffer[2] = {};
		size_t audio_buffer_index = 0;

		std::thread m_working_thread;
		std::shared_mutex m_player_lock;

		// FFT

		std::vector<float> fft_wave_data;
		std::vector<float> fft_window;
		std::vector<float> fft_data;
		std::vector<float> fft_complex_output;
		std::vector<float> fft_output;
	};
}
