#pragma once
#include "Core/Object.hpp"
#include "Core/Audio/Device.hpp"

#include "xaudio2redist.h"

namespace Core::Audio
{
	struct IAudioDeviceEventListener
	{
		virtual void onAudioDeviceCreate() = 0;
		virtual void onAudioDeviceDestroy() = 0;
	};

	class Shared_XAUDIO2 : public Object<IObject>
	{
	public:
		winrt::com_ptr<IXAudio2> xaudio2;
		winrt::xaudio2_voice_ptr<IXAudio2MasteringVoice> voice_master;
		winrt::xaudio2_voice_ptr<IXAudio2SubmixVoice> voice_sound_effect;
		winrt::xaudio2_voice_ptr<IXAudio2SubmixVoice> voice_music;
	public:
		Shared_XAUDIO2();
		~Shared_XAUDIO2();
	};

	class Device_XAUDIO2 : public Object<IAudioDevice>
	{
	private:
		std::unordered_set<IAudioDeviceEventListener*> m_listener;
		bool m_dispatch_event{};
		void dispatchEventAudioDeviceCreate();
		void dispatchEventAudioDeviceDestroy();
	public:
		void addEventListener(IAudioDeviceEventListener* p_m_listener);
		void removeEventListener(IAudioDeviceEventListener* p_m_listener);

	private:
		struct AudioDeviceInfo
		{
			std::string id;
			std::string name;
		};
		std::vector<AudioDeviceInfo> m_audio_device_list;
		std::string m_target_audio_device_name;
		std::string m_current_audio_device_name;
		bool refreshAudioDeviceList();
	public:
		uint32_t getAudioDeviceCount(bool refresh);
		std::string_view getAudioDeviceName(uint32_t index) const noexcept;
		bool setTargetAudioDevice(std::string_view const audio_device_name);
		std::string_view getCurrentAudioDeviceName() const noexcept { return m_current_audio_device_name; }

	private:
		ScopeObject<Shared_XAUDIO2> m_shared;
		float m_volume_direct = 1.0f;
		float m_volume_sound_effect = 1.0f;
		float m_volume_music = 1.0f;
	public:
		Shared_XAUDIO2* getShared() { return m_shared.get(); }
		bool createResources();
		void destroyResources();

	public:
		void setVolume(float v);
		float getVolume();
		void setMixChannelVolume(MixChannel ch, float v);
		float getMixChannelVolume(MixChannel ch);

		bool createAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player);
		bool createLoopAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player);
		bool createStreamAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player);

	public:
		Device_XAUDIO2();
		~Device_XAUDIO2();

	public:
		static bool create(Device_XAUDIO2** pp_audio);
	};

	struct XAudio2VoiceCallbackPlaceholder : public IXAudio2VoiceCallback
	{
		virtual void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired) noexcept { UNREFERENCED_PARAMETER(BytesRequired); }
		virtual void WINAPI OnVoiceProcessingPassEnd() noexcept {}
		virtual void WINAPI OnStreamEnd() noexcept {}
		virtual void WINAPI OnBufferStart(void* pBufferContext) noexcept { UNREFERENCED_PARAMETER(pBufferContext); }
		virtual void WINAPI OnBufferEnd(void* pBufferContext) noexcept { UNREFERENCED_PARAMETER(pBufferContext); }
		virtual void WINAPI OnLoopEnd(void* pBufferContext) noexcept { UNREFERENCED_PARAMETER(pBufferContext); }
		virtual void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error) noexcept { UNREFERENCED_PARAMETER(pBufferContext); UNREFERENCED_PARAMETER(Error); }
	};

	class AudioPlayer_XAUDIO2
		: public Object<IAudioPlayer>
		, public XAudio2VoiceCallbackPlaceholder
		, public IAudioDeviceEventListener
	{
	private:
		ScopeObject<Device_XAUDIO2> m_device;
		ScopeObject<Shared_XAUDIO2> m_shared;
	#ifndef NDEBUG
		ScopeObject<IDecoder> m_decoder;
	#endif
		winrt::xaudio2_voice_ptr<IXAudio2SourceVoice> m_player;
		WAVEFORMATEX m_format{};
		XAUDIO2_BUFFER m_player_buffer = {};
		std::vector<BYTE> m_pcm_data;
		float m_volume = 1.0f;
		float m_output_balance = 0.0f;
		float m_speed = 1.0f;
		bool m_is_playing{};
	public:
		void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error) noexcept;
	public:
		void onAudioDeviceCreate();
		void onAudioDeviceDestroy();
	private:
		bool createResources();
		void destoryResources();

	public:
		bool start();
		bool stop();
		bool reset();

		bool isPlaying();

		double getTotalTime();
		double getTime();
		bool setTime(double time);
		bool setLoop(bool enable, double start_pos, double length);

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
		AudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder);
		~AudioPlayer_XAUDIO2();
	};

	class LoopAudioPlayer_XAUDIO2
		: public Object<IAudioPlayer>
		, public XAudio2VoiceCallbackPlaceholder
		, public IAudioDeviceEventListener
	{
	private:
		ScopeObject<Device_XAUDIO2> m_device;
		ScopeObject<Shared_XAUDIO2> m_shared;
	#ifndef NDEBUG
		ScopeObject<IDecoder> m_decoder;
	#endif
		winrt::xaudio2_voice_ptr<IXAudio2SourceVoice> m_player;
		WAVEFORMATEX m_format{};
		std::vector<BYTE> m_pcm_data;
		float m_volume = 1.0f;
		float m_output_balance = 0.0f;
		float m_speed = 1.0f;
		bool m_is_playing{};
		double m_start_time{};
		bool m_is_loop{};
		double m_loop_start{};
		double m_loop_length{};
		uint32_t m_total_frame{};
		uint32_t m_sample_rate{};
		uint16_t m_frame_size{};
	public:
		void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error) noexcept;
	public:
		void onAudioDeviceCreate();
		void onAudioDeviceDestroy();
	private:
		bool createResources();
		void destoryResources();

	public:
		bool start();
		bool stop();
		bool reset();

		bool isPlaying();

		double getTotalTime();
		double getTime();
		bool setTime(double t);
		bool setLoop(bool enable, double start_pos, double length);

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
		LoopAudioPlayer_XAUDIO2(Device_XAUDIO2* p_device, IDecoder* p_decoder);
		~LoopAudioPlayer_XAUDIO2();
	};

	class StreamAudioPlayer_XAUDIO2
		: public Object<IAudioPlayer>
		, public XAudio2VoiceCallbackPlaceholder
		, public IAudioDeviceEventListener
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
		ScopeObject<Shared_XAUDIO2> m_shared;
		ScopeObject<IDecoder> m_decoder;
		winrt::xaudio2_voice_ptr<IXAudio2SourceVoice> m_player;
		wil::critical_section m_player_lock;
		WAVEFORMATEX m_format{};
		float m_volume = 1.0f;
		float m_output_balance = 0.0f;
		float m_speed = 1.0f;
		State source_state = State::Stop;
		Microsoft::WRL::Wrappers::ThreadHandle working_thread;
		double start_time = 0.0;
		double total_time = 0.0;
		double current_time = 0.0;
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
		void WINAPI OnBufferEnd(void* pBufferContext) noexcept;
		void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error) noexcept;
	public:
		void onAudioDeviceCreate();
		void onAudioDeviceDestroy();
	private:
		bool createResources();
		void destoryResources();

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
		bool setLoop(bool, double, double) { return true; }

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
