#include "GameResource/Implement/ResourceSoundEffectImpl.hpp"

namespace luastg {
	void ResourceSoundEffectImpl::FlushCommand() {
		// 根据最后的命令对音效进行操作
		switch (m_last_command.type) {
		case CommandType::None:
			break;
		case CommandType::Play:
			m_player->resume();
			break;
		case CommandType::Stop:
			m_player->pause();
			break;
		case CommandType::Reset:
			m_player->setVolume(m_last_command.vol);
			m_player->setBalance(m_last_command.pan);
			m_player->play(0.0);
			break;
		case CommandType::ResetAndStop:
			m_player->stop();
			break;
		}
		// 重置为无命令
		m_last_command.type = CommandType::None;
		m_last_command.vol = 0.0f;
		m_last_command.pan = 0.0f;
		// 刷新状态
		if (m_status == 2) {
			if (m_player->getState() != core::AudioPlayerState::playing) {
				m_status = 0; // 已结束播放
			}
		}
	}
	void ResourceSoundEffectImpl::Play(float const vol, float const pan) {
		// 优先级最高的命令，覆盖其他一切命令
		m_last_command.type = CommandType::Reset;
		m_last_command.vol = std::max(m_last_command.vol, vol); // 取音量最高
		m_last_command.pan = pan;
		m_status = 2; // playing
	}
	void ResourceSoundEffectImpl::Resume() {
		switch (m_last_command.type) {
		case CommandType::None: // 初始化命令
		case CommandType::Play: // 就是这个命令
		case CommandType::Stop: // 覆盖暂停/停止命令
			m_last_command.type = CommandType::Play;
			break;
		case CommandType::Reset:
			// 保持当前命令
			break;
		case CommandType::ResetAndStop:
			// 修正为重新播放命令
			m_last_command.type = CommandType::Reset;
			break;
		}
		m_status = 2; // playing
	}
	void ResourceSoundEffectImpl::Pause() {
		switch (m_last_command.type) {
		case CommandType::None: // 初始化命令
		case CommandType::Play: // 覆盖播放/恢复命令
		case CommandType::Stop: // 就是这个命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Reset:
			// 修正为回到起始命令
			m_last_command.type = CommandType::ResetAndStop;
			break;
		case CommandType::ResetAndStop:
			// 保持当前命令
			break;
		}
		m_status = 1; // pause
	}
	void ResourceSoundEffectImpl::Stop() {
		switch (m_last_command.type) {
		case CommandType::None: // 初始化命令
		case CommandType::Play: // 覆盖播放/恢复命令
		case CommandType::Stop: // 就是这个命令
			m_last_command.type = CommandType::Stop;
			break;
		case CommandType::Reset:
			// 修正为回到起始命令
			m_last_command.type = CommandType::ResetAndStop;
			break;
		case CommandType::ResetAndStop:
			// 保持当前命令
			break;
		}
		m_status = 0; // stop
	}
	bool ResourceSoundEffectImpl::IsPlaying() { return m_player->getState() == core::AudioPlayerState::playing || m_status == 2; }
	bool ResourceSoundEffectImpl::IsStopped() { return !IsPlaying() && m_status != 1; }
	bool ResourceSoundEffectImpl::SetSpeed(float const speed) { return m_player->setSpeed(speed); }
	float ResourceSoundEffectImpl::GetSpeed() { return m_player->getSpeed(); }

	ResourceSoundEffectImpl::ResourceSoundEffectImpl(const char* name, core::IAudioPlayer* p_player)
		: ResourceBaseImpl(ResourceType::SoundEffect, name)
		, m_player(p_player) {
	}
}
