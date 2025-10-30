#pragma once
#include "GameResource/ResourceBase.hpp"
#include "spine/Extension.h"

namespace luastg
{
	struct IResourceSpine : public IResourceBase
	{
        
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceSpine
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceSpine>() { return UUID::parse("8f0a4281-6b0e-5db5-8cf4-2315eedf0c0f"); }
}
