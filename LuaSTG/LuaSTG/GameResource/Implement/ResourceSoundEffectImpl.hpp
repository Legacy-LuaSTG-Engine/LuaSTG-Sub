#pragma once
#include "GameResource/ResourceSoundEffect.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include "Core/Audio/Device.hpp"

namespace luastg
{
	class ResourceSoundEffectImpl : public ResourceBaseImpl<IResourceSoundEffect>
	{
	private:
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
		ResourceSoundEffectImpl(const char* name, Core::Audio::IAudioPlayer* p_player);
	};
}
