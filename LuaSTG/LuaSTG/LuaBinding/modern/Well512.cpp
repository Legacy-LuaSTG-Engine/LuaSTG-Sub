#include "Well512.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace luastg::binding {
	std::string_view const Well512::class_name{ "lstg.Rand" };

	struct Well512Binding : Well512 {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __tostring(lua_State* vm) {
			lua::stack_t const ctx(vm);
			std::ignore = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}

		// NOLINTEND(*-reserved-identifier)

		// method

		static int seed(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const seed = ctx.get_value<uint32_t>(2);
			self->seed(seed);
			return 0;
		}
		static int getSeed(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->seed());
			return 1;
		}
		static int integer(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto a = ctx.get_value<int32_t>(2);
			auto b = ctx.get_value<int32_t>(3);
			if (a > b) {
				std::swap(a, b);
			}
			if (auto const range = static_cast<int64_t>(b) - static_cast<int64_t>(a); range > 0x7fffffff) {
				auto const message = std::format("range [a:{}, b:{}] too large, (b - a) must <= 2147483647", a, b);
				return luaL_error(vm, message.c_str());
			}
			auto const result = self->integer(a, b);
			ctx.push_value(result);
			return 1;
		}
		static int number(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto a = ctx.get_value<float>(2);
			auto b = ctx.get_value<float>(3);
			if (a > b) {
				std::swap(a, b);
			}
			auto const result = self->number(a, b);
			ctx.push_value(result);
			return 1;
		}
		static int sign(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const result = self->sign();
			ctx.push_value(result);
			return 1;
		}
		static int clone(lua_State* const vm) {
			auto const self = as(vm, 1);
			auto const other = Well512::create(vm);
			*other = *self; // copy state
			return 1;
		}
		static int serialize(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const result = self->serialize();
			ctx.push_value(result);
			return 1;
		}
		static int deserialize(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const data = ctx.get_value<std::string_view>(2);
			auto const result = self->deserialize(data);
			ctx.push_value(result);
			return 1;
		}

		// static method

		static int create(lua_State* const vm) {
			Well512::create(vm);
			return 1;
		}

	};

	bool Well512::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	random::well512* Well512::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		auto const userdata = ctx.as_userdata<Well512>(index);
		return &userdata->well512;
	}
	random::well512* Well512::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<Well512>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		new(self) Well512();
		return &self->well512;
	}
	void Well512::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "seed", &Well512Binding::seed);
		ctx.set_map_value(method_table, "integer", &Well512Binding::integer);
		ctx.set_map_value(method_table, "number", &Well512Binding::number);
		ctx.set_map_value(method_table, "sign", &Well512Binding::sign);
		ctx.set_map_value(method_table, "clone", &Well512Binding::clone);
		ctx.set_map_value(method_table, "serialize", &Well512Binding::serialize);
		ctx.set_map_value(method_table, "deserialize", &Well512Binding::deserialize);
		ctx.set_map_value(method_table, "create", &Well512Binding::create);

		ctx.set_map_value(method_table, "Seed", &Well512Binding::seed);
		ctx.set_map_value(method_table, "GetSeed", &Well512Binding::getSeed);
		ctx.set_map_value(method_table, "Int", &Well512Binding::integer);
		ctx.set_map_value(method_table, "Float", &Well512Binding::number);
		ctx.set_map_value(method_table, "Sign", &Well512Binding::sign);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__tostring", &Well512Binding::__tostring);
		ctx.set_map_value(metatable, "__index", method_table);

		// legacy constructor
		auto const lstg_table = ctx.push_module("lstg"sv);
		ctx.set_map_value(lstg_table, "Rand"sv, &Well512Binding::create);
	}
}
