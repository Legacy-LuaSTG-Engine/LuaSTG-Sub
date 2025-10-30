#pragma once
#include "GameResource/ResourceBase.hpp"
#include <spine/spine.h>

namespace luastg
{
	struct IResourceSpineSkeleton : public IResourceBase
	{
		//virtual std::unique_ptr<spine::Skeleton> CreateInstance() = 0;

	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceSpineSkeleton
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceSpineSkeleton>() { return UUID::parse("7ff21330-a4cc-580b-bd25-adb5bbb09c90"); }
}
