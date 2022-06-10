#include "ResourceAudio.hpp"
#include <cassert>

namespace LuaSTGPlus
{
	void ResSound::Play(float vol, float pan)
	{
		m_player->reset();

		m_player->setVolume(vol);
		m_player->setBalance(pan);

		m_player->start();
		m_status = 2;
	}
	void ResSound::Resume()
	{
		m_player->start();
		m_status = 2;
	}
	void ResSound::Pause()
	{
		m_player->stop();
		m_status = 1;
	}
	void ResSound::Stop()
	{
		m_player->reset();
		m_status = 0;
	}
	bool ResSound::IsPlaying() { return m_player->isPlaying(); }
	bool ResSound::IsStopped() { return !IsPlaying() && m_status != 1; }
	bool ResSound::SetSpeed(float speed) { return m_player->setSpeed(speed); }
	float ResSound::GetSpeed() { return m_player->getSpeed(); }

	ResSound::ResSound(const char* name, LuaSTG::Core::Audio::IAudioPlayer* p_player)
		: Resource(ResourceType::SoundEffect, name)
		, m_player(p_player)
	{
	}
	ResSound::~ResSound() {}

	bool ResMusic::LoopDecoder::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame)
	{
		uint8_t* p_buffer = (uint8_t*)buffer;

		// 填充音频数据
		if (m_is_loop)
		{
			while (pcm_frame)
			{
				// 获得当前解码器位置(采样)
				uint32_t current_frame = 0;
				if (!tell(&current_frame)) { assert(false); return false; }

				// 检查读取位置是否超出循环节
				if ((current_frame + pcm_frame) > m_end_sample)
				{
					// 填充尚未填充数据
					if (current_frame < m_end_sample)
					{
						uint32_t const vaild_frame = m_end_sample - current_frame;
						uint32_t const should_read_size = vaild_frame * getFrameSize();
						uint32_t read_frame = 0; // 这个会被忽略
						if (!m_decoder->read(vaild_frame, p_buffer, &read_frame)) { assert(false); return false; }
						
						// 即使实际上没有读取出来这么多内容，也要如此
						p_buffer += should_read_size;
						pcm_frame -= vaild_frame;
					}

					// 跳到循环头
					if (!seek(m_start_sample)) { assert(false); return false; }
				}
				else
				{
					// 直接填充数据
					uint32_t read_frame = 0; // 这个会被忽略
					if (!m_decoder->read(pcm_frame, p_buffer, &read_frame)) { assert(false); return false; }
					break;
				}
			}
		}
		else
		{
			// 直接填充数据
			uint32_t read_frame = 0;
			if (!m_decoder->read(pcm_frame, p_buffer, &read_frame)) { assert(false); return false; }
			p_buffer += read_frame * getFrameSize();

			// 剩下的数据全部置为0
			uint32_t const fill_size = (pcm_frame - read_frame) * getFrameSize();
			if (fill_size > 0)
			{
				std::memset(p_buffer, 0, fill_size);
			}
		}

		if (read_pcm_frame)
		{
			// 即使实际上没有读取出来这么多内容，也要如此
			*read_pcm_frame = pcm_frame;
		}

		return true;
	}

	ResMusic::LoopDecoder::LoopDecoder(LuaSTG::Core::Audio::IDecoder* p_decoder, double LoopStart, double LoopEnd)
		: m_decoder(p_decoder)
	{
		// 计算参数
		m_total_sample = getFrameCount();

		if (LoopStart <= 0)
			m_start_sample = 0;
		else
			m_start_sample = (uint32_t)(LoopStart * getSampleRate());

		if (LoopEnd <= 0)
			m_end_sample = m_total_sample;
		else
			m_end_sample = std::min(m_total_sample, (uint32_t)(LoopEnd * getSampleRate()));

		if (m_end_sample < m_start_sample)
			std::swap(m_start_sample, m_end_sample);

		if (m_start_sample == m_end_sample)
			throw std::runtime_error("ResMusic::LoopDecoder::LoopDecoder (1)");
	}

	void ResMusic::Play(float vol, double position)
	{
		m_player->reset();

		m_player->setTime(position);
		m_player->setVolume(vol);

		m_player->start();
		m_status = 2;
	}
	void ResMusic::Stop()
	{
		m_player->reset();
		m_status = 0;
	}
	void ResMusic::Pause()
	{
		m_player->stop();
		m_status = 1;
	}
	void ResMusic::Resume()
	{
		m_player->start();
		m_status = 2;
	}
	bool ResMusic::IsPlaying() { return m_player->isPlaying(); }
	bool ResMusic::IsPaused() { return m_status == 1; }
	bool ResMusic::IsStopped() { return !IsPlaying() && m_player->getTotalTime() == 0.0; }
	void ResMusic::SetVolume(float v) { m_player->setVolume(v); }
	float ResMusic::GetVolume() { return m_player->getVolume(); }
	bool ResMusic::SetSpeed(float speed) { return m_player->setSpeed(speed); }
	float ResMusic::GetSpeed() { return m_player->getSpeed(); }
	void ResMusic::SetLoop(bool v) { m_decoder->setLoop(v); }

	ResMusic::ResMusic(const char* name, LoopDecoder* p_decoder, LuaSTG::Core::Audio::IAudioPlayer* p_player)
		: Resource(ResourceType::Music, name)
		, m_decoder(p_decoder)
		, m_player(p_player)
	{
	}
}
