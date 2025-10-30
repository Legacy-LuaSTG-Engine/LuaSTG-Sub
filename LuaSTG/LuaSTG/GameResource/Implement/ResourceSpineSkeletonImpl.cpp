#include "GameResource/Implement/ResourceSpineSkeletonImpl.hpp"
#include "AppFrame.h"


namespace luastg
{
	ResourceSpineSkeletonImpl::ResourceSpineSkeletonImpl(const char* name, const char* skel_path, const std::shared_ptr<spine::Atlas>& atlas)
		: ResourceBaseImpl(ResourceType::SpineSkeleton, name)
		//, atlas_holder(atlas)
	{
		std::string_view skeleton_path = skel_path;
		if (skeleton_path.ends_with(".json"))
		{
			auto skelJson = new spine::SkeletonJson(atlas.get());
			skeleton.reset(skelJson->readSkeletonDataFile(skel_path));
			delete skelJson;
		}
		else if (skeleton_path.ends_with(".skel"))
		{
			auto skelBin = new spine::SkeletonBinary(atlas.get());
			skeleton.reset(skelBin->readSkeletonDataFile(skel_path));
			delete skelBin;
		}
		else
		{
			spdlog::error("'{}' 不是可识别的SpineSkeleton文件");
		}
	}

}