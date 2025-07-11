#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
	// BLACK MAGIC!!!

	// Dear ImGui v1.92.1 call hierarchical:
	// - .NextWindowData.SizeCallback
	//     - CalcWindowSizeAfterConstraint
	//         - CalcWindowAutoFitSize
	//             - Begin
	//             - CalcWindowNextAutoFitSize
	//         - CalcWindowNextAutoFitSize
	//             - BeginComboPopup
	//                 - BeginCombo
	//         - CalcResizePosSizeFromAnyCorner
	//             - UpdateWindowManualResize
	//         - UpdateWindowManualResize
	//             - Begin
	//         - Begin

	// Summary:
	// - Begin
	// - BeginCombo

	struct ImGuiSizeCallbackDataBinding {
		static constexpr auto class_name{ "imgui.ImGuiSizeCallbackData"sv };

		static int toString(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}
		static int getter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(2);

		#define GET_VEC2(name) if (key == (#name ""sv)) { ImVec2Binding::create(vm, (*self)->name); return 1; } (void)0

			if (key == "UserData"sv) {
				return luaL_error(vm, "not supported");
			}
			GET_VEC2(Pos);
			GET_VEC2(CurrentSize);
			GET_VEC2(DesiredSize);

		#undef GET_VEC2

			return luaL_error(vm, "field '%s' does not exist", key.data());
		}
		static int setter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(2);

		#define SET_VEC2(name) if (key == (#name ""sv)) { (*self)->name = *ImVec2Binding::as(vm, 3); return 0; } (void)0
		#define SET_READONLY(name) if (key == (#name ""sv)) { return luaL_error(vm, "field '%s' is read-only", key.data()); } (void)0

			SET_READONLY(UserData);
			SET_READONLY(Pos);
			SET_READONLY(CurrentSize);
			SET_VEC2(DesiredSize);

		#undef SET_READONLY
		#undef SET_VEC2

			return luaL_error(vm, "field '%s' does not exist", key.data());
		}

		static bool is(lua_State* const vm, int const index) {
			lua::stack_t const ctx(vm);
			return ctx.is_metatable(index, class_name);
		}
		static ImGuiSizeCallbackData** as(lua_State* const vm, int const index) {
			lua::stack_t const ctx(vm);
			return ctx.as_userdata<ImGuiSizeCallbackData*>(index, class_name);
		}
		static ImGuiSizeCallbackData** reference(lua_State* const vm, ImGuiSizeCallbackData* const value) {
			lua::stack_t const ctx(vm);
			auto const self = ctx.create_userdata<ImGuiSizeCallbackData*>();
			auto const self_index = ctx.index_of_top();
			ctx.set_metatable(self_index, class_name);
			*self = value;
			return self;
		}
		static void registerClass(lua_State* const vm) {
			lua::stack_balancer_t const sb(vm);
			lua::stack_t const ctx(vm);

			auto const mt = ctx.create_metatable(class_name);
			ctx.set_map_value(mt, "__tostring"sv, &ImGuiSizeCallbackDataBinding::toString);
			ctx.set_map_value(mt, "__index"sv, &ImGuiSizeCallbackDataBinding::getter);
			ctx.set_map_value(mt, "__newindex"sv, &ImGuiSizeCallbackDataBinding::setter);
		}
	};

	struct ImGuiSizeCallbackWrapper {
		static inline size_t key{};
		static inline lua_State* vm{};

		static void callback(ImGuiSizeCallbackData* const data) {
			if (vm == nullptr) {
				assert(false); // unlikely
				return;
			}
			lua_pushlightuserdata(vm, &ImGuiSizeCallbackWrapper::key);
			lua_gettable(vm, LUA_REGISTRYINDEX);
			if (auto const c = lua_gettop(vm); lua_isfunction(vm, c)) {
				ImGuiSizeCallbackDataBinding::reference(vm, data);
				lua_call(vm, 1, 0);
			}
			else {
				lua_pop(vm, 1);
			}
		}
	};

	void beginSetNextWindowSizeConstraintsCallbackWrapper(lua_State* const vm) {
		ImGuiSizeCallbackWrapper::vm = vm;
	}
	void endSetNextWindowSizeConstraintsCallbackWrapper(lua_State* const vm) {
		ImGuiSizeCallbackWrapper::vm = nullptr;
		// cleanup
		lua_pushlightuserdata(vm, &ImGuiSizeCallbackWrapper::key);
		lua_pushnil(vm);
		lua_settable(vm, LUA_REGISTRYINDEX);
	}
}

