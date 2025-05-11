#include "LuaBinding/modern/GameObject.hpp"
#include "LuaBinding/generated/GameObjectMember.hpp"
#include "GameObject/GameObjectPool.h"
#include "AppFrame.h"
#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/LuaWrapperMisc.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	std::byte game_object_meta_table_key{};
	std::byte game_object_tables_key{};

	std::string_view getStatusName(luastg::GameObjectStatus const status) {
		switch (status) {
		case luastg::GameObjectStatus::Active:
			return "normal"sv;
		case luastg::GameObjectStatus::Dead:
			return "del"sv;
		case luastg::GameObjectStatus::Killed:
			return "kill"sv;
		case luastg::GameObjectStatus::Free:
		default:
			return "free"sv;
		}
	}
	luastg::GameObjectStatus getStatusFromName(std::string_view const& name) {
		if (name == "normal"sv)
			return luastg::GameObjectStatus::Active;
		if (name == "del"sv)
			return luastg::GameObjectStatus::Dead;
		if (name == "kill"sv)
			return luastg::GameObjectStatus::Killed;
		return luastg::GameObjectStatus::Free;
	}
}

namespace luastg::binding {
	std::string_view const GameObject::class_name{ "lstg.GameObject"sv };

	struct GameObjectBinding : GameObject {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __index(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const key = ctx.get_value<std::string_view>(2);
			switch (LuaSTG::MapGameObjectMember(key.data(), key.size())) {
				// 基本信息

			case LuaSTG::GameObjectMember::STATUS:
				if (self->status == GameObjectStatus::Free) {
					return luaL_error(vm, "unknown lstg object status.");
				}
				ctx.push_value(getStatusName(self->status));
				return 1;
			case LuaSTG::GameObjectMember::CLASS:
				lua_rawgeti(vm, 1, 1); // self[1]
				return 1;

				// 分组

			#ifdef USING_MULTI_GAME_WORLD
			case LuaSTG::GameObjectMember::WORLD:
				ctx.push_value(vm, self->world);
				return 1;
			#endif // USING_MULTI_GAME_WORLD

				// 位置

			case LuaSTG::GameObjectMember::X:
				ctx.push_value(self->x);
				return 1;
			case LuaSTG::GameObjectMember::Y:
				ctx.push_value(self->y);
				return 1;
			case LuaSTG::GameObjectMember::DX:
				ctx.push_value(self->dx);
				return 1;
			case LuaSTG::GameObjectMember::DY:
				ctx.push_value(self->dy);
				return 1;

				// 运动学

			case LuaSTG::GameObjectMember::VX:
				ctx.push_value(self->vx);
				return 1;
			case LuaSTG::GameObjectMember::VY:
				ctx.push_value(self->vy);
				return 1;
			case LuaSTG::GameObjectMember::AX:
				ctx.push_value(self->ax);
				return 1;
			case LuaSTG::GameObjectMember::AY:
				ctx.push_value(self->ay);
				return 1;
			#ifdef USER_SYSTEM_OPERATION
			case LuaSTG::GameObjectMember::MAXVX:
				ctx.push_value(self->max_vx);
				return 1;
			case LuaSTG::GameObjectMember::MAXVY:
				ctx.push_value(self->max_vy);
				return 1;
			case LuaSTG::GameObjectMember::MAXV:
				ctx.push_value(self->max_v);
				return 1;
			case LuaSTG::GameObjectMember::AG:
				ctx.push_value(self->ag);
				return 1;
			#endif
			case LuaSTG::GameObjectMember::VSPEED:
				ctx.push_value(self->calculateSpeed());
				return 1;
			case LuaSTG::GameObjectMember::VANGLE:
				ctx.push_value(self->calculateSpeedDirection() * L_RAD_TO_DEG);
				return 1;

				// 碰撞体

			case LuaSTG::GameObjectMember::GROUP:
				lua_pushinteger(vm, self->group); // interesting
				return 1;
			case LuaSTG::GameObjectMember::BOUND:
				ctx.push_value<bool>(self->bound);
				return 1;
			case LuaSTG::GameObjectMember::COLLI:
				ctx.push_value<bool>(self->colli);
				return 1;
			case LuaSTG::GameObjectMember::RECT:
				ctx.push_value<bool>(self->rect);
				return 1;
			case LuaSTG::GameObjectMember::A:
			#ifdef GLOBAL_SCALE_COLLI_SHAPE
				ctx.push_value(self->a / LRES.GetGlobalImageScaleFactor());
			#else
				ctx.push_value(self->a);
			#endif // GLOBAL_SCALE_COLLI_SHAPE
				return 1;
			case LuaSTG::GameObjectMember::B:
			#ifdef GLOBAL_SCALE_COLLI_SHAPE
				ctx.push_value(self->b / LRES.GetGlobalImageScaleFactor());
			#else
				ctx.push_value(self->b);
			#endif // GLOBAL_SCALE_COLLI_SHAPE
				return 1;

				// 渲染

			case LuaSTG::GameObjectMember::LAYER:
				ctx.push_value(self->layer);
				return 1;
			case LuaSTG::GameObjectMember::HSCALE:
				ctx.push_value(self->hscale);
				return 1;
			case LuaSTG::GameObjectMember::VSCALE:
				ctx.push_value(self->vscale);
				return 1;
			case LuaSTG::GameObjectMember::ROT:
				ctx.push_value(self->rot * L_RAD_TO_DEG);
				return 1;
			case LuaSTG::GameObjectMember::OMEGA:
			case LuaSTG::GameObjectMember::OMIGA:
				ctx.push_value(self->omega * L_RAD_TO_DEG);
				return 1;
			case LuaSTG::GameObjectMember::_BLEND:
				if (self->features.is_render_class) {
					TranslateBlendModeToString(vm, self->blend_mode);
					return 1;
				}
				break;
			case LuaSTG::GameObjectMember::_COLOR:
				if (self->features.is_render_class) {
					Color::CreateAndPush(vm, self->vertex_color);
					return 1;
				}
				break;
			case LuaSTG::GameObjectMember::_A:
				if (self->features.is_render_class) {
					ctx.push_value(self->vertex_color.a);
					return 1;
				}
				break;
			case LuaSTG::GameObjectMember::_R:
				if (self->features.is_render_class) {
					ctx.push_value(self->vertex_color.r);
					return 1;
				}
				break;
			case LuaSTG::GameObjectMember::_G:
				if (self->features.is_render_class) {
					ctx.push_value(self->vertex_color.g);
					return 1;
				}
				break;
			case LuaSTG::GameObjectMember::_B:
				if (self->features.is_render_class) {
					ctx.push_value(self->vertex_color.b);
					return 1;
				}
				break;
			case LuaSTG::GameObjectMember::ANI:
				lua_pushinteger(vm, self->ani_timer); // interesting
				return 1;
			case LuaSTG::GameObjectMember::HIDE:
				ctx.push_value<bool>(self->hide);
				return 1;
			case LuaSTG::GameObjectMember::NAVI:
				ctx.push_value<bool>(self->navi);
				return 1;
			case LuaSTG::GameObjectMember::IMG:
				if (self->hasRenderResource()) {
					ctx.push_value(self->getRenderResourceName());
					return 1;
				}
				break;
			#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
			case LuaSTG::GameObjectMember::RES_RC:
				if (self->features.is_render_class) {
					lua_rawgeti(vm, 1, 4); // self[4]
					return 1;
				}
				break;
			#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT

				// 更新控制

			case LuaSTG::GameObjectMember::TIMER:
				lua_pushinteger(vm, self->timer); // interesting
				return 1;
			#ifdef	LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
			case LuaSTG::GameObjectMember::PAUSE:
				lua_pushinteger(vm, self->pause); // interesting
				return 1;
			case LuaSTG::GameObjectMember::RESOLVEMOVE:
				ctx.push_value<bool>(self->resolve_move);
				return 1;
			#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
			case LuaSTG::GameObjectMember::IGNORESUPERPAUSE:
				ctx.push_value<bool>(self->ignore_super_pause);
				return 1;

			default:
				break;
			}

			lua_rawget(vm, 1); // self[key]
			return 1;
		}
		static int __newindex(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (self == nullptr)
				return luaL_error(vm, "nullptr");
			auto const key = ctx.get_value<std::string_view>(2);
			switch (LuaSTG::MapGameObjectMember(key.data(), key.size())) {
				// 基本信息

			case LuaSTG::GameObjectMember::STATUS: {
				auto const value = ctx.get_value<std::string_view>(3);
				auto const status = getStatusFromName(value);
				if (status == GameObjectStatus::Free) {
					return luaL_error(vm, "invalid argument for property 'status', must be 'normal', 'del' or 'kill'");
				}
				self->status = status;
				return 0;
			}
			case LuaSTG::GameObjectMember::CLASS:
				self->features.read(vm, 3); // 刷新对象的 class
				if (!self->features.is_class)
					return luaL_error(vm, "invalid argument for property 'class', required luastg object class.");
			#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
				if (!self.features.is_render_class)
					self->ReleaseLuaRC(vm, 1);
			#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
				lua_rawseti(vm, 1, 1); // self[1] = value
				return 0;

				// 分组

			#ifdef USING_MULTI_GAME_WORLD
			case LuaSTG::GameObjectMember::WORLD:
				self->world = luaL_checkinteger(vm, 3); // interesting
				return 0;
			#endif // USING_MULTI_GAME_WORLD

				// 位置

			case LuaSTG::GameObjectMember::X:
				self->x = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::Y:
				self->y = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::DX:
				return luaL_error(vm, "property 'dx' is readonly.");
			case LuaSTG::GameObjectMember::DY:
				return luaL_error(vm, "property 'dy' is readonly.");

				// 运动学

			case LuaSTG::GameObjectMember::VX:
				self->vx = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::VY:
				self->vy = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::AX:
				self->ax = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::AY:
				self->ay = ctx.get_value<lua_Number>(3);
				return 0;
			#ifdef USER_SYSTEM_OPERATION
			case LuaSTG::GameObjectMember::MAXVX:
				self->max_vx = std::abs(ctx.get_value<lua_Number>(3));
				return 0;
			case LuaSTG::GameObjectMember::MAXVY:
				self->max_vy = std::abs(ctx.get_value<lua_Number>(3));
				return 0;
			case LuaSTG::GameObjectMember::MAXV:
				self->max_v = std::abs(ctx.get_value<lua_Number>(3));
				return 0;
			case LuaSTG::GameObjectMember::AG:
				self->ag = ctx.get_value<lua_Number>(3);
				return 0;
			#endif
			case LuaSTG::GameObjectMember::VSPEED:
				self->setSpeed(ctx.get_value<lua_Number>(3));
				return 0;
			case LuaSTG::GameObjectMember::VANGLE:
				self->setSpeedDirection(ctx.get_value<lua_Number>(3) * L_DEG_TO_RAD);
				return 0;

				// 碰撞体

			case LuaSTG::GameObjectMember::GROUP:
				if (LPOOL.isLockedByDetectIntersection(self)) {
					return luaL_error(vm, "illegal operation, lstg object 'group' property should not be modified in 'lstg.CollisionCheck'");
				}
				if (auto const group = luaL_checkinteger(vm, 3); group < 0 || group >= LOBJPOOL_GROUPN) {
					return luaL_error(vm, "invalid argument for property 'group', required 0 <= group <= %d.", LOBJPOOL_GROUPN - 1);
				}
				else if (self->group != group) {
					self->setGroup(group);
				}
				return 0;
			case LuaSTG::GameObjectMember::BOUND:
				self->bound = ctx.get_value<bool>(3);
				return 0;
			case LuaSTG::GameObjectMember::COLLI:
				self->colli = ctx.get_value<bool>(3);
				return 0;
			case LuaSTG::GameObjectMember::RECT:
				self->rect = ctx.get_value<bool>(3);
				self->UpdateCollisionCircleRadius();
				return 0;
			case LuaSTG::GameObjectMember::A:
			#ifdef GLOBAL_SCALE_COLLI_SHAPE
				self->a = ctx.get_value<lua_Number>(3) * LRES.GetGlobalImageScaleFactor();
			#else
				self->a = ctx.get_value<lua_Number>(3);
			#endif // GLOBAL_SCALE_COLLI_SHAPE
				self->UpdateCollisionCircleRadius();
				return 0;
			case LuaSTG::GameObjectMember::B:
			#ifdef GLOBAL_SCALE_COLLI_SHAPE
				self->b = ctx.get_value<lua_Number>(3) * LRES.GetGlobalImageScaleFactor();
			#else
				self->b = ctx.get_value<lua_Number>(3);
			#endif // GLOBAL_SCALE_COLLI_SHAPE
				self->UpdateCollisionCircleRadius();
				return 0;

				// 渲染

			case LuaSTG::GameObjectMember::LAYER:
				if (LPOOL.isRendering()) {
					return luaL_error(vm, "illegal operation, lstg object 'layer' property should not be modified in 'lstg.ObjRender'");
				}
				if (auto const layer = ctx.get_value<lua_Number>(3); self->layer != layer) {
					self->setLayer(layer);
				}
				return 0;
			case LuaSTG::GameObjectMember::HSCALE:
				self->hscale = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::VSCALE:
				self->vscale = ctx.get_value<lua_Number>(3);
				return 0;
			case LuaSTG::GameObjectMember::ROT:
				self->rot = ctx.get_value<lua_Number>(3) * L_DEG_TO_RAD;
				return 0;
			case LuaSTG::GameObjectMember::OMEGA:
			case LuaSTG::GameObjectMember::OMIGA:
				self->omega = ctx.get_value<lua_Number>(3) * L_DEG_TO_RAD;
				return 0;
			case LuaSTG::GameObjectMember::_BLEND:
				if (self->features.is_render_class) {
					self->blend_mode = TranslateBlendMode(vm, 3);
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_COLOR:
				if (self->features.is_render_class) {
					self->vertex_color = *Color::Cast(vm, 3);
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_A:
				if (self->features.is_render_class) {
					self->vertex_color.a = std::clamp<lua_Integer>(luaL_checkinteger(vm, 3), 0, 255);
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_R:
				if (self->features.is_render_class) {
					self->vertex_color.r = std::clamp<lua_Integer>(luaL_checkinteger(vm, 3), 0, 255);
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_G:
				if (self->features.is_render_class) {
					self->vertex_color.g = std::clamp<lua_Integer>(luaL_checkinteger(vm, 3), 0, 255);
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_B:
				if (self->features.is_render_class) {
					self->vertex_color.b = std::clamp<lua_Integer>(luaL_checkinteger(vm, 3), 0, 255);
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::ANI:
				return luaL_error(vm, "property 'ani' is readonly.");
			case LuaSTG::GameObjectMember::HIDE:
				self->hide = ctx.get_value<bool>(3);
				return 0;
			case LuaSTG::GameObjectMember::NAVI:
				self->navi = ctx.get_value<bool>(3);
				return 0;
			case LuaSTG::GameObjectMember::IMG:
				if (ctx.is_string(3)) {
					auto const resource_name = ctx.get_value<std::string_view>(3);
					if (!self->hasRenderResource() || self->getRenderResourceName() != resource_name) {
					#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
						ReleaseLuaRC(L, 1); // TODO: 默认 table 是第一个？
					#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
						self->ReleaseResource();
						if (!self->ChangeResource(resource_name))
							return luaL_error(vm, "can't find resource '%s' in image/animation/particle pool.", resource_name.data());
					#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
						ChangeLuaRC(L, 1); // TODO: 默认 table 是第一个？
					#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
					}
				}
				else {
				#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
					ReleaseLuaRC(L, 1); // TODO: 默认 table 是第一个？
				#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
					self->ReleaseResource();
				}
				return 0;
			#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
			case LuaSTG::GameObjectMember::RES_RC:
				if (self->features.is_render_class) {
					return luaL_error(vm, "property 'rc' is readonly.");
				}
				break;
			#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT

				// 更新控制

			case LuaSTG::GameObjectMember::TIMER:
				self->timer = luaL_checkinteger(vm, 3); // interesting
				return 0;
			#ifdef	LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
			case LuaSTG::GameObjectMember::PAUSE:
				self->pause = luaL_checkinteger(vm, 3);
				return 0;
			case LuaSTG::GameObjectMember::RESOLVEMOVE:
				self->resolve_move = ctx.get_value<bool>(3);
				return 0;
			#endif
			case LuaSTG::GameObjectMember::IGNORESUPERPAUSE:
				self->ignore_super_pause = ctx.get_value<bool>(3);
				return 0;

			default:
				break;
			}

			lua_rawset(vm, 1);
			return 0;
		}

		// NOLINTEND(*-reserved-identifier)

		// methods

		static int render(lua_State* const vm) {
			auto const self = as(vm, 1);
			self->Render();
			return 0;
		}

		// static methods

		static int allocateAndManage(lua_State* const vm) {
			auto const object = LPOOL.allocate();
			if (object == nullptr) {
				return luaL_error(vm, "failed to allocate object, object pool has been exhausted.");
			}
			return manage(vm, object, 1);
		}

	};

	bool GameObject::is(lua_State* const vm, int const index) {
		if (!lua_istable(vm, index)) {
			return false;
		}
		lua_rawgeti(vm, index, 3);
		if (!lua_islightuserdata(vm, -1)) {
			lua_pop(vm, 1);
			return false;
		}
		auto const ptr = static_cast<luastg::GameObject*>(lua_touserdata(vm, -1));
		lua_pop(vm, 1);
		return ptr != nullptr;
	}

	luastg::GameObject* GameObject::as(lua_State* const vm, int const index) {
		if (!lua_istable(vm, index)) {
			return nullptr;
		}
		lua_rawgeti(vm, index, 3);
		if (!lua_islightuserdata(vm, -1)) {
			lua_pop(vm, 1);
			return nullptr;
		}
		auto const ptr = static_cast<luastg::GameObject*>(lua_touserdata(vm, -1));
		lua_pop(vm, 1);
		return ptr;
	}

	//luastg::GameObject* GameObject::create(lua_State* vm);

	int GameObject::manage(lua_State* const vm, luastg::GameObject* const object, int const class_index) {
		lua::stack_t const ctx(vm);

		GameObjectFeatures features;
		features.read(vm, class_index);
		if (!features.is_class) {
			return luaL_error(vm, "invalid argument #1, luastg object class required for 'New'.");
		}

		object->features = features;

		auto const table = ctx.create_array(3);			// class object
		ctx.set_array_value(table, 1, lua::stack_index_t(class_index));
		ctx.set_array_value(table, 2, static_cast<int32_t>(object->id));
		ctx.set_array_value(table, 3, object);

		lua_pushlightuserdata(vm, &game_object_meta_table_key);		// class object k
		lua_gettable(vm, LUA_REGISTRYINDEX);						// class object mt
		lua_setmetatable(vm, table.value);							// class object

		pushGameObjectTable(vm);									// class object t
		auto const objects_table = ctx.index_of_top();
		auto const index = static_cast<int32_t>(object->id + 1);
		ctx.set_array_value(objects_table, index, table);			// class object t
		ctx.pop_value();											// class object

		ctx.push_value<bool>(object->features.has_callback_create);	// class object init

	#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
		static std::string _name("<null>");
		spdlog::debug("[object] new {}-{} (img = {})",
			object->id, object->unique_id, object->res ? object->res->GetResName() : _name);
	#endif

		return 2;
	}

	int GameObject::unmanage(lua_State* vm, luastg::GameObject* object) {
		return 0;
	}

	int GameObject::pushGameObjectTable(lua_State* const vm) {
		lua_pushlightuserdata(vm, &game_object_tables_key);
		lua_gettable(vm, LUA_REGISTRYINDEX);
		return 1;
	}

	void GameObject::registerClass(lua_State* const vm) {
		lua::stack_balancer_t sb(vm);
		lua::stack_t const ctx(vm);

		auto const meta_table = ctx.create_module(class_name);
		ctx.set_map_value(meta_table, "__index"sv, &GameObjectBinding::__index);
		ctx.set_map_value(meta_table, "__newindex"sv, &GameObjectBinding::__newindex);

		lua_pushlightuserdata(vm, &game_object_meta_table_key);
		ctx.push_value(meta_table);
		lua_settable(vm, LUA_REGISTRYINDEX);

		lua_pushlightuserdata(vm, &game_object_tables_key);
		auto const objects_table = ctx.create_array(LOBJPOOL_SIZE + 1); // TODO: 移除兼容代码
		ctx.set_array_value(objects_table, LOBJPOOL_SIZE + 1, meta_table);
		lua_settable(vm, LUA_REGISTRYINDEX);

		auto const lstg_table = ctx.push_module("lstg"sv);
		ctx.set_map_value(lstg_table, "GetAttr"sv, &GameObjectBinding::__index);
		ctx.set_map_value(lstg_table, "SetAttr"sv, &GameObjectBinding::__newindex);
		ctx.set_map_value(lstg_table, "DefaultRenderFunc"sv, &GameObjectBinding::render);
		ctx.set_map_value(lstg_table, "_New"sv, &GameObjectBinding::allocateAndManage);
		ctx.set_map_value(lstg_table, "ObjTable"sv, &pushGameObjectTable);
	}
}
