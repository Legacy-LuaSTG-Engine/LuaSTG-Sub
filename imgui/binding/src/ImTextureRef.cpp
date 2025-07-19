#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
	std::string_view const ImTextureRefBinding::class_name{ "imgui.ImTextureRef"sv };

	struct ImTextureRefWrapper : ImTextureRefBinding {
		static int equals(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(self->GetTexID() == other->GetTexID());
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

		static int GetTexID(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const value = self->GetTexID();
			auto const marshalled = ImTextureIDUtils::marshal(value);
			ctx.push_value(marshalled);
			return 1;
		}

		static int constructor(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			if (ctx.index_of_top() == 0) {
				auto const self = create(vm);
				*self = ImTextureRef();
			}
			else {
				auto const value = ctx.get_value<std::string_view>(1);
				auto const unmarshalled = ImTextureIDUtils::unmarshal(value);
				auto const self = create(vm);
				*self = ImTextureRef(unmarshalled);
			}
			return 1;
		}
	};

	bool ImTextureRefBinding::is(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	ImTextureRef* ImTextureRefBinding::as(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<ImTextureRef>(index, class_name);
	}
	ImTextureRef* ImTextureRefBinding::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImTextureRef>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		return self;
	}
	ImTextureRef* ImTextureRefBinding::create(lua_State* const vm, ImTextureRef const& value) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<ImTextureRef>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		*self = value;
		return self;
	}
	void ImTextureRefBinding::registerClass(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const c = ctx.create_map();
		ctx.set_map_value(c, "GetTexID"sv, &ImTextureRefWrapper::GetTexID);

		auto const mt = ctx.create_metatable(class_name);
		ctx.set_map_value(mt, "__eq"sv, &ImTextureRefWrapper::equals);
		ctx.set_map_value(mt, "__tostring"sv, &ImTextureRefWrapper::toString);
		ctx.set_map_value(mt, "__index"sv, c);

		auto const m = ctx.push_module(module_name);
		ctx.set_map_value(m, "ImTextureRef"sv, &ImTextureRefWrapper::constructor);
	}
}
