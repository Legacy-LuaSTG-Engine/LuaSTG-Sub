#pragma once
#include "Core/Object.hpp"
#include "Core/Audio/Device.hpp"

#include "xaudio2redist.h"

namespace LuaSTG::Core::Audio
{
	class Shared_XAUDIO2 : public Object<IObject>
	{
	public:
		Microsoft::WRL::ComPtr<IXAudio2> xa2_xaudio2;
		IXAudio2MasteringVoice* xa2_master;
		IXAudio2SubmixVoice* xa2_soundeffect;
		IXAudio2SubmixVoice* xa2_music;
	public:
		Shared_XAUDIO2();
		~Shared_XAUDIO2();
	};

	class Device_XAUDIO2 : public Object<IAudioDevice>
	{
	private:
		ScopeObject<Shared_XAUDIO2> m_shared;
		
	public:
		Shared_XAUDIO2* GetShared() { return m_shared.get(); }

	public:
		void setVolume(float v);
		float getVolume();
		void setMixChannelVolume(MixChannel ch, float v);
		float getMixChannelVolume(MixChannel ch);

		bool createAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player);
		bool createStreamAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player);

	public:
		Device_XAUDIO2();
		~Device_XAUDIO2();

	public:
		static bool create(Device_XAUDIO2** pp_audio);
	};

	class AudioPlayer_XAUDIO2
		: public Object<IAudioPlayer>
		, public IXAudio2VoiceCallback
	{
	private:
		ScopeObject<Device_XAUDIO2> m_device;
		IXAudio2SourceVoice* xa2_source;
		XAUDIO2_BUFFER xa2_buffer = {};
		Microsoft::WRL::Wrappers::Event event_end;
		std::vector<BYTE> pcm_data;
		float output_balance = 0.0f;
		float empty_fft[1]{};
		bool is_playing = false;

	public:
		void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired);
		void WINAPI OnVoiceProcessingPassEnd();
		void WINAPI OnStreamEnd();
		void WINAPI OnBufferStart(void* pBufferContext);
		void WINAPI OnBufferEnd(void* pBufferContext);
		void WINAPI OnLoopEnd(void* pBufferContext);
		void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error);

	public:
		bool start();
		bool stop();
		bool reset();

		bool isPlaying();

		double getTotalTime() { return 0.0; }
		double getTime() { return 0.0; }
		bool setTime(double) { return true; }

		float getVolume();
		bool setVolume(float v);
		float getBalance();
		bool setBalance(float v);
		float getSpeed();
		bool setSpeed(float v);

		void updateFFT() {}
		uint32_t getFFTSize() { return 0; }
		float* getFFT() { return empty_fft; }

	public:
		AudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder);
		~AudioPlayer_XAUDIO2();
	};

	class StreamAudioPlayer_XAUDIO2
		: public Object<IAudioPlayer>
		, public IXAudio2VoiceCallback
	{
	private:
		enum class ActionType
		{
			Exit,
			Stop,
			Start,
			Reset,
			SetTime,
			BufferAvailable, // 注意，严禁通过 sendAction 发出，请使用 notifyBufferAvailable
		};
		struct Action
		{
			ActionType type;
			union
			{
				struct ActionExit
				{
					uint8_t _dummy;
				} action_exit;
				struct ActionStop
				{
					uint8_t _dummy;
				} action_stop;
				struct ActionStart
				{
					uint8_t _dummy;
				} action_start;
				struct ActionReset
				{
					bool play;
				} action_reset;
				struct ActionSetTime
				{
					double time;
				} action_set_time;
				struct ActionBufferAvailable
				{
					size_t index;
				} action_buffer_available;
			};
		};
		struct ActionQueue
		{
			size_t const size = 64;
			Action data[64] = {};
			size_t writer_index = 0;
			size_t reader_index = 0;
			HANDLE semaphore_space = NULL;
			HANDLE semaphore_data = NULL;
			HANDLE event_exit = NULL;
			HANDLE event_buffer[2] = { NULL, NULL };

			bool createObjects();
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
			void reciveAction(Action& v);

			ActionQueue();
			~ActionQueue();
		};
		enum class State
		{
			Stop,
			Pause,
			Play,
		};

	private:
		ScopeObject<Device_XAUDIO2> m_device;
		ScopeObject<IDecoder> m_decoder;
		IXAudio2SourceVoice* xa2_source;
		State source_state = State::Stop;
		Microsoft::WRL::Wrappers::ThreadHandle working_thread;
		double start_time = 0.0;
		double total_time = 0.0;
		double current_time = 0.0;
		float output_balance = 0.0f;
		ActionQueue action_queue;
		std::vector<uint8_t> raw_buffer;

		uint8_t* p_audio_buffer[2] = {};
		size_t audio_buffer_index = 0;
		std::vector<float> fft_wave_data;
		std::vector<float> fft_window;
		std::vector<float> fft_data;
		std::vector<float> fft_complex_output;
		std::vector<float> fft_output;

	public:
		void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired);
		void WINAPI OnVoiceProcessingPassEnd();
		void WINAPI OnStreamEnd();
		void WINAPI OnBufferStart(void* pBufferContext);
		void WINAPI OnBufferEnd(void* pBufferContext);
		void WINAPI OnLoopEnd(void* pBufferContext);
		void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error);

	private:
		static DWORD WINAPI WorkingThread(LPVOID lpThreadParameter);

	public:

		bool start();
		bool stop();
		bool reset();

		bool isPlaying();

		double getTotalTime();
		double getTime();
		bool setTime(double t);

		float getVolume();
		bool setVolume(float v);
		float getBalance();
		bool setBalance(float v);
		float getSpeed();
		bool setSpeed(float v);

		void updateFFT();
		uint32_t getFFTSize();
		float* getFFT();

	public:
		StreamAudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder);
		~StreamAudioPlayer_XAUDIO2();
	};
}
