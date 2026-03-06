#pragma once
#include "core/SmartReference.hpp"
#include "GameResource/ResourceMusic.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include "core/AudioDecoder.hpp"
#include "core/AudioPlayer.hpp"

namespace luastg {
	class ResourceMusicImpl final : public ResourceBaseImpl<IResourceMusic> {
	public:
		core::IAudioPlayer* GetAudioPlayer() override { return m_player.get(); }

		void Play(float vol, double position) override;
		void Stop() override;
		void Pause() override;
		void Resume() override;
		bool IsPlaying() override;
		bool IsPaused() override;
		bool IsStopped() override;
		void SetVolume(float v) override;
		float GetVolume() override;
		bool SetSpeed(float speed) override;
		float GetSpeed() override;
		void SetLoop(bool v) override;
		void SetLoopRange(MusicRoopRange range) override;

		ResourceMusicImpl(const char* name, core::IAudioDecoder* decoder, core::IAudioPlayer* p_player);

	private:
		core::SmartReference<core::IAudioDecoder> m_decoder;
		core::SmartReference<core::IAudioPlayer> m_player;
	};
}
