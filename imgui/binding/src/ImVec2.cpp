#include "lua.hpp"
#include "imgui.h"
#include "lua_imgui_type.hpp"

namespace lua {
	inline constexpr char type_name[]{ "imgui.ImVec2" };

	enum class property {
		not_found,
		x,
		y,
	};

	static inline property map_property(lua_State* L, int index) {
		size_t len{};
		const char* key = luaL_checklstring(L, index, &len);
		switch (len) {
		case 1:
			switch (key[0]) {
			case 'x': return property::x;
			case 'y': return property::y;
			default: return property::not_found;
			}
		default: return property::not_found;
		}
	}

	struct binding {
		static inline ImVec2* as(lua_State* L, int index) {
			return static_cast<ImVec2*>(lua_touserdata(L, index));
		}

		static int __eq(lua_State* L)
		{
			if (is_type_instance<ImVec2>(L, 1) && is_type_instance<ImVec2>(L, 2)) {
				ImVec2* left = as(L, 1);
				ImVec2* right = as(L, 2);
				lua_pushboolean(L, left->x == right->x && left->y == right->y);
			}
			else {
				lua_pushboolean(L, false);
			}
			return 1;
		};

		static int __index(lua_State* L)
		{
			ImVec2* self = as(L, 1);
			switch (map_property(L, 2))
			{
			case property::x:
				lua_pushnumber(L, self->x);
				return 1;
			case property::y:
				lua_pushnumber(L, self->y);
				return 1;
			default:
				return luaL_error(L, "property '%s' not exists.", lua_tostring(L, 2));
			}
		};
		static int __newindex(lua_State* L)
		{
			ImVec2* self = as(L, 1);
			switch (map_property(L, 2))
			{
			case property::x:
				self->x = (float)luaL_checknumber(L, 3);
				return 0;
			case property::y:
				self->y = (float)luaL_checknumber(L, 3);
				return 0;
			default:
				return luaL_error(L, "property '%s' not exists.", lua_tostring(L, 2));
			}
		};
		static int __tostring(lua_State* L)
		{
			as(L, 1);
			lua_pushlstring(L, type_name, sizeof(type_name) - 1);
			return 1;
		};

		static int create(lua_State* L)
		{
			const int argc = lua_gettop(L);
			ImVec2* self = create_type_instance<ImVec2>(L);
			switch (argc)
			{
			default:
			case 0:
				self->x = 0.0f;
				self->y = 0.0f;
				return 1;
			case 1:
			case 2:
				self->x = (float)luaL_checknumber(L, 1);
				self->y = (float)luaL_checknumber(L, 2);
				return 1;
			}
		};
	};
	
	template<>
	void register_type<ImVec2>(lua_State* L) {
		const luaL_Reg metatable[]{
			{"__eq", &binding::__eq},

			{"__index", &binding::__index},
			{"__newindex", &binding::__newindex},
			{"__tostring", &binding::__tostring},
			{NULL, NULL},
		};
		luaL_newmetatable(L, type_name);
		luaL_register(L, NULL, metatable);
		lua_pop(L, 1);
		const luaL_Reg constructors[]{
			{"ImVec2", &binding::create},

			{NULL, NULL},
		};
		luaL_register(L, NULL, constructors);
	}

	template<>
	ImVec2* create_type_instance<ImVec2>(lua_State* L) {
		ImVec2* self = static_cast<ImVec2*>(lua_newuserdata(L, sizeof(ImVec2)));
		luaL_getmetatable(L, type_name);
		lua_setmetatable(L, -2);
		return self;
	}

	template<>
	ImVec2* create_type_instance<ImVec2>(lua_State* L, const ImVec2& init) {
		ImVec2* self = static_cast<ImVec2*>(lua_newuserdata(L, sizeof(ImVec2)));
		*self = init;
		luaL_getmetatable(L, type_name);
		lua_setmetatable(L, -2);
		return self;
	}

	template<>
	ImVec2* as_type_instance<ImVec2>(lua_State* L, int index) {
		return static_cast<ImVec2*>(luaL_checkudata(L, index, type_name));
	}

	template<>
	bool is_type_instance<ImVec2>(lua_State* L, int index) {
		return luaL_testudata(L, index, type_name) != NULL;
	}
}
