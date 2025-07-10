#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
	std::string_view const ImVec4Binding::class_name{ "imgui.ImVec4"sv };

	struct ImVec4Wrapper : ImVec4Binding {
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
				case 'z':
					ctx.push_value(self->z);
					return 1;
				case 'w':
					ctx.push_value(self->w);
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
				case 'z':
					self->z = value;
					return 0;
				case 'w':
					self->w = value;
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
				*self = ImVec4();
			}
			else {
				auto const x = ctx.get_value<float>(1);
				auto const y = ctx.get_value<float>(2);
				auto const z = ctx.get_value<float>(3);
				auto const w = ctx.get_value<float>(4);
				auto const self = create(vm);
				*self = ImVec4(x, y, z, w);
			}
			return 1;
		}
	};

	bool ImVec4Binding::is(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	ImVec4* ImVec4Binding::as(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<ImVec4>(index, class_name);
	}
	ImVec4* ImVec4Binding::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImVec4>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		return self;
	}
	ImVec4* ImVec4Binding::create(lua_State* const vm, ImVec4 const& value) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImVec4>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		*self = value;
		return self;
	}
	void ImVec4Binding::registerClass(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const mt = ctx.create_metatable(class_name);
		ctx.set_map_value(mt, "__eq"sv, &ImVec4Wrapper::equals);
		ctx.set_map_value(mt, "__tostring"sv, &ImVec4Wrapper::toString);
		ctx.set_map_value(mt, "__index"sv, &ImVec4Wrapper::getter);
		ctx.set_map_value(mt, "__newindex"sv, &ImVec4Wrapper::setter);

		auto const m = ctx.push_module(module_name);
		ctx.set_map_value(m, "ImVec4"sv, &ImVec4Wrapper::constructor); // imgui.ImVec4(x:number, y:number, z:number, w:number) -> imgui.ImVec2
	}
}
