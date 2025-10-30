#pragma once
#include "GameResource/ResourceSpineSkeleton.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include <spine/spine.h>

namespace luastg
{
	class ResourceSpineSkeletonImpl : public ResourceBaseImpl<IResourceSpineSkeleton>
	{
	private:
		std::shared_ptr<spine::Atlas> atlas_holder;
		std::unique_ptr<spine::SkeletonData> skeleton;
		std::unique_ptr<spine::AnimationStateData> anistate;

	public:
		spine::SkeletonData* getSkeletonData();
		spine::AnimationStateData* getAnimationStateData();
		void setAnimationMix(const char* ani1, const char* ani2, float mix_time);
		void setAnimationMix(float mix_time);
	public:
		ResourceSpineSkeletonImpl(const char* name, const char* skelPath, const std::shared_ptr<spine::Atlas>& atlas);
	};
}