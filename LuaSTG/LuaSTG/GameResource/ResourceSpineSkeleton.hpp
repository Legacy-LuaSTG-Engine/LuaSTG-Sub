#pragma once
#ifdef LUASTG_SUPPORTS_SPINE

#include "GameResource/ResourceBase.hpp"
#include <spine/spine.h>

namespace luastg
{
	struct IResourceSpineSkeleton : public IResourceBase
	{
		virtual spine::SkeletonData* getSkeletonData() = 0;
		virtual spine::AnimationStateData* getAnimationStateData() = 0;
		virtual void setAnimationMix(const char* ani1, const char* ani2, float mix_time) = 0;
		virtual void setAnimationMix(float mix_time) = 0;
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceSpineSkeleton
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceSpineSkeleton>() { return UUID::parse("7ff21330-a4cc-580b-bd25-adb5bbb09c90"); }
}

#endif // LUASTG_SUPPORTS_SPINE