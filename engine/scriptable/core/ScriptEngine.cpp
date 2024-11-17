#include "core/ScriptEngine.hpp"

namespace core {
	void* ScriptEngine::getNativeHandle() {
		return handle;
	}

	ScriptEngine::ScriptEngine() : handle(nullptr) {
	}
	ScriptEngine::~ScriptEngine() {
		close();
	}

	ScriptEngine& ScriptEngine::getInstance() {
		static ScriptEngine instance;
		return instance;
	}
}
