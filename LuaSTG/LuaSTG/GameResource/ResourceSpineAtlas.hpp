#pragma once
#include "GameResource/ResourceBase.hpp"

namespace luastg
{
	struct IResourceSpineAtlas : public IResourceBase
	{
        
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceSpineAtlas
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceSpineAtlas>() { return UUID::parse("a7995836-0ed4-57ee-a014-417162b8541d"); }
}
