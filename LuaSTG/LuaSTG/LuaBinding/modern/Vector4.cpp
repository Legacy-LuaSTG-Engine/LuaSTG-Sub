// ReSharper disable CppTooWideScopeInitStatement

#include "Vector4.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	constexpr auto embedded_script = R"(--- LuaSTG Sub built-in script
)"sv;
}

namespace luastg::binding {
	std::string_view Vector4::class_name{ "lstg.Vector4" };

	struct Vector4Binding : Vector4 {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __tostring(lua_State* vm) {
			lua::stack_t const ctx(vm);
			std::ignore = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}
		static int __index(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(1 + 1);
			if (key == "x"sv) {
				ctx.push_value(self->data.x);
			}
			else if (key == "y"sv) {
				ctx.push_value(self->data.y);
			}
			else if (key == "z"sv) {
				ctx.push_value(self->data.z);
			}
			else if (key == "w"sv) {
				ctx.push_value(self->data.w);
			}
			else {
				auto const method_table = ctx.push_module(class_name);
				ctx.push_map_value(method_table, key);
				if (ctx.is_nil(ctx.index_of_top())) {
					return luaL_error(vm, "field '%s' not exists", key.data());
				}
			}
			return 1;
		}
		static int __newindex(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(1 + 1);
			auto const value = ctx.get_value<lua_Number>(1 + 2);
			if (key == "x"sv) {
				self->data.x = value;
			}
			else if (key == "y"sv) {
				self->data.y = value;
			}
			else if (key == "z"sv) {
				self->data.z = value;
			}
			else if (key == "w"sv) {
				self->data.w = value;
			}
			else {
				return luaL_error(vm, "field '%s' not exists", key.data());
			}
			return 1;
		}

		static int __eq(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(self->data == other->data);
			}
			else {
				ctx.push_value(false);
			}
			return 1;
		}
		static int __add(lua_State* vm) {
			lua::stack_t const ctx(vm);
			if (is(vm, 1)) {
				auto const left = as(vm, 1);
				if (is(vm, 2)) {
					auto const right = as(vm, 2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data + right->data;
				}
				else {
					auto const right = ctx.get_value<lua_Number>(2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data + right;
				}
			}
			else {
				auto const right = as(vm, 2);
				if (is(vm, 1)) {
					auto const left = as(vm, 1);
					auto const copy = Vector4::create(vm);
					copy->data = left->data + right->data;
				}
				else {
					auto const left = ctx.get_value<lua_Number>(1);
					auto const copy = Vector4::create(vm);
					copy->data = core::Vector4(left, left, left, left) + right->data;
				}
			}
			return 1;
		}
		static int __sub(lua_State* vm) {
			lua::stack_t const ctx(vm);
			if (is(vm, 1)) {
				auto const left = as(vm, 1);
				if (is(vm, 2)) {
					auto const right = as(vm, 2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data - right->data;
				}
				else {
					auto const right = ctx.get_value<lua_Number>(2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data - right;
				}
			}
			else {
				auto const right = as(vm, 2);
				if (is(vm, 1)) {
					auto const left = as(vm, 1);
					auto const copy = Vector4::create(vm);
					copy->data = left->data - right->data;
				}
				else {
					auto const left = ctx.get_value<lua_Number>(1);
					auto const copy = Vector4::create(vm);
					copy->data = core::Vector4(left, left, left, left) - right->data;
				}
			}
			return 1;
		}
		static int __mul(lua_State* vm) {
			lua::stack_t const ctx(vm);
			if (is(vm, 1)) {
				auto const left = as(vm, 1);
				if (is(vm, 2)) {
					auto const right = as(vm, 2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data * right->data;
				}
				else {
					auto const right = ctx.get_value<lua_Number>(2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data * right;
				}
			}
			else {
				auto const right = as(vm, 2);
				if (is(vm, 1)) {
					auto const left = as(vm, 1);
					auto const copy = Vector4::create(vm);
					copy->data = left->data * right->data;
				}
				else {
					auto const left = ctx.get_value<lua_Number>(1);
					auto const copy = Vector4::create(vm);
					copy->data = core::Vector4(left, left, left, left) * right->data;
				}
			}
			return 1;
		}
		static int __div(lua_State* vm) {
			lua::stack_t const ctx(vm);
			if (is(vm, 1)) {
				auto const left = as(vm, 1);
				if (is(vm, 2)) {
					auto const right = as(vm, 2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data / right->data;
				}
				else {
					auto const right = ctx.get_value<lua_Number>(2);
					auto const copy = Vector4::create(vm);
					copy->data = left->data / right;
				}
			}
			else {
				auto const right = as(vm, 2);
				if (is(vm, 1)) {
					auto const left = as(vm, 1);
					auto const copy = Vector4::create(vm);
					copy->data = left->data / right->data;
				}
				else {
					auto const left = ctx.get_value<lua_Number>(1);
					auto const copy = Vector4::create(vm);
					copy->data = core::Vector4(left, left, left, left) / right->data;
				}
			}
			return 1;
		}
		static int __unm(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const copy = Vector4::create(vm);
			copy->data = -self->data;
			return 1;
		}

		// NOLINTEND(*-reserved-identifier)

		// method

		static int length(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data.length());
			return 1;
		}
		static int normalize(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->data.normalize();
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int normalized(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const copy = Vector4::create(vm);
			copy->data = self->data.normalized();
			return 1;
		}
		static int dot(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const other = as(vm, 2);
			ctx.push_value(self->data.dot(other->data));
			return 1;
		}

		// static method

		static int create(lua_State* vm) {
			lua::stack_t const ctx(vm);
			if (ctx.index_of_top() >= 2) {
				auto const x = ctx.get_value<lua_Number>(1);
				auto const y = ctx.get_value<lua_Number>(2);
				auto const z = ctx.get_value<lua_Number>(3);
				auto const w = ctx.get_value<lua_Number>(4);
				auto const self = Vector4::create(vm);
				self->data = core::Vector4(x, y, z, w);
			}
			else {
				std::ignore = Vector4::create(vm);
			}
			return 1;
		}

	};

	bool Vector4::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	Vector4* Vector4::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<Vector4>(index);
	}
	Vector4* Vector4::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<Vector4>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = {};
		return self;
	}
	void Vector4::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "length", &Vector4Binding::length);
		ctx.set_map_value(method_table, "normalize", &Vector4Binding::normalize);
		ctx.set_map_value(method_table, "normalized", &Vector4Binding::normalized);
		ctx.set_map_value(method_table, "dot", &Vector4Binding::dot);
		ctx.set_map_value(method_table, "create", &Vector4Binding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__tostring", &Vector4Binding::__tostring);
		ctx.set_map_value(metatable, "__index", &Vector4Binding::__index);
		ctx.set_map_value(metatable, "__newindex", &Vector4Binding::__newindex);
		ctx.set_map_value(metatable, "__eq", &Vector4Binding::__eq);
		ctx.set_map_value(metatable, "__add", &Vector4Binding::__add);
		ctx.set_map_value(metatable, "__sub", &Vector4Binding::__sub);
		ctx.set_map_value(metatable, "__mul", &Vector4Binding::__mul);
		ctx.set_map_value(metatable, "__div", &Vector4Binding::__div);
		ctx.set_map_value(metatable, "__unm", &Vector4Binding::__unm);

		// embedded script

		if (LUA_OK == luaL_loadbuffer(vm, embedded_script.data(), embedded_script.size(), "lstg/Vector4.lua")) {
			lua_call(vm, 0, 0);
		}
	}
}
