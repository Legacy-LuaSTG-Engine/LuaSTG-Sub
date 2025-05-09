#pragma once
#include "core/ReferenceCounted.hpp"

namespace core::implement {
	class ReferenceCountedDebugger {
	public:
		static void startTracking(IReferenceCounted* object);
		static void stopTracking(IReferenceCounted* object);
		static bool hasLeak();
		static void reportLeak();
	};
}
