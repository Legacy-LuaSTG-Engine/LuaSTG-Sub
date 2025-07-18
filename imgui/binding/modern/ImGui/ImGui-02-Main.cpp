#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
	int GetStyle(lua_State* const vm) {
		imgui::binding::ImGuiStyleBinding::reference(vm, &ImGui::GetStyle());
		return 1;
	}
	int NewFrame(lua_State*) {
		ImGui::NewFrame();
		return 0;
	}
	int EndFrame(lua_State*) {
		ImGui::EndFrame();
		return 0;
	}
	int Render(lua_State*) {
		ImGui::Render();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiMain(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "GetIO"sv, &notSupported);
		ctx.set_map_value(m, "GetPlatformIO"sv, &notSupported);
		ctx.set_map_value(m, "GetStyle"sv, &GetStyle);
		ctx.set_map_value(m, "NewFrame"sv, &NewFrame);
		ctx.set_map_value(m, "EndFrame"sv, &EndFrame);
		ctx.set_map_value(m, "Render"sv, &Render);
		ctx.set_map_value(m, "GetDrawData"sv, &notSupported);
	}
}
