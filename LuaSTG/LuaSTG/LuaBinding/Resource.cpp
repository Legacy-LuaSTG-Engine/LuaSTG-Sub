#include "lua_utility.hpp"
#include "AppFrame.h"

namespace LuaSTG::Sub::LuaBinding
{
#define check_self_data() if (self->data == nullptr) return luaL_error(L, "null pointer exception");
#define get_self() auto* self = cast(L, 1);
#define API(x) static int api_##x##(lua_State* L)

	static constexpr std::string_view const ModuleID{ "LuaSTG.Sub" };

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

	struct ResourceTexture
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceTexture" };

		LuaSTGPlus::IResourceTexture* data;

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

		static ResourceTexture* create(lua_State* L)
		{
			lua::stack_t S(L);

			auto* self = S.create_userdata<ResourceTexture>();
			auto const self_index = S.index_of_top();
			S.set_metatable(self_index, ClassID);

			self->data = nullptr;
			return self;
		}
		static ResourceTexture* cast(lua_State* L, int idx)
		{
			return static_cast<ResourceTexture*>(luaL_checkudata(L, idx, ClassID.data()));
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

			// 暂时不暴露出创建接口
			//auto const class_table = S.create_map();
			//S.set_map_value(class_table, "createFromFile", &api_createFromFile);

			// register

			// 暂时不暴露出创建接口
			//auto const M = S.push_module("LuaSTG.Sub");
			//S.set_map_value(M, "Texture2D", class_table);
		}
	};

	struct ResourceSet
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceSet" };

		LuaSTGPlus::ResourcePool* data;

		static int api_createTextureFromFile(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const name = S.get_value<std::string_view>(2);
			auto const path = S.get_value<std::string_view>(3);
			auto const mipmap = S.get_value<bool>(4);
			if (!self->data->LoadTexture(name.data(), path.data(), mipmap)) {
				return luaL_error(L, "can't create texture '%s' from file '%s'.", name.data(), path.data());
			}
			auto res = self->data->GetTexture(name);
			auto* tex = ResourceTexture::create(L);
			tex->data = res.detach(); // 转移所有权
			return 1;
		}

		static int api___gc(lua_State* L)
		{
			// 目前 ResourcePool 都是静态对象，不需要释放
			std::ignore = cast(L, 1);
			return 0;
		}
		static int api___tostring(lua_State* L)
		{
			lua::stack_t S(L);
			std::ignore = cast(L, 1);
			S.push_value<std::string_view>(ClassID);
			return 1;
		}

		static ResourceSet* create(lua_State* L)
		{
			lua::stack_t S(L);

			auto* self = S.create_userdata<ResourceSet>();
			auto const self_index = S.index_of_top();
			S.set_metatable(self_index, ClassID);

			self->data = nullptr;
			return self;
		}
		static ResourceSet* cast(lua_State* L, int idx)
		{
			return static_cast<ResourceSet*>(luaL_checkudata(L, idx, ClassID.data()));
		}
		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// method

			auto const method_table = S.create_map();
			S.set_map_value(method_table, "createTextureFromFile", &api_createTextureFromFile);

			// metatable

			auto const metatable = S.create_metatable(ClassID);
			S.set_map_value(metatable, "__gc", &api___gc);
			S.set_map_value(metatable, "__tostring", &api___tostring);
			S.set_map_value(metatable, "__index", method_table);

			// register

			// 暂时不暴露出创建接口
			//auto const M = S.push_module("lstg");
			//S.set_map_value(M, "ResourceSet", class_table);
		}
	};

	struct ResourceManager
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceManager" };

		static int api_getResourceSet(lua_State* L)
		{
			lua::stack_t S(L);
			auto const name = S.get_value<std::string_view>(1);
			auto* set = ResourceSet::create(L);
			if (name == "global") {
				set->data = LRES.GetResourcePool(LuaSTGPlus::ResourcePoolType::Global);
			}
			else if (name == "stage") {
				set->data = LRES.GetResourcePool(LuaSTGPlus::ResourcePoolType::Stage);
			}
			else {
				return luaL_error(L, "resource set '%s' not found", name.data());
			}
			return 1;
		}

		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// class

			auto const class_table = S.create_map();
			S.set_map_value(class_table, "getResourceSet", &api_getResourceSet);

			// register

			auto const M = S.push_module("lstg");
			S.set_map_value(M, "ResourceManager", class_table);
		}
	};
}

int luaopen_LuaSTG_Sub(lua_State* L)
{
	lua::stack_t S(L);
	LuaSTG::Sub::LuaBinding::ResourceTexture::registerClass(L);
	LuaSTG::Sub::LuaBinding::ResourceSet::registerClass(L);
	LuaSTG::Sub::LuaBinding::ResourceManager::registerClass(L);
	//S.push_module("LuaSTG.Sub");
	//lua_pushnil(L);
	//lua_setglobal(L, "LuaSTG.Sub");
	return 1;
}
