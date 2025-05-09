#pragma once
#include "core/WeakReference.hpp"

namespace core {
	struct CORE_NO_VIRTUAL_TABLE IWeakReferenceSource : IReferenceCounted {
		virtual void getWeakReference(IWeakReference** output) = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IWeakReferenceSource
	template<> constexpr InterfaceId getInterfaceId<IWeakReferenceSource>() { return UUID::parse("beabd630-bc16-5df7-b4f1-595f4c55d1a4"); }
}
