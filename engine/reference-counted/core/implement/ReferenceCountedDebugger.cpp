#include "core/implement/ReferenceCountedDebugger.hpp"
#include <cassert>
#include <mutex>
#include <set>
#include <format>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
	std::recursive_mutex object_set_lock;
	std::set<core::IReferenceCounted*> object_set;
}

namespace core::implement {
	void ReferenceCountedDebugger::startTracking(IReferenceCounted* const object) {
		[[maybe_unused]] std::scoped_lock lock(object_set_lock);
		object_set.insert(object);
	}
	void ReferenceCountedDebugger::stopTracking(IReferenceCounted* const object) {
		[[maybe_unused]] std::scoped_lock lock(object_set_lock);
		object_set.erase(object);
	}
	bool ReferenceCountedDebugger::hasLeak() {
		[[maybe_unused]] std::scoped_lock lock(object_set_lock);
		return !object_set.empty();
	}
	void ReferenceCountedDebugger::reportLeak() {
		[[maybe_unused]] std::scoped_lock lock(object_set_lock);
		if (!object_set.empty()) {
			std::string buffer;
			buffer.append(std::format("[ReferenceCountedDebugger] Detected {} objects leaking\n", object_set.size()));
			for (auto const& object : object_set) {
				buffer.append(std::format("    object 0x{:X}\n", reinterpret_cast<size_t>(object)));
			}
			std::cerr << buffer;
		#ifdef _WIN32
			OutputDebugStringA(buffer.c_str());
		#endif
			assert(false);
		}
	}
}
