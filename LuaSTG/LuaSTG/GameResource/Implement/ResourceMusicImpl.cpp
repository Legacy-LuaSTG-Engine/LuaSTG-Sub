#include "GameResource/Implement/ResourceMusicImpl.hpp"

namespace luastg {
	void ResourceMusicImpl::Play(float const vol, double const position) {
		m_player->setVolume(vol);
		m_player->play(position);
	}
	void ResourceMusicImpl::Stop() { m_player->stop(); }
	void ResourceMusicImpl::Pause() { m_player->pause(); }
	void ResourceMusicImpl::Resume() { m_player->resume(); }
	bool ResourceMusicImpl::IsPlaying() { return m_player->getState() == core::AudioPlayerState::playing; }
	bool ResourceMusicImpl::IsPaused() { return m_player->getState() == core::AudioPlayerState::paused; }
	bool ResourceMusicImpl::IsStopped() { return m_player->getState() == core::AudioPlayerState::stopped; }
	void ResourceMusicImpl::SetVolume(float const v) { m_player->setVolume(v); }
	float ResourceMusicImpl::GetVolume() { return m_player->getVolume(); }
	bool ResourceMusicImpl::SetSpeed(float const speed) { return m_player->setSpeed(speed); }
	float ResourceMusicImpl::GetSpeed() { return m_player->getSpeed(); }
	void ResourceMusicImpl::SetLoop(bool const v) { if (!v) m_player->setLoop(false, 0.0, 0.0); }
	void ResourceMusicImpl::SetLoopRange(MusicRoopRange range) {
		if (m_decoder) {
			// 转换单位
			auto const sample_rate = static_cast<double>(m_decoder->getSampleRate());
			if (range.unit == MusicRoopRangeUnit::Sample) {
				range.start_in_seconds = static_cast<double>(range.start_in_samples) / sample_rate;
				range.end_in_seconds = static_cast<double>(range.end_in_samples) / sample_rate;
				range.length_in_seconds = static_cast<double>(range.length_in_samples) / sample_rate;
			}
			if (range.unit == MusicRoopRangeUnit::Second) {
				range.start_in_samples = static_cast<uint32_t>(range.start_in_seconds * sample_rate);
				range.end_in_samples = static_cast<uint32_t>(range.end_in_seconds * sample_rate);
				range.length_in_samples = static_cast<uint32_t>(range.length_in_seconds * sample_rate);
			}
			// 限制范围
			auto const total_samples = m_decoder->getFrameCount();
			auto const total_seconds = static_cast<double>(m_decoder->getFrameCount()) / sample_rate;
			range.start_in_samples = std::min(range.start_in_samples, total_samples);
			range.end_in_samples = std::min(range.end_in_samples, total_samples);
			if (range.type == MusicRoopRangeType::StartPointAndLength) {
				range.length_in_samples = std::min(range.length_in_samples, total_samples - range.start_in_samples);
			}
			else if (range.type == MusicRoopRangeType::LengthAndEndPoint) {
				range.length_in_samples = std::min(range.length_in_samples, range.end_in_samples);
			}
			// 转换范围
			switch (range.type) {
			case MusicRoopRangeType::Disable:
				m_player->setLoop(false, 0.0, 0.0);
				break;
			case MusicRoopRangeType::All:
				m_player->setLoop(true, 0.0, total_seconds);
				break;
			case MusicRoopRangeType::StartPointToEnd:
				m_player->setLoop(true, range.start_in_seconds, total_seconds - range.start_in_seconds);
				break;
			case MusicRoopRangeType::StartPointAndLength:
				m_player->setLoop(true, range.start_in_seconds, range.length_in_seconds);
				break;
			case MusicRoopRangeType::LengthAndEndPoint:
				m_player->setLoop(true, range.end_in_seconds - range.length_in_seconds, range.end_in_seconds);
				break;
			case MusicRoopRangeType::StartToEndPoint:
				m_player->setLoop(true, 0.0, range.end_in_seconds);
				break;
			case MusicRoopRangeType::StartPointAndEndPoint:
				m_player->setLoop(true, range.start_in_seconds, range.end_in_seconds - range.start_in_seconds);
				break;
			}
			// 打印日志
			switch (range.type) {
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

	ResourceMusicImpl::ResourceMusicImpl(const char* name, core::IAudioDecoder* decoder, core::IAudioPlayer* p_player)
		: ResourceBaseImpl(ResourceType::Music, name)
		, m_decoder(decoder)
		, m_player(p_player) {
	}
}
