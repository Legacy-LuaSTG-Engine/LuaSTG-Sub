#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Audio/Device.hpp"

namespace luastg
{
	enum class MusicRoopRangeType : int32_t
	{
		Disable,
		All,
		StartPointToEnd,
		StartPointAndLength,
		LengthAndEndPoint,
		StartToEndPoint,
		StartPointAndEndPoint,
	};

	enum class MusicRoopRangeUnit : int32_t
	{
		Sample,
		Second,
	};

	struct MusicRoopRange
	{
		MusicRoopRangeType type{ MusicRoopRangeType::All };
		MusicRoopRangeUnit unit{ MusicRoopRangeUnit::Sample };
		uint32_t start_in_samples{};
		uint32_t end_in_samples{};
		uint32_t length_in_samples{};
		double start_in_seconds{};
		double end_in_seconds{};
		double length_in_seconds{};
	};

	struct IResourceMusic : public IResourceBase
	{
		virtual core::Audio::IAudioPlayer* GetAudioPlayer() = 0;

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
		virtual void SetLoopRange(MusicRoopRange range) = 0;
	};
}
