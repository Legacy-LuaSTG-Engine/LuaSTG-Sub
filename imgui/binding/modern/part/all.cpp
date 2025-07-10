#include "lua_imgui_binding.hpp"

namespace imgui::binding {
	void registerAll(lua_State* const vm) {
		registerGlobal(vm);

		ImVec2Binding::registerClass(vm);

		ImVec4Binding::registerClass(vm);

		ImTextureRefBinding::registerClass(vm);

		registerImGuiContextCreationAndAccess(vm);
	}
}
