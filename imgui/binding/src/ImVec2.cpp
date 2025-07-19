#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
	std::string_view const ImVec2Binding::class_name{ "imgui.ImVec2"sv };

	struct ImVec2Wrapper : ImVec2Binding {
		static int equals(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(*self == *other);
			}
			else {
				ctx.push_value(false);
			}
			return 1;
		}
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
			if (key.length() == 1) {
				switch (key[0]) {
				case 'x':
					ctx.push_value(self->x);
					return 1;
				case 'y':
					ctx.push_value(self->y);
					return 1;
				default:
					break;
				}
			}
			return luaL_error(vm, "field '%s' does not exist", key.data());
		}
		static int setter(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(2);
			auto const value = ctx.get_value<float>(3);
			if (key.length() == 1) {
				switch (key[0]) {
				case 'x':
					self->x = value;
					return 0;
				case 'y':
					self->y = value;
					return 0;
				default:
					break;
				}
			}
			return luaL_error(vm, "field '%s' does not exist", key.data());
		}

		static int constructor(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			if (ctx.index_of_top() == 0) {
				auto const self = create(vm);
				*self = ImVec2();
			}
			else {
				auto const x = ctx.get_value<float>(1);
				auto const y = ctx.get_value<float>(2);
				auto const self = create(vm);
				*self = ImVec2(x, y);
			}
			return 1;
		}
	};

	bool ImVec2Binding::is(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	ImVec2* ImVec2Binding::as(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<ImVec2>(index, class_name);
	}
	ImVec2 const* ImVec2Binding::as(lua_State* vm, int index, ImVec2 const& default_value) {
		lua::stack_t const ctx(vm);
		if (ctx.has_value(index)) {
			return as(vm, index);
		}
		return &default_value;
	}
	ImVec2* ImVec2Binding::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImVec2>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		return self;
	}
	ImVec2* ImVec2Binding::create(lua_State* const vm, ImVec2 const& value) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImVec2>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		*self = value;
		return self;
	}
	void ImVec2Binding::registerClass(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const mt = ctx.create_metatable(class_name);
		ctx.set_map_value(mt, "__eq"sv, &ImVec2Wrapper::equals);
		ctx.set_map_value(mt, "__tostring"sv, &ImVec2Wrapper::toString);
		ctx.set_map_value(mt, "__index"sv, &ImVec2Wrapper::getter);
		ctx.set_map_value(mt, "__newindex"sv, &ImVec2Wrapper::setter);

		auto const m = ctx.push_module(module_name);
		ctx.set_map_value(m, "ImVec2"sv, &ImVec2Wrapper::constructor); // imgui.ImVec2(x:number, y:number) -> imgui.ImVec2
	}
}
