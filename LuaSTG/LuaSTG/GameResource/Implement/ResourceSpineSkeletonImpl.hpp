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

	public:
		spine::SkeletonData* getSkeletonData();
	public:
		ResourceSpineSkeletonImpl(const char* name, const char* skelPath, const std::shared_ptr<spine::Atlas>& atlas);
	};
}