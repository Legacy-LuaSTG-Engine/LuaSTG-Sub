#pragma once
#include "core/WeakReference.hpp"

namespace core {
	struct CORE_NO_VIRTUAL_TABLE IWeakReferenceSource : IReferenceCounted {
		virtual void getWeakReference(IWeakReference** output) = 0;
	};

	template<> constexpr UUID uuid_of<IWeakReferenceSource>() { return CORE_UUID_INITIALIZER(5def4b09, 9a61, 53e5, 9519, 73, da, ff, 8d, 9d, 88); }
}
