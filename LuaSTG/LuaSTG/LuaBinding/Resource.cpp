#include "lua_utility.hpp"
#include "AppFrame.h"

namespace LuaSTG::Sub::LuaBinding
{
#define check_self_data() if (self->data == nullptr) return luaL_error(L, "null pointer exception");
#define get_self() auto* self = cast(L, 1);

	struct Texture2D
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.Texture2D" };

		LuaSTGPlus::IResourceTexture* data;

		// LuaSTG.Sub.Resource
		static int api_getResourceType(lua_State* L)
		{
			get_self();
			lua::stack_t S(L);
			S.push_value(static_cast<int32_t>(self->data->GetType()));
			return 1;
		}
		static int api_getResourceName(lua_State* L)
		{
			get_self();
			lua::stack_t S(L);
			S.push_value(self->data->GetResName());
			return 1;
		}
		// LuaSTG.Sub.Texture2D
		static int api_getSize(lua_State* L)
		{
			get_self();
			lua::stack_t S(L);
			auto const result = self->data->GetTexture()->getSize();
			S.push_vector2(result);
			return 1;
		}

		static int api___gc(lua_State* L)
		{
			auto* self = cast(L, 1);
			if (self->data)
			{
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}
		static int api___tostring(lua_State* L)
		{
			lua::stack_t S(L);
			std::ignore = cast(L, 1);
			S.push_value<std::string_view>(ClassID);
			return 1;
		}

		static int api_createFromFile(lua_State* L)
		{
			lua::stack_t S(L);

			// ignore 1
			auto const path = S.get_value<std::string_view>(2);
			auto const mipmap = S.get_value<bool>(3);

			// create texture
			auto const loading_name = "{LuaSTG.Sub.Texture2D}/loading";
			auto pool = LAPP.GetResourceMgr().GetActivedPool();
			if (!pool->LoadTexture(loading_name, path.data(), mipmap))
			{
				return luaL_error(L, "load texture '%s' failed", path.data());
			}
			auto tex = pool->GetTexture(loading_name);
			pool->RemoveResource(LuaSTGPlus::ResourceType::Texture, loading_name);

			// create userdata
			auto* udata = create(L);
			udata->data = tex.get();
			if (udata->data)
			{
				udata->data->retain();
			}
			
			return 1;
		}

		static Texture2D* create(lua_State* L)
		{
			lua::stack_t S(L);

			auto* self = S.create_userdata<Texture2D>();
			auto const self_index = S.index_of_top();
			S.set_metatable(self_index, ClassID);

			self->data = nullptr;
			return self;
		}
		static Texture2D* cast(lua_State* L, int idx)
		{
			return static_cast<Texture2D*>(luaL_checkudata(L, idx, ClassID.data()));
		}
		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// method

			auto const method_table = S.create_map();
			S.set_map_value(method_table, "getResourceType", &api_getResourceType);
			S.set_map_value(method_table, "getResourceName", &api_getResourceName);
			S.set_map_value(method_table, "getSize", &api_getSize);

			// metatable

			auto const metatable = S.create_metatable(ClassID);
			S.set_map_value(metatable, "__gc", &api___gc);
			S.set_map_value(metatable, "__tostring", &api___tostring);
			S.set_map_value(metatable, "__index", method_table);

			// factory

			auto const class_table = S.create_map();
			S.set_map_value(class_table, "createFromFile", &api_createFromFile);
			
			// register

			auto const M = S.push_module("LuaSTG.Sub");
			S.set_map_value(M, "Texture2D", class_table);
		}
	};

	struct Sprite2D
	{
		void test()
		{
			std::string s(Texture2D::ClassID);
			std::printf(s.c_str());
		}
	};
}

int luaopen_LuaSTG_Sub(lua_State* L)
{
	lua::stack_t S(L);
	LuaSTG::Sub::LuaBinding::Texture2D::registerClass(L);
	S.push_module("LuaSTG.Sub");
	lua_pushnil(L);
	lua_setglobal(L, "LuaSTG.Sub");
	return 1;
}
