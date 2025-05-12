#include "LuaBinding/modern/GameObject.hpp"
#include "LuaBinding/generated/GameObjectMember.hpp"
#include "GameObject/GameObjectPool.h"
#include "AppFrame.h"
#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/LuaWrapperMisc.hpp"
#include "lua/plus.hpp"
#include "core/FixedObjectPool.hpp"

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

	struct GameObjectManagerCallbacks : luastg::IGameObjectManagerCallbacks {
		std::vector<lua_State*> lua_vm;
		lua::stack_index_t game_object_tables_index{};

		std::string_view getCallbacksName() const noexcept override {
			return "lua"sv;
		}
		void onBeforeBatchDestroy() override {
			beforeBatch();
		}
		void onAfterBatchDestroy() override {
			afterBatch();
		}
		void onBeforeBatchUpdate() override {
			beforeBatch();
		}
		void onAfterBatchUpdate() override {
			afterBatch();
		}
		void onBeforeBatchRender() override {
			beforeBatch();
		}
		void onAfterBatchRender() override {
			afterBatch();
		}
		void onBeforeBatchOutOfWorldBoundCheck() override {
			beforeBatch();
		}
		void onAfterBatchOutOfWorldBoundCheck() override {
			afterBatch();
		}
		void onBeforeBatchIntersectDetect() override {
			beforeBatch();
		}
		void onAfterBatchIntersectDetect() override {
			afterBatch();
		}

		void beforeBatch() {
			auto const vm = lua_vm.back();
			luastg::binding::GameObject::pushGameObjectTable(vm);
			game_object_tables_index.value = lua_gettop(vm);
		}
		void afterBatch() {
			auto const vm = lua_vm.back();
			lua_settop(vm, game_object_tables_index.value - 1);
			game_object_tables_index = {};
		}
	};

	GameObjectManagerCallbacks game_object_manager_callbacks;

	struct GameObjectCallbacks : luastg::IGameObjectCallbacks {
		size_t id{};
		IGameObjectCallbacks* next_callbacks{};

		std::string_view getCallbacksName(luastg::GameObject*) const noexcept override {
			return "lua"sv;
		}
		IGameObjectCallbacks* getNextCallbacks(luastg::GameObject*) const noexcept override { return next_callbacks; }
		void setNextCallbacks(luastg::GameObject*, IGameObjectCallbacks* callbacks) override { next_callbacks = callbacks; }
		void onCreate(luastg::GameObject* self) override {}
		void onDestroy(luastg::GameObject* self) override {
		#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
			static std::string null_name("<null>");
			spdlog::debug("[object] free {}-{} (img = {})", self->id, self->unique_id, self->res ? self->res->GetResName() : null_name);
		#endif

			auto const vm = game_object_manager_callbacks.lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = game_object_manager_callbacks.game_object_tables_index;
			auto const lua_index = static_cast<int32_t>(self->id + 1);

			auto const object = ctx.get_array_value<lua::stack_index_t>(table, lua_index); // ... t ... object
			ctx.set_array_value(object, 3, std::nullopt); // object[3] = nil
		#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
			self->ReleaseLuaRC(vm, object.value); // 释放可能的粒子系统
		#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
			ctx.pop_value(); // ... t ...

			ctx.set_array_value(table, lua_index, std::nullopt); // table[lua_index] = nil
			getPool().free(id);
			id = 0;
		}
		void onQueueToDestroy(luastg::GameObject* self, std::string_view const reason) override {
			auto const vm = game_object_manager_callbacks.lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = game_object_manager_callbacks.game_object_tables_index;
			auto const lua_index = static_cast<int32_t>(self->id + 1);

			auto const object = ctx.get_array_value<lua::stack_index_t>(table, lua_index); // ... t ... object
			auto const object_class = ctx.get_array_value<lua::stack_index_t>(object, 1); // ... t ... object class
			std::ignore = ctx.get_array_value<lua::stack_index_t>(object_class, LGOBJ_CC_DEL);	 // ... t ... object class callback
			ctx.push_value(object); // ... t ... object class callback object
			ctx.push_value(reason); // ... t ... object class callback object reason
			lua_call(vm, 2, 0); // ... t ... object class
			ctx.pop_value(2); // ... t ...
		}
		void onUpdate(luastg::GameObject* self) override {
			call(self, LGOBJ_CC_FRAME);
		}
		void onLateUpdate(luastg::GameObject* self) override {}
		void onRender(luastg::GameObject* self) override {
			call(self, LGOBJ_CC_RENDER);
		}
		void onTrigger(luastg::GameObject* self, luastg::GameObject* other) override {
			auto const vm = game_object_manager_callbacks.lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = game_object_manager_callbacks.game_object_tables_index;
			auto const lua_index = static_cast<int32_t>(self->id + 1);
			auto const other_lua_index = static_cast<int32_t>(other->id + 1);

			auto const object = ctx.get_array_value<lua::stack_index_t>(table, lua_index); // ... t ... object
			auto const object_class = ctx.get_array_value<lua::stack_index_t>(object, 1); // ... t ... object class
			std::ignore = ctx.get_array_value<lua::stack_index_t>(object_class, LGOBJ_CC_COLLI);	 // ... t ... object class callback
			ctx.push_value(object); // ... t ... object class callback object
			std::ignore = ctx.get_array_value<lua::stack_index_t>(table, other_lua_index); // ... t ... object class callback object other
			lua_call(vm, 2, 0); // ... t ... object class
			ctx.pop_value(2); // ... t ...
		}

		static void call(luastg::GameObject const* const self, int const type) {
			auto const vm = game_object_manager_callbacks.lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = game_object_manager_callbacks.game_object_tables_index;
			auto const lua_index = static_cast<int32_t>(self->id + 1);

			auto const object = ctx.get_array_value<lua::stack_index_t>(table, lua_index); // ... t ... object
			auto const object_class = ctx.get_array_value<lua::stack_index_t>(object, 1); // ... t ... object class
			std::ignore = ctx.get_array_value<lua::stack_index_t>(object_class, type);	 // ... t ... object class callback
			ctx.push_value(object); // ... t ... object class callback object
			lua_call(vm, 1, 0); // ... t ... object class
			ctx.pop_value(2); // ... t ...
		}

		using Pool = core::FixedObjectPool<GameObjectCallbacks, LOBJPOOL_SIZE>;

		static Pool& getPool() {
			static Pool instance{};
			return instance;
		}
	};
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

			case LuaSTG::GameObjectMember::STATUS:
			{
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
			size_t callbacks_id{};
			if (!GameObjectCallbacks::getPool().alloc(callbacks_id)) {
				return luaL_error(vm, "failed to allocate object, object pool has been exhausted.");
			}
			auto const callbacks = GameObjectCallbacks::getPool().object(callbacks_id);
			callbacks->id = callbacks_id;

			auto const object = LPOOL.allocateWithCallbacks(callbacks);
			if (object == nullptr) {
				return luaL_error(vm, "failed to allocate object, object pool has been exhausted.");
			}

			lua::stack_t const ctx(vm);

			GameObjectFeatures features{};
			features.read(vm, 1);
			if (!features.is_class) {
				return luaL_error(vm, "invalid argument #1, luastg object class required for 'New'.");
			}

			object->features = features;

			auto const table = ctx.create_array(3);			// class object
			ctx.set_array_value(table, 1, lua::stack_index_t(1));
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
		static int queueToFree(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const has_callback = LPOOL.queueToFree(self);
			ctx.push_value(has_callback);
			return 1;
		}
		static int queueToFreeLegacyKillMode(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const has_callback = LPOOL.queueToFree(self, true);
			ctx.push_value(has_callback);
			return 1;
		}
		static int updateNext(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			// version 2
			if (lua::stack_t const ctx(vm); ctx.is_number(1)) {
				if (auto const version = ctx.get_value<int32_t>(1); version == 2) {
					game_object_manager_callbacks.lua_vm.push_back(vm);
					LPOOL.updateNext();
					game_object_manager_callbacks.lua_vm.pop_back();
					return 0;
				}
			}
			// version 1
			game_object_manager_callbacks.lua_vm.push_back(vm);
			LPOOL.updateNextLegacy();
			game_object_manager_callbacks.lua_vm.pop_back();
			return 0;
		}
		static int resetGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			game_object_manager_callbacks.lua_vm.push_back(vm);
			LPOOL.ResetPool();
			game_object_manager_callbacks.lua_vm.pop_back();
			return 0;
		}
		static int updateGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			// version 2
			if (lua::stack_t const ctx(vm); ctx.is_number(1)) {
				if (auto const version = ctx.get_value<int32_t>(1); version == 2) {
					game_object_manager_callbacks.lua_vm.push_back(vm);
					LPOOL.updateMovements();
					game_object_manager_callbacks.lua_vm.pop_back();
					return 0;
				}
			}
			// version 1
			game_object_manager_callbacks.lua_vm.push_back(vm);
			LPOOL.updateMovementsLegacy();
			game_object_manager_callbacks.lua_vm.pop_back();
			return 0;
		}
		static int renderGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			game_object_manager_callbacks.lua_vm.push_back(vm);
			LPOOL.render();
			game_object_manager_callbacks.lua_vm.pop_back();
			return 0;
		}
		static int boundCheckGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			// version 2
			if (lua::stack_t const ctx(vm); ctx.is_number(1)) {
				if (auto const version = ctx.get_value<int32_t>(1); version == 2) {
					game_object_manager_callbacks.lua_vm.push_back(vm);
					LPOOL.detectOutOfWorldBound();
					game_object_manager_callbacks.lua_vm.pop_back();
					return 0;
				}
			}
			// version 1
			game_object_manager_callbacks.lua_vm.push_back(vm);
			LPOOL.detectOutOfWorldBoundLegacy();
			game_object_manager_callbacks.lua_vm.pop_back();
			return 0;
		}
		static int intersectDetectGameObjectManager(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			if (LPOOL.isDetectingIntersect()) {
				return luaL_error(vm, "invalid operation");
			}
			if (ctx.is_number(1) && ctx.is_number(2)) {
				auto const group1 = ctx.get_value<uint32_t>(1);
				auto const group2 = ctx.get_value<uint32_t>(2);
				if (group1 < 0 || group1 >= LOBJPOOL_GROUPN) {
					return luaL_error(vm, "invalid collision group <%d>", group1);
				}
				if (group2 < 0 || group2 >= LOBJPOOL_GROUPN) {
					return luaL_error(vm, "invalid collision group <%d>", group1);
				}
				game_object_manager_callbacks.lua_vm.push_back(vm);
				LPOOL.detectIntersectionLegacy(group1, group2);
				game_object_manager_callbacks.lua_vm.pop_back();
				return 0;
			}
			if (ctx.is_table(1)) {
				// Stage 1
				auto const group_count = ctx.get_array_size(1);
				if (group_count == 0) {
					return 0; // early return
				}
				// Stage 2
				std::array<uint32_t, 32> stack_buffer{};
				std::pmr::monotonic_buffer_resource local_memory_resource(
					stack_buffer.data(), stack_buffer.size() * sizeof(uint32_t),
					std::pmr::get_default_resource());
				std::pmr::vector<GameObjectPool::IntersectionDetectionGroupPair> group_pairs{ &local_memory_resource };
				group_pairs.reserve(group_count);
				for (int32_t i = 1; i <= static_cast<int32_t>(group_count); i += 1) {
					auto const group_pair = ctx.get_array_value<lua::stack_index_t>(1, i);
					auto const group1 = ctx.get_array_value<uint32_t>(group_pair, 1);
					auto const group2 = ctx.get_array_value<uint32_t>(group_pair, 2);
					if (group1 < 0 || group1 >= LOBJPOOL_GROUPN) {
						return luaL_error(vm, "invalid collision group <%d>", group1);
					}
					if (group2 < 0 || group2 >= LOBJPOOL_GROUPN) {
						return luaL_error(vm, "invalid collision group <%d>", group1);
					}
					ctx.pop_value();
					group_pairs.emplace_back(group1, group2);
				}
				// Stage 3
				game_object_manager_callbacks.lua_vm.push_back(vm);
				LPOOL.detectIntersection(group_pairs);
				game_object_manager_callbacks.lua_vm.pop_back();
				return 0;
			}
			return luaL_error(vm, "invalid parameters");
		}
		static int getUpdateListFirst(lua_State* const vm) {
			if (auto const object = LPOOL.getUpdateListFirst(); object == nullptr) {
				lua_pushinteger(vm, 0);
			}
			else {
				lua_pushinteger(vm, static_cast<lua_Integer>(object->id + 1));
			}
			return 1;
		}
		static int getUpdateListNext(lua_State* const vm) {
			auto const id = static_cast<size_t>(luaL_checkinteger(vm, 1));
			if (auto const object = LPOOL.getUpdateListNext(id - 1); object == nullptr) {
				lua_pushinteger(vm, 0);
			}
			else {
				lua_pushinteger(vm, static_cast<lua_Integer>(object->id + 1));
			}
			return 1;
		}
		static int getDetectListFirst(lua_State* const vm) {
			auto const group = static_cast<size_t>(luaL_checkinteger(vm, 1));
			if (auto const object = LPOOL.getDetectListFirst(group); object == nullptr) {
				lua_pushinteger(vm, 0);
			}
			else {
				lua_pushinteger(vm, static_cast<lua_Integer>(object->id + 1));
			}
			return 1;
		}
		static int getDetectListNext(lua_State* const vm) {
			auto const group = static_cast<size_t>(luaL_checkinteger(vm, 1));
			auto const id = static_cast<size_t>(luaL_checkinteger(vm, 2));
			if (auto const object = LPOOL.getDetectListNext(group, id - 1); object == nullptr) {
				lua_pushinteger(vm, 0);
			}
			else {
				lua_pushinteger(vm, static_cast<lua_Integer>(object->id + 1));
			}
			return 1;
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
			luaL_error(vm, "invalid lstg object");
			return nullptr;
		}
		lua_rawgeti(vm, index, 3);
		if (!lua_islightuserdata(vm, -1)) {
			lua_pop(vm, 1);
			luaL_error(vm, "invalid lstg object");
			return nullptr;
		}
		auto const ptr = static_cast<luastg::GameObject*>(lua_touserdata(vm, -1));
		lua_pop(vm, 1);
		if (ptr == nullptr) {
			luaL_error(vm, "invalid lstg object");
		}
		return ptr;
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
		ctx.set_map_value(lstg_table, "_Del"sv, &GameObjectBinding::queueToFree);
		ctx.set_map_value(lstg_table, "_Kill"sv, &GameObjectBinding::queueToFreeLegacyKillMode);
		ctx.set_map_value(lstg_table, "AfterFrame"sv, &GameObjectBinding::updateNext);
		ctx.set_map_value(lstg_table, "ResetPool"sv, &GameObjectBinding::resetGameObjectManager);
		ctx.set_map_value(lstg_table, "ObjFrame"sv, &GameObjectBinding::updateGameObjectManager);
		ctx.set_map_value(lstg_table, "ObjRender"sv, &GameObjectBinding::renderGameObjectManager);
		ctx.set_map_value(lstg_table, "BoundCheck"sv, &GameObjectBinding::boundCheckGameObjectManager);
		ctx.set_map_value(lstg_table, "CollisionCheck"sv, &GameObjectBinding::intersectDetectGameObjectManager);
		ctx.set_map_value(lstg_table, "_UpdateListFirst"sv, &GameObjectBinding::getUpdateListFirst);
		ctx.set_map_value(lstg_table, "_UpdateListNext"sv, &GameObjectBinding::getUpdateListNext);
		ctx.set_map_value(lstg_table, "_DetectListFirst"sv, &GameObjectBinding::getDetectListFirst);
		ctx.set_map_value(lstg_table, "_DetectListNext"sv, &GameObjectBinding::getDetectListNext);
		ctx.set_map_value(lstg_table, "ObjTable"sv, &pushGameObjectTable);

		LPOOL.addCallbacks(&game_object_manager_callbacks);
		game_object_manager_callbacks.lua_vm.reserve(16);
		game_object_manager_callbacks.lua_vm.push_back(LAPP.GetLuaEngine());
	}
}
