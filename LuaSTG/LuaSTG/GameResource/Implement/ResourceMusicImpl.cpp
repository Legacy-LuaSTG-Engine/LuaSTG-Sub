#include "GameResource/Implement/ResourceMusicImpl.hpp"

namespace LuaSTGPlus
{
	bool ResourceMusicImpl::LoopDecoder::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame)
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

	ResourceMusicImpl::LoopDecoder::LoopDecoder(Core::Audio::IDecoder* p_decoder, double LoopStart, double LoopEnd)
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

	void ResourceMusicImpl::Play(float vol, double position)
	{
		m_player->reset();

		m_player->setTime(position);
		m_player->setVolume(vol);

		m_player->start();
		m_status = 2;
	}
	void ResourceMusicImpl::Stop()
	{
		m_player->reset();
		m_status = 0;
	}
	void ResourceMusicImpl::Pause()
	{
		m_player->stop();
		m_status = 1;
	}
	void ResourceMusicImpl::Resume()
	{
		m_player->start();
		m_status = 2;
	}
	bool ResourceMusicImpl::IsPlaying() { return m_player->isPlaying(); }
	bool ResourceMusicImpl::IsPaused() { return m_status == 1; }
	bool ResourceMusicImpl::IsStopped() { return !IsPlaying() && m_player->getTotalTime() == 0.0; }
	void ResourceMusicImpl::SetVolume(float v) { m_player->setVolume(v); }
	float ResourceMusicImpl::GetVolume() { return m_player->getVolume(); }
	bool ResourceMusicImpl::SetSpeed(float speed) { return m_player->setSpeed(speed); }
	float ResourceMusicImpl::GetSpeed() { return m_player->getSpeed(); }
	void ResourceMusicImpl::SetLoop(bool v) { if (m_decoder) m_decoder->setLoop(v); }
	void ResourceMusicImpl::SetLoopRange(MusicRoopRange range)
	{
		// 修改循环范围必须停止BGM
		Stop();
		if (m_decoder)
		{
			// 转换单位
			if (range.unit == MusicRoopRangeUnit::Second)
			{
				auto const sample_rate = double(m_decoder->getSampleRate());
				range.start_in_samples = uint32_t(range.start_in_seconds * sample_rate);
				range.end_in_samples = uint32_t(range.end_in_seconds * sample_rate);
				range.length_in_samples = uint32_t(range.length_in_seconds * sample_rate);
			}
			// 循环开关
			m_decoder->setLoop(range.type != MusicRoopRangeType::Disable);
			// 限制范围
			auto const total_samples = m_decoder->getFrameCount();
			range.start_in_samples = std::min(range.start_in_samples, total_samples);
			range.end_in_samples = std::min(range.end_in_samples, total_samples);
			if (range.type == MusicRoopRangeType::StartPointAndLength)
			{
				range.length_in_samples = std::min(range.length_in_samples, total_samples - range.start_in_samples);
			}
			else if (range.type == MusicRoopRangeType::LengthAndEndPoint)
			{
				range.length_in_samples = std::min(range.length_in_samples, range.end_in_samples);
			}
			// 转换范围
			switch (range.type)
			{
			case MusicRoopRangeType::Disable:
			case MusicRoopRangeType::All:
				m_decoder->setLoopRange(0, total_samples);
				break;
			case MusicRoopRangeType::StartPointToEnd:
				m_decoder->setLoopRange(range.start_in_samples, total_samples);
				break;
			case MusicRoopRangeType::StartPointAndLength:
				m_decoder->setLoopRange(range.start_in_samples, range.start_in_samples + range.length_in_samples);
				break;
			case MusicRoopRangeType::LengthAndEndPoint:
				m_decoder->setLoopRange(range.end_in_samples - range.length_in_samples, range.end_in_samples);
				break;
			case MusicRoopRangeType::StartToEndPoint:
				m_decoder->setLoopRange(0, range.end_in_samples);
				break;
			case MusicRoopRangeType::StartPointAndEndPoint:
				m_decoder->setLoopRange(range.start_in_samples, range.end_in_samples);
				break;
			}
			// 打印日志
			switch (range.type)
			{
			case MusicRoopRangeType::Disable:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : Disable", GetResName());
				break;
			case MusicRoopRangeType::All:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : All", GetResName());
				break;
			case MusicRoopRangeType::StartPointToEnd:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : StartPointToEnd ({} ({}s) -> end)"
					, GetResName()
					, range.start_in_samples
					, double(range.start_in_samples) / sample_rate
				);
				break;
			case MusicRoopRangeType::StartPointAndLength:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : StartPointAndLength ({} ({}s) + {} ({}s))"
					, GetResName()
					, range.start_in_samples
					, double(range.start_in_samples) / sample_rate
					, range.length_in_samples
					, double(range.length_in_samples) / sample_rate
				);
				break;
			case MusicRoopRangeType::LengthAndEndPoint:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : LengthAndEndPoint ({} ({}s) - {} ({}s))"
					, GetResName()
					, range.length_in_samples
					, double(range.length_in_samples) / sample_rate
					, range.end_in_samples
					, double(range.end_in_samples) / sample_rate
				);
				break;
			case MusicRoopRangeType::StartToEndPoint:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : StartToEndPoint (start -> {} ({}s))"
					, GetResName()
					, range.end_in_samples
					, double(range.end_in_samples) / sample_rate
				);
				break;
			case MusicRoopRangeType::StartPointAndEndPoint:
				spdlog::info("[luastg] SetMusicLoopRange '{}' : StartPointAndEndPoint ({} ({}s) -> {} ({}s))"
					, GetResName()
					, range.start_in_samples
					, double(range.start_in_samples) / sample_rate
					, range.end_in_samples
					, double(range.end_in_samples) / sample_rate
				);
				break;
			}
		}
	}

	ResourceMusicImpl::ResourceMusicImpl(const char* name, LoopDecoder* p_decoder, Core::Audio::IAudioPlayer* p_player)
		: ResourceBaseImpl(ResourceType::Music, name)
		, m_decoder(p_decoder)
		, m_player(p_player)
	{
	}
}
