#pragma once
#ifndef NDEBUG
#include "core/ReferenceCounted.hpp"

namespace core::implement {
	class ReferenceCountedDebugger {
	public:
		static void startTracking(IReferenceCounted* const object);
		static void stopTracking(IReferenceCounted* const object);
		static void reportLeak();
	};
}
#endif
