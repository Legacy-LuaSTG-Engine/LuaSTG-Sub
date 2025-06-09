#include "GameResource/Implement/ResourceSoundEffectImpl.hpp"

namespace luastg {
	void ResourceSoundEffectImpl::FlushCommand() {
		// apply
		switch (m_last_command.type) {
		case CommandType::none:
			break;
		case CommandType::play:
			m_player->setVolume(m_last_command.vol);
			m_player->setBalance(m_last_command.pan);
			m_player->play(0.0);
			break;
		case CommandType::pause:
			m_player->pause();
			break;
		case CommandType::resume:
			m_player->resume();
			break;
		case CommandType::stop:
			m_player->stop();
			break;
		}
		// clear
		m_last_command.type = CommandType::none;
		m_last_command.vol = 1.0f;
		m_last_command.pan = 0.0f;
		// read state
		if (m_state != core::AudioPlayerState::stopped) {
			if (m_player->getState() == core::AudioPlayerState::stopped) {
				m_state = core::AudioPlayerState::stopped;
			}
		}
	}
	void ResourceSoundEffectImpl::Play(float const vol, float const pan) {
		m_last_command.type = CommandType::play;
		m_last_command.vol = vol;
		m_last_command.pan = pan;
		m_state = core::AudioPlayerState::playing;
	}
	void ResourceSoundEffectImpl::Resume() {
		if (m_state == core::AudioPlayerState::paused) {
			m_last_command.type = CommandType::resume;
			m_state = core::AudioPlayerState::playing;
		}
	}
	void ResourceSoundEffectImpl::Pause() {
		if (m_state == core::AudioPlayerState::playing) {
			m_last_command.type = CommandType::pause;
			m_state = core::AudioPlayerState::paused;
		}
	}
	void ResourceSoundEffectImpl::Stop() {
		m_last_command.type = CommandType::stop;
		m_state = core::AudioPlayerState::stopped;
	}
	bool ResourceSoundEffectImpl::IsPlaying() { return m_state == core::AudioPlayerState::playing; }
	bool ResourceSoundEffectImpl::IsStopped() { return m_state == core::AudioPlayerState::stopped; }
	bool ResourceSoundEffectImpl::SetSpeed(float const speed) { return m_player->setSpeed(speed); }
	float ResourceSoundEffectImpl::GetSpeed() { return m_player->getSpeed(); }

	ResourceSoundEffectImpl::ResourceSoundEffectImpl(const char* name, core::IAudioPlayer* p_player)
		: ResourceBaseImpl(ResourceType::SoundEffect, name)
		, m_player(p_player) {
	}
}
