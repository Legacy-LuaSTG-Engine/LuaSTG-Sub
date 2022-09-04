#include "GameResource/ResourceAudio.hpp"

namespace LuaSTGPlus
{
	// ResSound

	void ResSound::FlushCommand()
	{
		// 根据最后的命令对音效进行操作
		switch (m_last_command.type)
		{
		case CommandType::None:
			break;
		case CommandType::Play:
			m_player->start();
			break;
		case CommandType::Stop:
			m_player->stop();
			break;
		case CommandType::Reset:
			m_player->reset();
			m_player->setVolume(m_last_command.vol);
			m_player->setBalance(m_last_command.pan);
			m_player->start();
			break;
		case CommandType::ResetAndStop:
			m_player->reset();
			m_player->setVolume(m_last_command.vol);
			m_player->setBalance(m_last_command.pan);
			break;
		default:
			assert(false);
			break;
		}
		// 重置为无命令
		m_last_command.type = CommandType::None;
		m_last_command.vol = 0.0f;
		m_last_command.pan = 0.0f;
	}
	void ResSound::Play(float vol, float pan)
	{
		// 优先级最高的命令，覆盖其他一切命令
		m_last_command.type = CommandType::Reset;
		m_last_command.vol = std::max(m_last_command.vol, vol); // 取音量最高
		m_last_command.pan = pan;
		m_status = 2; // playing
	}
	void ResSound::Resume()
	{
		switch (m_last_command.type)
		{
		case CommandType::None:
			// 初始化命令
			m_last_command.type = CommandType::Play;
			break;
		case CommandType::Play:
			// 就是这个命令
			m_last_command.type = CommandType::Play;
			break;
		case CommandType::Stop:
			// 覆盖暂停/停止命令
			m_last_command.type = CommandType::Play;
			break;
		case CommandType::Reset:
			// 保持当前命令
			break;
		case CommandType::ResetAndStop:
			// 修正为重新播放命令
			m_last_command.type = CommandType::Reset;
			break;
		default:
			assert(false);
			break;
		}
		m_status = 2; // playing
	}
	void ResSound::Pause()
	{
		switch (m_last_command.type)
		{
		case CommandType::None:
			// 初始化命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Play:
			// 覆盖播放/恢复命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Stop:
			// 就是这个命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Reset:
			// 修正为回到起始命令
			m_last_command.type = CommandType::ResetAndStop;
			break;
		case CommandType::ResetAndStop:
			// 保持当前命令
			break;
		default:
			assert(false);
			break;
		}
		m_status = 1; // pause
	}
	void ResSound::Stop()
	{
		switch (m_last_command.type)
		{
		case CommandType::None:
			// 初始化命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Play:
			// 覆盖播放/恢复命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Stop:
			// 就是这个命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Reset:
			// 修正为回到起始命令
			m_last_command.type = CommandType::ResetAndStop;
			break;
		case CommandType::ResetAndStop:
			// 保持当前命令
			break;
		default:
			assert(false);
			break;
		}
		m_status = 0; // stop
	}
	bool ResSound::IsPlaying() { return m_player->isPlaying() || m_status == 2; }
	bool ResSound::IsStopped() { return !IsPlaying() && m_status != 1; }
	bool ResSound::SetSpeed(float speed) { return m_player->setSpeed(speed); }
	float ResSound::GetSpeed() { return m_player->getSpeed(); }

	ResSound::ResSound(const char* name, Core::Audio::IAudioPlayer* p_player)
		: Resource(ResourceType::SoundEffect, name)
		, m_player(p_player)
	{
	}
	ResSound::~ResSound() {}

	// ResMusic

	bool ResMusic::LoopDecoder::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame)
	{
		uint32_t const raw_pcm_frame = pcm_frame;
		uint8_t* p_buffer = (uint8_t*)buffer;

		// 填充音频数据
		if (m_is_loop)
		{
			while (pcm_frame > 0)
			{
				// 获得当前解码器位置(采样)
				uint32_t current_frame = 0;
				if (!tell(&current_frame)) { assert(false); return false; }

				// 检查读取位置是否超出循环节
				if ((current_frame + pcm_frame) >= m_end_sample)
				{
					// 填充尚未填充数据
					if (current_frame < m_end_sample)
					{
						uint32_t const vaild_frame = m_end_sample - current_frame;
						uint32_t const should_read_size = vaild_frame * getFrameSize();
						uint32_t read_frame = 0;
						if (!m_decoder->read(vaild_frame, p_buffer, &read_frame)) { assert(false); return false; }
						//spdlog::debug("[Loop] read [{}, {}) total {} samples", current_frame, current_frame + read_frame, read_frame);

						// 剩下的数据全部置为0
						uint8_t* ptr = p_buffer + (read_frame * getFrameSize());
						uint32_t const fill_size = (vaild_frame - read_frame) * getFrameSize();
						if (fill_size > 0)
						{
							assert(false); std::memset(ptr, 0, fill_size);
						}
						
						// 即使实际上没有读取出来这么多内容，也要如此
						p_buffer += should_read_size;
						pcm_frame -= vaild_frame;
					}

					// 跳到循环头
					if (!seek(m_start_sample)) { assert(false); return false; }
				}
				else
				{
					uint32_t const vaild_frame = pcm_frame;
					uint32_t const should_read_size = vaild_frame * getFrameSize();
					uint32_t read_frame = 0;
					if (!m_decoder->read(vaild_frame, p_buffer, &read_frame)) { assert(false); return false; }
					//spdlog::debug("[Loop] read [{}, {}) total {} samples", current_frame, current_frame + read_frame, read_frame);

					// 剩下的数据全部置为0
					uint8_t* ptr = p_buffer + (read_frame * getFrameSize());
					uint32_t const fill_size = (vaild_frame - read_frame) * getFrameSize();
					if (fill_size > 0)
					{
						assert(false); std::memset(ptr, 0, fill_size);
					}

					// 即使实际上没有读取出来这么多内容，也要如此
					p_buffer += should_read_size;
					pcm_frame -= vaild_frame;
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
			*read_pcm_frame = raw_pcm_frame;
		}

		return true;
	}

	ResMusic::LoopDecoder::LoopDecoder(Core::Audio::IDecoder* p_decoder, double LoopStart, double LoopEnd)
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
	void ResMusic::SetLoop(bool v) { if (m_decoder) m_decoder->setLoop(v); }

	ResMusic::ResMusic(const char* name, LoopDecoder* p_decoder, Core::Audio::IAudioPlayer* p_player)
		: Resource(ResourceType::Music, name)
		, m_decoder(p_decoder)
		, m_player(p_player)
	{
	}
	ResMusic::~ResMusic() {}
}
