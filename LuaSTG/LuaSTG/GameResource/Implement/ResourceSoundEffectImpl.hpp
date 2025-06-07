#pragma once
#include "GameResource/ResourceSoundEffect.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include "core/AudioPlayer.hpp"

namespace luastg {
	class ResourceSoundEffectImpl final : public ResourceBaseImpl<IResourceSoundEffect> {
	public:
		void FlushCommand() override;
		void Play(float vol, float pan) override;
		void Resume() override;
		void Pause() override;
		void Stop() override;
		bool IsPlaying() override;
		bool IsStopped() override;
		bool SetSpeed(float speed) override;
		float GetSpeed() override;

		ResourceSoundEffectImpl(const char* name, core::IAudioPlayer* p_player);

	private:
		enum class CommandType : uint8_t {
			None,
			Play,
			Stop,
			Reset,
			ResetAndStop,
		};

		struct Command {
			CommandType type = CommandType::None;
			float vol = 0.0f;
			float pan = 0.0f;
		};

		core::SmartReference<core::IAudioPlayer> m_player;
		int m_status = 0; // 0停止 1暂停 2播放
		Command m_last_command;
	};
}