namespace {
	int SetNextWindowPos(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const pos = imgui::binding::ImVec2Binding::as(vm, 1);
		auto const cond = ctx.get_value<ImGuiCond>(2, 0);
		if (ctx.index_of_top() >= 3) {
			auto const pivot = imgui::binding::ImVec2Binding::as(vm, 3);
			ImGui::SetNextWindowPos(*pos, cond, *pivot);
		}
		else {
			ImGui::SetNextWindowPos(*pos, cond);
		}
		return 0;
	}
	int SetNextWindowSize(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const pos = imgui::binding::ImVec2Binding::as(vm, 1);
		auto const cond = ctx.get_value<ImGuiCond>(2, 0);
		ImGui::SetNextWindowSize(*pos, cond);
		return 0;
	}
	int SetNextWindowSizeConstraints(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const size_min = imgui::binding::ImVec2Binding::as(vm, 1);
		auto const size_max = imgui::binding::ImVec2Binding::as(vm, 2);
		if (ctx.index_of_top() >= 3) {
			if (!ctx.is_function(3)) {
				return luaL_typerror(vm, 3, "function");
			}
			lua_pushlightuserdata(vm, &imgui::binding::ImGuiSizeCallbackWrapper::key);
			lua_pushvalue(vm, 3);
			lua_settable(vm, LUA_REGISTRYINDEX);
			ImGui::SetNextWindowSizeConstraints(*size_min, *size_max, &imgui::binding::ImGuiSizeCallbackWrapper::callback);
		}
		else {
			ImGui::SetNextWindowSizeConstraints(*size_min, *size_max);
		}
		return 0;
	}
	int SetNextWindowContentSize(lua_State* const vm) {
		auto const size = imgui::binding::ImVec2Binding::as(vm, 1);
		ImGui::SetNextWindowContentSize(*size);
		return 0;
	}
	int SetNextWindowCollapsed(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const collapsed = ctx.get_value<bool>(1);
		auto const cond = ctx.get_value<ImGuiCond>(2, 0);
		ImGui::SetNextWindowCollapsed(collapsed, cond);
		return 0;
	}
	int SetNextWindowFocus(lua_State*) {
		ImGui::SetNextWindowFocus();
		return 0;
	}
	int SetNextWindowScroll(lua_State* const vm) {
		auto const scroll = imgui::binding::ImVec2Binding::as(vm, 1);
		ImGui::SetNextWindowScroll(*scroll);
		return 0;
	}
	int SetNextWindowBgAlpha(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const alpha = ctx.get_value<float>(1);
		ImGui::SetNextWindowBgAlpha(alpha);
		return 0;
	}
	int SetWindowPos(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_string(1)) {
			auto const name = ctx.get_value<std::string_view>(1);
			auto const pos = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const cond = ctx.get_value<ImGuiCond>(3, 0);
			ImGui::SetWindowPos(name.data(), *pos, cond);
		}
		else {
			auto const pos = imgui::binding::ImVec2Binding::as(vm, 1);
			auto const cond = ctx.get_value<ImGuiCond>(2, 0);
			ImGui::SetWindowPos(*pos, cond);
		}
		return 0;
	}
	int SetWindowSize(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_string(1)) {
			auto const name = ctx.get_value<std::string_view>(1);
			auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const cond = ctx.get_value<ImGuiCond>(3, 0);
			ImGui::SetWindowSize(name.data(), *size, cond);
		}
		else {
			auto const size = imgui::binding::ImVec2Binding::as(vm, 1);
			auto const cond = ctx.get_value<ImGuiCond>(2, 0);
			ImGui::SetWindowSize(*size, cond);
		}
		return 0;
	}
	int SetWindowCollapsed(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_string(1)) {
			auto const name = ctx.get_value<std::string_view>(1);
			auto const collapsed = ctx.get_value<bool>(2);
			auto const cond = ctx.get_value<ImGuiCond>(3, 0);
			ImGui::SetWindowCollapsed(name.data(), collapsed, cond);
		}
		else {
			auto const collapsed = ctx.get_value<bool>(1);
			auto const cond = ctx.get_value<ImGuiCond>(2, 0);
			ImGui::SetWindowCollapsed(collapsed, cond);
		}
		return 0;
	}
	int SetWindowFocus(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_string(1)) {
			auto const name = ctx.get_value<std::string_view>(1);
			ImGui::SetWindowFocus(name.data());
		}
		else {
			ImGui::SetWindowFocus();
		}
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWindowManipulation(lua_State* const vm) {
		ImGuiSizeCallbackDataBinding::registerClass(vm);

		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "SetNextWindowPos"sv, &SetNextWindowPos);
		ctx.set_map_value(m, "SetNextWindowSize"sv, &SetNextWindowSize);
		ctx.set_map_value(m, "SetNextWindowSizeConstraints"sv, &SetNextWindowSizeConstraints);
		ctx.set_map_value(m, "SetNextWindowContentSize"sv, &SetNextWindowContentSize);
		ctx.set_map_value(m, "SetNextWindowCollapsed"sv, &SetNextWindowCollapsed);
		ctx.set_map_value(m, "SetNextWindowFocus"sv, &SetNextWindowFocus);
		ctx.set_map_value(m, "SetNextWindowScroll"sv, &SetNextWindowScroll);
		ctx.set_map_value(m, "SetNextWindowBgAlpha"sv, &SetNextWindowBgAlpha);
		ctx.set_map_value(m, "SetWindowPos"sv, &SetWindowPos);
		ctx.set_map_value(m, "SetWindowSize"sv, &SetWindowSize);
		ctx.set_map_value(m, "SetWindowCollapsed"sv, &SetWindowCollapsed);
		ctx.set_map_value(m, "SetWindowFocus"sv, &SetWindowFocus);
	}
}
