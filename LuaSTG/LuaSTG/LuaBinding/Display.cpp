#include "Display.hpp"
#include "lua_utility.hpp"

static void pushSize(lua_State* L, lua_Number const width, lua_Number const height) {
	lua::stack_t S(L);
	auto const index = S.create_map(2);
	S.set_map_value(index, "width", width);
	S.set_map_value(index, "height", height);
}

static void pushPoint(lua_State* L, lua_Number const x, lua_Number const y) {
	lua::stack_t S(L);
	auto const index = S.create_map(2);
	S.set_map_value(index, "x", x);
	S.set_map_value(index, "y", y);
}

static void pushRect(lua_State* L, lua_Number const left, lua_Number const top, lua_Number const right, lua_Number const bottom) {
	lua::stack_t S(L);
	auto const index = S.create_map(4);
	S.set_map_value(index, "left", left);
	S.set_map_value(index, "top", top);
	S.set_map_value(index, "right", right);
	S.set_map_value(index, "bottom", bottom);
}

namespace LuaSTG::Sub::LuaBinding {

	std::string_view Display::class_name{ "lstg.Display" };

	struct DisplayBinding : public Display {

		// meta methods

		static int __gc(lua_State* L) {
			[[maybe_unused]] auto self = as(L, 1); // TODO
			return 0;
		}

		static int __tostring(lua_State* L) {
			lua::stack_t S(L);
			[[maybe_unused]] auto self = as(L, 1);
			S.push_value(class_name);
			return 1;
		}

		static int __eq(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			if (is(L, 2)) {
				auto other = as(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		// instance methods

		static int getSize(lua_State* L) {
			pushSize(L, 0.0, 0.0); // TODO
			return 1;
		}

		static int getPosition(lua_State* L) {
			pushPoint(L, 0.0, 0.0); // TODO
			return 1;
		}

		static int getRect(lua_State* L) {
			pushRect(L, 0.0, 0.0, 0.0, 0.0); // TODO
			return 1;
		}

		static int getWorkAreaSize(lua_State* L) {
			pushSize(L, 0.0, 0.0); // TODO
			return 1;
		}

		static int getWorkAreaPosition(lua_State* L) {
			pushPoint(L, 0.0, 0.0); // TODO
			return 1;
		}

		static int getWorkAreaRect(lua_State* L) {
			pushRect(L, 0.0, 0.0, 0.0, 0.0); // TODO
			return 1;
		}

		static int isPrimary(lua_State* L) {
			lua::stack_t S(L);
			S.push_value(false); // TODO
			return 1;
		}

		static int getDisplayScale(lua_State* L) {
			lua::stack_t S(L);
			S.push_value(0.0); // TODO
			return 1;
		}

		// static methods

		static int getAll(lua_State* L) {
			lua::stack_t S(L);
			S.create_array(); // TODO
			return 1;
		}

		static int getPrimary(lua_State* L) {
			lua::stack_t S(L);
			S.push_value(std::nullopt); // TODO
			return 1;
		}

		static int getNearestFromWindow(lua_State* L) {
			lua::stack_t S(L);
			S.push_value(std::nullopt); // TODO
			return 1;
		}

	};

	bool Display::is(lua_State* L, int index) {
		return nullptr != luaL_testudata(L, index, class_name.data());
	}

	Display* Display::as(lua_State* L, int index) {
		return static_cast<Display*>(luaL_checkudata(L, index, class_name.data()));
	}

	Display* Display::create(lua_State* L)
	{
		lua::stack_t S(L);
		auto self = S.create_userdata<Display>();
		auto const self_index = S.index_of_top();
		S.set_metatable(self_index, class_name);
		return self;
	}

	void Display::registerClass(lua_State* L) {
		[[maybe_unused]] lua::stack_balancer_t SB(L);
		lua::stack_t S(L);

		// method

		auto const method_table = S.create_map();
		S.set_map_value(method_table, "getSize", &DisplayBinding::getSize);
		S.set_map_value(method_table, "getPosition", &DisplayBinding::getPosition);
		S.set_map_value(method_table, "getRect", &DisplayBinding::getRect);
		S.set_map_value(method_table, "getWorkAreaSize", &DisplayBinding::getWorkAreaSize);
		S.set_map_value(method_table, "getWorkAreaPosition", &DisplayBinding::getWorkAreaPosition);
		S.set_map_value(method_table, "getWorkAreaRect", &DisplayBinding::getWorkAreaRect);
		S.set_map_value(method_table, "isPrimary", &DisplayBinding::isPrimary);
		S.set_map_value(method_table, "getDisplayScale", &DisplayBinding::getDisplayScale);
		S.set_map_value(method_table, "getAll", &DisplayBinding::getAll);
		S.set_map_value(method_table, "getPrimary", &DisplayBinding::getPrimary);
		S.set_map_value(method_table, "getNearestFromWindow", &DisplayBinding::getNearestFromWindow);

		// metatable

		auto const metatable = S.create_metatable(class_name);
		S.set_map_value(metatable, "__gc", &DisplayBinding::__gc);
		S.set_map_value(metatable, "__tostring", &DisplayBinding::__tostring);
		S.set_map_value(metatable, "__eq", &DisplayBinding::__eq);
		S.set_map_value(metatable, "__index", method_table);

		// register

		auto const module_table = S.push_module("lstg");
		S.set_map_value(module_table, "Display", method_table);
	}

}
