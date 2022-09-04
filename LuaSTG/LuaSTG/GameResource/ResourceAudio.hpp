#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Object.hpp"
#include "Core/Audio/Device.hpp"

namespace LuaSTGPlus
{
	// 音效
	class ResSound : public Resource
	{
	public:
		enum class CommandType : uint32_t
		{
			None,
			Play,
			Stop,
			Reset,
			ResetAndStop,
		};
		struct Command
		{
			CommandType type = CommandType::None;
			float vol = 0.0f;
			float pan = 0.0f;
		};
	private:
		Core::ScopeObject<Core::Audio::IAudioPlayer> m_player;
		int m_status = 0; // 0停止 1暂停 2播放
		Command m_last_command;
	public:
		void FlushCommand();
		void Play(float vol, float pan);
		void Resume();
		void Pause();
		void Stop();
		bool IsPlaying();
		bool IsStopped();
		bool SetSpeed(float speed);
		float GetSpeed();

	public:
		ResSound(const char* name, Core::Audio::IAudioPlayer* p_player);
		~ResSound();
	};

	// 背景音乐
	class ResMusic : public Resource
	{
	public:
		class LoopDecoder : public Core::Object<Core::Audio::IDecoder>
		{
		protected:
			Core::ScopeObject<Core::Audio::IDecoder> m_decoder;
			uint32_t m_total_sample = 0;
			uint32_t m_start_sample = 0;
			uint32_t m_end_sample = 0;
			bool m_is_loop = true;
		public:
			uint16_t getSampleSize() { return m_decoder->getSampleSize(); }
			uint16_t getChannelCount() { return m_decoder->getChannelCount(); }
			uint16_t getFrameSize() { return m_decoder->getFrameSize(); }
			uint32_t getSampleRate() { return m_decoder->getSampleRate(); }
			uint32_t getByteRate() { return m_decoder->getByteRate(); }
			uint32_t getFrameCount() { return m_decoder->getFrameCount(); }

			bool seek(uint32_t pcm_frame) { return m_decoder->seek(pcm_frame); }
			bool seekByTime(double sec) { return m_decoder->seekByTime(sec); }
			bool tell(uint32_t* pcm_frame) { return m_decoder->tell(pcm_frame); }
			bool tellAsTime(double* sec) { return m_decoder->tellAsTime(sec); }

			bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame);
			
			void setLoop(bool v) { m_is_loop = v ;}
		public:
			LoopDecoder(Core::Audio::IDecoder* p_decoder, double LoopStart, double LoopEnd);
		};

	private:
		Core::ScopeObject<LoopDecoder> m_decoder;
		Core::ScopeObject<Core::Audio::IAudioPlayer> m_player;
		int m_status = 0; // 0停止 1暂停 2播放

	public:
		Core::Audio::IAudioPlayer* GetAudioPlayer() { return m_player.get(); }

		void Play(float vol, double position);
		void Stop();
		void Pause();
		void Resume();
		bool IsPlaying();
		bool IsPaused();
		bool IsStopped();
		void SetVolume(float v);
		float GetVolume();
		bool SetSpeed(float speed);
		float GetSpeed();
		void SetLoop(bool v);

	public:
		ResMusic(const char* name, LoopDecoder* p_decoder, Core::Audio::IAudioPlayer* p_player);
		~ResMusic();
	};
}
