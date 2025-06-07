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
	class StreamAudioPlayerXAudio2 final
		: public implement::ReferenceCounted<IAudioPlayer>
		, public XAudio2VoiceCallbackHelper
		, public IAudioEndpointEventListener {
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

		void WINAPI OnBufferEnd(void* buffer_context) noexcept override;
		void WINAPI OnVoiceError(void*, HRESULT error) noexcept override;

		// IAudioEndpointEventListener

		void onAudioEndpointCreate() override;
		void onAudioEndpointDestroy() override;

		// StreamAudioPlayerXAudio2

		StreamAudioPlayerXAudio2() = default;
		StreamAudioPlayerXAudio2(StreamAudioPlayerXAudio2 const&) = delete;
		StreamAudioPlayerXAudio2(StreamAudioPlayerXAudio2&&) = delete;
		~StreamAudioPlayerXAudio2() override;

		StreamAudioPlayerXAudio2& operator=(StreamAudioPlayerXAudio2 const&) = delete;
		StreamAudioPlayerXAudio2& operator=(StreamAudioPlayerXAudio2&&) = delete;

		bool create();
		bool create(AudioEndpointXAudio2* parent, AudioMixingChannel mixing_channel, IAudioDecoder* decoder);
		void destroy();
		void worker();

	private:
		enum class ActionType : uint8_t {
			exit,
			buffer_available, // 注意，严禁通过 sendAction 发出，请使用 notifyBufferAvailable
			play,
			pause,
			resume,
			stop,
			update_loop,
		};

		struct Action {
			double start;
			double length;
			uint8_t buffer_available_index;
			ActionType type;
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
			std::atomic_size_t m_writer_index{ 0 };
			std::atomic_size_t m_reader_index{ 0 };
			std::counting_semaphore<255> m_semaphore_space{ 64 };
			std::counting_semaphore<255> m_semaphore_data{ 0 };
			std::atomic_bool m_event_exit{ false };
			std::atomic_int m_buffer_available_mask{ 0x0 };
		};

		SmartReference<AudioEndpointXAudio2> m_parent;
		IXAudio2SourceVoice* m_voice{};
		WAVEFORMATEX m_format{};
		AudioMixingChannel m_mixing_channel;
		float m_volume{ 1.0f };
		float m_output_balance{ 0.0f };
		float m_speed{ 1.0f };

		// decode

		SmartReference<IAudioDecoder> m_decoder;
		std::vector<uint8_t> raw_buffer;
		uint8_t* p_audio_buffer[2] = {};

		// worker

		ActionQueue m_action_queue;
		std::thread m_working_thread;
		std::shared_mutex m_voice_lock;

		// state

		double m_played_seconds{};
		double m_current_seconds{};
		AudioPlayerState m_state{ AudioPlayerState::stopped };

		// loop

		bool m_loop_enabled{};
		uint32_t m_loop_sample_start{};
		uint32_t m_loop_sample_count{};
		uint32_t m_current_sample{}; // [worker thread only]

		// fft

		size_t m_fft_buffer_index{};
		std::vector<float> m_fft_input;
		std::vector<float> m_fft_window;
		std::vector<float> m_fft_data;
		std::vector<float> m_fft_complex_result;
		std::vector<float> m_fft_result;
	};
}
