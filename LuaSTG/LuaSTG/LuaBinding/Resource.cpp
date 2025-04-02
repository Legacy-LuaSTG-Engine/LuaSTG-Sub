#include "lua/plus.hpp"
#include "AppFrame.h"

namespace luastg::binding
{
	static constexpr std::string_view const ModuleID{ "LuaSTG.Sub" };

	struct ResourceTexture
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceTexture" };

		luastg::IResourceTexture* data;

		static int api_getResourceType(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			S.push_value(static_cast<int32_t>(self->data->GetType()));
			return 1;
		}
		static int api_getResourceName(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			S.push_value(self->data->GetResName());
			return 1;
		}
		static int api_getSize(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const result = self->data->GetTexture()->getSize();
			S.push_value(result.x);
			S.push_value(result.y);
			return 2;
		}
		static int api_getWidth(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const result = self->data->GetTexture()->getSize();
			S.push_value(result.x);
			return 1;
		}
		static int api_getHeight(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const result = self->data->GetTexture()->getSize();
			S.push_value(result.y);
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
		static int api___eq(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			if (test(L, 2)) {
				auto* other = cast(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
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
		static bool test(lua_State* L, int idx)
		{
			return nullptr != luaL_testudata(L, idx, ClassID.data());
		}
		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// method

			auto const method_table = S.create_map();
			S.set_map_value(method_table, "getResourceType", &api_getResourceType);
			S.set_map_value(method_table, "getResourceName", &api_getResourceName);
			//S.set_map_value(method_table, "getSize", &api_getSize);
			S.set_map_value(method_table, "getWidth", &api_getWidth);
			S.set_map_value(method_table, "getHeight", &api_getHeight);

			// metatable

			auto const metatable = S.create_metatable(ClassID);
			S.set_map_value(metatable, "__gc", &api___gc);
			S.set_map_value(metatable, "__tostring", &api___tostring);
			S.set_map_value(metatable, "__eq", &api___eq);
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

	struct ResourceSprite
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceSprite" };

		luastg::IResourceSprite* data;

		// IResource
		static int api_getResourceType(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			S.push_value(static_cast<int32_t>(self->data->GetType()));
			return 1;
		}
		static int api_getResourceName(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			S.push_value(self->data->GetResName());
			return 1;
		}
		// IResourceSprite
		static int api_setCenter(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const x = S.get_value<float>(2);
			auto const y = S.get_value<float>(3);
			self->data->GetSprite()->setTextureCenter(Core::Vector2F(x, y));
			return 0;
		}
		static int api_setUnitsPerPixel(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const v = S.get_value<float>(2);
			self->data->GetSprite()->setUnitsPerPixel(v);
			return 0;
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
		static int api___eq(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			if (test(L, 2)) {
				auto* other = cast(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		static ResourceSprite* create(lua_State* L)
		{
			lua::stack_t S(L);

			auto* self = S.create_userdata<ResourceSprite>();
			auto const self_index = S.index_of_top();
			S.set_metatable(self_index, ClassID);

			self->data = nullptr;
			return self;
		}
		static ResourceSprite* cast(lua_State* L, int idx)
		{
			return static_cast<ResourceSprite*>(luaL_checkudata(L, idx, ClassID.data()));
		}
		static bool test(lua_State* L, int idx)
		{
			return nullptr != luaL_testudata(L, idx, ClassID.data());
		}
		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// method

			auto const method_table = S.create_map();
			S.set_map_value(method_table, "getResourceType", &api_getResourceType);
			S.set_map_value(method_table, "getResourceName", &api_getResourceName);
			S.set_map_value(method_table, "setCenter", &api_setCenter);
			S.set_map_value(method_table, "setUnitsPerPixel", &api_setUnitsPerPixel);

			// metatable

			auto const metatable = S.create_metatable(ClassID);
			S.set_map_value(metatable, "__gc", &api___gc);
			S.set_map_value(metatable, "__tostring", &api___tostring);
			S.set_map_value(metatable, "__eq", &api___eq);
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
	
	struct ResourceSpriteSequence
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceSpriteSequence" };

		luastg::IResourceAnimation* data;

		static int api_getResourceType(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			S.push_value(static_cast<int32_t>(self->data->GetType()));
			return 1;
		}
		static int api_getResourceName(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			S.push_value(self->data->GetResName());
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
		static int api___eq(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			if (test(L, 2)) {
				auto* other = cast(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		static ResourceSpriteSequence* create(lua_State* L)
		{
			lua::stack_t S(L);

			auto* self = S.create_userdata<ResourceSpriteSequence>();
			auto const self_index = S.index_of_top();
			S.set_metatable(self_index, ClassID);

			self->data = nullptr;
			return self;
		}
		static ResourceSpriteSequence* cast(lua_State* L, int idx)
		{
			return static_cast<ResourceSpriteSequence*>(luaL_checkudata(L, idx, ClassID.data()));
		}
		static bool test(lua_State* L, int idx)
		{
			return nullptr != luaL_testudata(L, idx, ClassID.data());
		}
		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// method

			auto const method_table = S.create_map();
			S.set_map_value(method_table, "getResourceType", &api_getResourceType);
			S.set_map_value(method_table, "getResourceName", &api_getResourceName);

			// metatable

			auto const metatable = S.create_metatable(ClassID);
			S.set_map_value(metatable, "__gc", &api___gc);
			S.set_map_value(metatable, "__tostring", &api___tostring);
			S.set_map_value(metatable, "__eq", &api___eq);
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

	struct ResourceCollection
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceCollection" };

		luastg::ResourcePool* data;

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
		static int api_createSprite(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const sprite_name = S.get_value<std::string_view>(2);
			Core::ScopeObject<luastg::IResourceTexture> texture;
			if (S.is_string(3)) {
				auto const texture_name = S.get_value<std::string_view>(3);
				texture = self->data->GetTexture(texture_name);
				if (!texture) {
					return luaL_error(L, "can't find texture '%s'.", texture_name.data());
				}
			}
			else {
				auto* p_tex = ResourceTexture::cast(L, 3);
				texture = p_tex->data;
			}
			auto const x = S.get_value<float>(4, 0.0f);
			auto const y = S.get_value<float>(5, 0.0f);
			auto const texture_size = texture->GetTexture()->getSize();
			auto const width = S.get_value<float>(6, float(texture_size.x));
			auto const height = S.get_value<float>(7, float(texture_size.y));
			auto const a = S.get_value<float>(8, 0.0f);
			auto const b = S.get_value<float>(9, 0.0f);
			auto const rect = S.get_value<bool>(10, false);
			if (!self->data->CreateSprite(sprite_name.data(), texture->GetResName().data(), x, y, width, height, a, b, rect))
			{
				return luaL_error(L, "load image failed (name='%s', tex='%s').", sprite_name.data(), texture->GetResName().data());
			}
			auto res = self->data->GetSprite(sprite_name);
			auto* sprite = ResourceSprite::create(L);
			sprite->data = res.detach(); // 转移所有权
			return 1;
		}
		static int api_createSpriteSequence(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);

			auto const sprite_sequence_name = S.get_value<std::string_view>(2);
			if (S.is_string(3) || S.is_userdata(3)) {
				Core::ScopeObject<luastg::IResourceTexture> texture;
				if (S.is_string(3)) {
					auto const texture_name = S.get_value<std::string_view>(3);
					texture = self->data->GetTexture(texture_name);
					if (!texture) {
						return luaL_error(L, "can't find texture '%s'.", texture_name.data());
					}
				}
				else {
					auto* p_tex = ResourceTexture::cast(L, 3);
					texture = p_tex->data;
				}
				auto const x = S.get_value<float>(4);
				auto const y = S.get_value<float>(5);
				auto const width = S.get_value<float>(6);
				auto const height = S.get_value<float>(7);
				auto const columns = S.get_value<int32_t>(8);
				auto const rows = S.get_value<int32_t>(9);
				auto const interval = S.get_value<int32_t>(10);
				auto const a = S.get_value<float>(11, 0.0f);
				auto const b = S.get_value<float>(12, 0.0f);
				auto const rect = S.get_value<bool>(13, false);
				if (!self->data->CreateAnimation(
					sprite_sequence_name.data(), texture->GetResName().data(),
					x, y, width, height,
					columns, rows,
					interval,
					a, b, rect))
				{
					return luaL_error(L, "load animation failed (name='%s', tex='%s').", sprite_sequence_name.data(), texture->GetResName());
				}
			}
			else /* (S.is_table(3)) */ {
				size_t const sprite_count = S.get_array_size(3);
				std::vector<Core::ScopeObject<luastg::IResourceSprite>> sprite_list;
				for (size_t index = 0; index < sprite_count; index += 1)
				{
					S.push_array_value_zero_base(3, index);
					auto* p_sprite = ResourceSprite::cast(L, -1);
					S.pop_value();
					sprite_list.push_back(p_sprite->data);
				}
				auto const interval = S.get_value<int32_t>(4);
				auto const a = S.get_value<float>(5, 0.0f);
				auto const b = S.get_value<float>(6, 0.0f);
				auto const rect = S.get_value<bool>(7, false);
				if (!self->data->CreateAnimation(sprite_sequence_name.data(), sprite_list, interval, a, b, rect))
				{
					return luaL_error(L, "load animation failed (name='%s').", sprite_sequence_name.data());
				}
			}
			auto res = self->data->GetAnimation(sprite_sequence_name);
			auto* sprite_sequence = ResourceSpriteSequence::create(L);
			sprite_sequence->data = res.detach(); // 转移所有权
			return 1;
		}
		static int api_removeTexture(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			Core::ScopeObject<luastg::IResourceTexture> texture;
			if (S.is_string(2)) {
				auto const texture_name = S.get_value<std::string_view>(2);
				texture = self->data->GetTexture(texture_name);
			}
			else {
				auto* p_texture = ResourceTexture::cast(L, 2);
				texture = p_texture->data;
			}
			if (texture) {
				self->data->RemoveResource(luastg::ResourceType::Texture, texture->GetResName().data());
			}
			return 0;
		}
		static int api_removeSprite(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			Core::ScopeObject<luastg::IResourceSprite> sprite;
			if (S.is_string(2)) {
				auto const sprite_name = S.get_value<std::string_view>(2);
				sprite = self->data->GetSprite(sprite_name);
			}
			else {
				auto* p_sprite = ResourceSprite::cast(L, 2);
				sprite = p_sprite->data;
			}
			if (sprite) {
				self->data->RemoveResource(luastg::ResourceType::Sprite, sprite->GetResName().data());
			}
			return 0;
		}
		static int api_removeSpriteSequence(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			Core::ScopeObject<luastg::IResourceAnimation> sprite_sequence;
			if (S.is_string(2)) {
				auto const sprite_sequence_name = S.get_value<std::string_view>(2);
				sprite_sequence = self->data->GetAnimation(sprite_sequence_name);
			}
			else {
				auto* p_sprite_seq = ResourceSpriteSequence::cast(L, 2);
				sprite_sequence = p_sprite_seq->data;
			}
			if (sprite_sequence) {
				self->data->RemoveResource(luastg::ResourceType::Animation, sprite_sequence->GetResName().data());
			}
			return 0;
		}
		static int api_getTexture(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const texture_name = S.get_value<std::string_view>(2);
			auto res = self->data->GetTexture(texture_name);
			if (!res) {
				return luaL_error(L, "can't find texture '%s'.", texture_name.data());
			}
			auto* tex = ResourceTexture::create(L);
			tex->data = res.detach(); // 转移所有权
			return 1;
		}
		static int api_getSprite(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const sprite_name = S.get_value<std::string_view>(2);
			auto res = self->data->GetSprite(sprite_name);
			if (!res) {
				return luaL_error(L, "can't find sprite '%s'.", sprite_name.data());
			}
			auto* sprite = ResourceSprite::create(L);
			sprite->data = res.detach(); // 转移所有权
			return 1;
		}
		static int api_getSpriteSequence(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const sprite_sequence_name = S.get_value<std::string_view>(2);
			auto res = self->data->GetAnimation(sprite_sequence_name);
			if (!res) {
				return luaL_error(L, "can't find animation '%s'.", sprite_sequence_name.data());
			}
			auto* sprite_sequence = ResourceSpriteSequence::create(L);
			sprite_sequence->data = res.detach(); // 转移所有权
			return 1;
		}
		static int api_isTextureExist(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const texture_name = S.get_value<std::string_view>(2);
			auto res = self->data->GetTexture(texture_name);
			S.push_value<bool>(res);
			return 1;
		}
		static int api_isSpriteExist(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const sprite_name = S.get_value<std::string_view>(2);
			auto res = self->data->GetSprite(sprite_name);
			S.push_value<bool>(res);
			return 1;
		}
		static int api_isSpriteSequenceExist(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			auto const sprite_sequence_name = S.get_value<std::string_view>(2);
			auto res = self->data->GetAnimation(sprite_sequence_name);
			S.push_value<bool>(res);
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
		static int api___eq(lua_State* L)
		{
			lua::stack_t S(L);
			auto* self = cast(L, 1);
			if (test(L, 2)) {
				auto* other = cast(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		static ResourceCollection* create(lua_State* L)
		{
			lua::stack_t S(L);

			auto* self = S.create_userdata<ResourceCollection>();
			auto const self_index = S.index_of_top();
			S.set_metatable(self_index, ClassID);

			self->data = nullptr;
			return self;
		}
		static ResourceCollection* cast(lua_State* L, int idx)
		{
			return static_cast<ResourceCollection*>(luaL_checkudata(L, idx, ClassID.data()));
		}
		static bool test(lua_State* L, int idx)
		{
			return nullptr != luaL_testudata(L, idx, ClassID.data());
		}
		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// method

			auto const method_table = S.create_map();
			S.set_map_value(method_table, "createTextureFromFile", &api_createTextureFromFile);
			S.set_map_value(method_table, "createSprite", &api_createSprite);
			S.set_map_value(method_table, "createSpriteSequence", &api_createSpriteSequence);
			S.set_map_value(method_table, "removeTexture", &api_removeTexture);
			S.set_map_value(method_table, "removeSprite", &api_removeSprite);
			S.set_map_value(method_table, "removeSpriteSequence", &api_removeSpriteSequence);
			S.set_map_value(method_table, "getTexture", &api_getTexture);
			S.set_map_value(method_table, "getSprite", &api_getSprite);
			S.set_map_value(method_table, "getSpriteSequence", &api_getSpriteSequence);
			S.set_map_value(method_table, "isTextureExist", &api_isTextureExist);
			S.set_map_value(method_table, "isSpriteExist", &api_isSpriteExist);
			S.set_map_value(method_table, "isSpriteSequenceExist", &api_isSpriteSequenceExist);

			// metatable

			auto const metatable = S.create_metatable(ClassID);
			S.set_map_value(metatable, "__gc", &api___gc);
			S.set_map_value(metatable, "__tostring", &api___tostring);
			S.set_map_value(metatable, "__eq", &api___eq);
			S.set_map_value(metatable, "__index", method_table);

			// register

			// 暂时不暴露出创建接口
			//auto const M = S.push_module("lstg");
			//S.set_map_value(M, "ResourceCollection", class_table);
		}
	};

	struct ResourceManager
	{
		static constexpr std::string_view const ClassID{ "LuaSTG.Sub.ResourceManager" };

		static int api_getResourceCollection(lua_State* L)
		{
			lua::stack_t S(L);
			auto const name = S.get_value<std::string_view>(1);
			auto* set = ResourceCollection::create(L);
			if (name == "global") {
				set->data = LRES.GetResourcePool(luastg::ResourcePoolType::Global);
			}
			else if (name == "stage") {
				set->data = LRES.GetResourcePool(luastg::ResourcePoolType::Stage);
			}
			else {
				return luaL_error(L, "resource set '%s' not found", name.data());
			}
			return 1;
		}
		static int api_setCurrentResourceCollection(lua_State* L)
		{
			lua::stack_t S(L);
			auto const name = S.get_value<std::string_view>(1);
			if (name == "global") {
				LRES.SetActivedPoolType(luastg::ResourcePoolType::Global);
			}
			else if (name == "stage") {
				LRES.SetActivedPoolType(luastg::ResourcePoolType::Stage);
			}
			else {
				return luaL_error(L, "resource set '%s' not found", name.data());
			}
			return 0;
		}
		static int api_getCurrentResourceCollection(lua_State* L)
		{
			lua::stack_t S(L);
			auto const type = LRES.GetActivedPoolType();
			if (luastg::ResourcePoolType::Global == type) {
				S.push_value<std::string_view>("global");
			}
			else if (luastg::ResourcePoolType::Stage == type) {
				S.push_value<std::string_view>("stage");
			}
			else if (luastg::ResourcePoolType::None == type) {
				S.push_value<std::string_view>("none");
			}
			return 1;
		}

		static void registerClass(lua_State* L)
		{
			[[maybe_unused]] lua::stack_balancer_t SB(L);
			lua::stack_t S(L);

			// class

			auto const class_table = S.create_map();
			S.set_map_value(class_table, "getResourceCollection", &api_getResourceCollection);
			S.set_map_value(class_table, "setCurrentResourceCollection", &api_setCurrentResourceCollection);
			S.set_map_value(class_table, "getCurrentResourceCollection", &api_getCurrentResourceCollection);

			// register

			auto const M = S.push_module("lstg");
			S.set_map_value(M, "ResourceManager", class_table);
		}
	};
}

int luaopen_LuaSTG_Sub(lua_State* L)
{
	luastg::binding::ResourceTexture::registerClass(L);
	luastg::binding::ResourceSprite::registerClass(L);
	luastg::binding::ResourceSpriteSequence::registerClass(L);
	luastg::binding::ResourceCollection::registerClass(L);
	luastg::binding::ResourceManager::registerClass(L);
	return 1;
}
