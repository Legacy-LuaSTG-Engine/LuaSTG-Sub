#ifdef LUASTG_SUPPORTS_SPINE
#include "GameResource/Implement/ResourceSpineSkeletonImpl.hpp"
#include "AppFrame.h"


namespace luastg
{
	ResourceSpineSkeletonImpl::ResourceSpineSkeletonImpl(const char* name, const char* skel_path, const std::shared_ptr<spine::Atlas>& atlas)
		: ResourceBaseImpl(ResourceType::SpineSkeleton, name)
		, atlas_holder(atlas)
	{
		std::string_view skeleton_path = skel_path;
		if (skeleton_path.ends_with(".json"))
		{
			auto skelJson = spine::SkeletonJson(atlas.get());
			skeleton.reset(skelJson.readSkeletonDataFile(skel_path));
		}
		else if (skeleton_path.ends_with(".skel"))
		{
			auto skelBin = spine::SkeletonBinary(atlas.get());
			skeleton.reset(skelBin.readSkeletonDataFile(skel_path));
		}
		else
		{
			spdlog::error("'{}' 不是可识别的SpineSkeleton文件.", skel_path);
			throw std::exception("spine skeleton only support .json and .skel format, check your skeleton file!");
		}

		if (skeleton.get() == nullptr) throw std::exception("load spine skeleton failed! check your spine export version and make sure it's 4.2.xx!");

		anistate.reset(new spine::AnimationStateData(skeleton.get()));
	}
	spine::SkeletonData* ResourceSpineSkeletonImpl::getSkeletonData() { return skeleton.get(); }
	spine::AnimationStateData* ResourceSpineSkeletonImpl::getAnimationStateData() { return anistate.get();  }
	void ResourceSpineSkeletonImpl::setAnimationMix(const char* ani1, const char* ani2, float mix_time)
	{ 
		auto from = skeleton->findAnimation(ani1);
		auto to = skeleton->findAnimation(ani2);
		
		if (!from || !to)
		{
			spdlog::error("SetSpineAnimationMix: 指定的骨骼 '{}' 上不存在动画 '{}' 和/或 '{}', 已取消", GetResName(), ani1, ani2);
			return;
		}

		anistate->setMix(ani1, ani2, mix_time);
	}
	void ResourceSpineSkeletonImpl::setAnimationMix(float mix_time) { anistate->setDefaultMix(mix_time); };
}
#endif