#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Audio/Device.hpp"

namespace LuaSTGPlus
{
	struct IResourceMusic : public IResourceBase
	{
		virtual Core::Audio::IAudioPlayer* GetAudioPlayer() = 0;

		virtual void Play(float vol, double position) = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual bool IsPlaying() = 0;
		virtual bool IsPaused() = 0;
		virtual bool IsStopped() = 0;
		virtual void SetVolume(float v) = 0;
		virtual float GetVolume() = 0;
		virtual bool SetSpeed(float speed) = 0;
		virtual float GetSpeed() = 0;
		virtual void SetLoop(bool v) = 0;
	};
}
