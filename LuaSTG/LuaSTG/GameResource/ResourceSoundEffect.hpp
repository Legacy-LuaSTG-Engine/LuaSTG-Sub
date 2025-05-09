#pragma once
#include "GameResource/ResourceBase.hpp"

namespace luastg
{
	struct IResourceSoundEffect : public IResourceBase
	{
		virtual void FlushCommand() = 0;
		virtual void Play(float vol, float pan) = 0;
		virtual void Resume() = 0;
		virtual void Pause() = 0;
		virtual void Stop() = 0;
		virtual bool IsPlaying() = 0;
		virtual bool IsStopped() = 0;
		virtual bool SetSpeed(float speed) = 0;
		virtual float GetSpeed() = 0;
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceSoundEffect
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceSoundEffect>() { return UUID::parse("300c1295-98f3-5f6d-b285-d0be8d5ae3a2"); }
}
