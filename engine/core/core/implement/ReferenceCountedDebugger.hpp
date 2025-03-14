#pragma once
#ifndef NDEBUG
#include "core/ReferenceCounted.hpp"

namespace core::implement {
	class ReferenceCountedDebugger {
	public:
		static void startTracking(IReferenceCounted* object);
		static void stopTracking(IReferenceCounted* object);
		static void reportLeak();
	};
}
#endif
