#include "Core/Object.hpp"

#ifndef NDEBUG
#include <stdexcept>
#include <unordered_set>
#include <format>

namespace core {
	static std::unordered_set<IObject*> g_object_set;
	void ObjectDebugger::add(IObject* object) {
		g_object_set.insert(object);
	}
	void ObjectDebugger::remove(IObject* object) {
		g_object_set.erase(object);
	}
	void ObjectDebugger::check() {
		if (!g_object_set.empty()) {
			auto const message = std::format("there are {} objects forgot to release", g_object_set.size());
			throw std::runtime_error(message);
		}
	}
}
#endif
