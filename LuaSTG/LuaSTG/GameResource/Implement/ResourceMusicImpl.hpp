#pragma once
#include "GameResource/ResourceMusic.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceMusicImpl : public ResourceBaseImpl<IResourceMusic>
	{
	public:
		class LoopDecoder : public core::implement::ReferenceCounted<core::Audio::IDecoder>
		{
		protected:
			core::SmartReference<core::Audio::IDecoder> m_decoder;
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

			void setLoop(bool v) { m_is_loop = v; }
			void setLoopRange(uint32_t start_samples, uint32_t end_samples) { m_start_sample = start_samples; m_end_sample = end_samples; }
		public:
			LoopDecoder(core::Audio::IDecoder* p_decoder, double LoopStart, double LoopEnd);
		};

	private:
		core::SmartReference<LoopDecoder> m_decoder;
		core::SmartReference<core::Audio::IAudioPlayer> m_player;
		int m_status = 0; // 0停止 1暂停 2播放

	public:
		core::Audio::IAudioPlayer* GetAudioPlayer() { return m_player.get(); }

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
		void SetLoopRange(MusicRoopRange range);

	public:
		ResourceMusicImpl(const char* name, LoopDecoder* p_decoder, core::Audio::IAudioPlayer* p_player);
	};
}
