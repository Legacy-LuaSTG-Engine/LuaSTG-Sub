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
	void updateGameObjectFeatures(luastg::GameObjectFeatures& self, lua_State* const vm, int const idx) {
		lua::stack_t const ctx(vm);
		self.reset();
		if (!ctx.is_table(idx)) {
			return;
		}
		self.is_class = ctx.get_map_value<bool>(idx, "is_class"sv, false);
		if (!self.is_class) {
			return;
		}
		self.is_render_class = ctx.get_map_value<bool>(idx, ".render"sv, false);
		auto const default_function_mask = ctx.get_map_value<int32_t>(idx, "default_function"sv, 0);
	#define TEST_CALLBACK(CALLBACK) (default_function_mask & (1 << (CALLBACK))) ? false : true
		self.has_callback_create = TEST_CALLBACK(LGOBJ_CC_INIT);
		self.has_callback_destroy = TEST_CALLBACK(LGOBJ_CC_DEL);
		self.has_callback_update = TEST_CALLBACK(LGOBJ_CC_FRAME);
		self.has_callback_render = TEST_CALLBACK(LGOBJ_CC_RENDER);
		self.has_callback_trigger = TEST_CALLBACK(LGOBJ_CC_COLLI);
		self.has_callback_legacy_kill = TEST_CALLBACK(LGOBJ_CC_KILL);
	#undef TEST_CALLBACK
	}
	[[maybe_unused]] void changeParticlePoolBinding(luastg::GameObject const* const self, lua_State* const vm, int const idx) {
		if (self->features.is_render_class && self->hasParticlePool()) {
			auto const p = luastg::binding::ParticleSystem::Create(vm);
			p->res = static_cast<luastg::IResourceParticle*>(self->res);
			p->res->retain();
			p->ptr = self->ps;
			lua_rawseti(vm, idx, 4);
		}
	}
	[[maybe_unused]] void releaseParticlePoolBinding(luastg::GameObject const* const, lua_State* const vm, int const idx) {
		// release
		lua_rawgeti(vm, idx, 4);
		if (lua_isuserdata(vm, -1)) {
			if (auto const p = luastg::binding::ParticleSystem::Cast(vm, -1); p != nullptr) {
				if (p->res) {
					p->res->release();
					p->res = nullptr;
				}
				p->ptr = nullptr; // 交给游戏对象的 ReleaseResource 释放，这里没有持有所有权
			}
		}
		lua_pop(vm, 1);
		// object[4] = nil
		lua_pushnil(vm);
		lua_rawseti(vm, idx, 4);
	}

	struct GameObjectManagerCallbacks : luastg::IGameObjectManagerCallbacks {
		std::vector<lua_State*> lua_vm;
		std::vector<lua::stack_index_t> game_object_tables_index;

		GameObjectManagerCallbacks() {
			lua_vm.reserve(16);
			game_object_tables_index.reserve(16);
		}

		[[nodiscard]] std::string_view getCallbacksName() const noexcept override {
			return "lua"sv;
		}
		void onCreate(luastg::GameObject*) override {
		}
		void onDestroy(luastg::GameObject* const object) override {
		#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
			static std::string null_name("<null>");
			spdlog::debug("[object] free {}-{} (img = {})", object->id, object->unique_id, object->res ? object->res->GetResName() : null_name);
		#endif

			auto const vm = getInstance().lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = getInstance().game_object_tables_index.back();
			auto const lua_index = static_cast<int32_t>(object->id + 1);

			auto const object_table = ctx.get_array_value<lua::stack_index_t>(table, lua_index); // ... t ... object
			ctx.set_array_value(object_table, 3, std::nullopt); // object[3] = nil
		#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
			releaseParticlePoolBinding(object, vm, object_table.value); // releaseParticlePoolBinding(object[4]); object[4] = nil
		#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
			ctx.pop_value(); // ... t ...

			ctx.set_array_value(table, lua_index, std::nullopt); // table[lua_index] = nil
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
			game_object_tables_index.emplace_back(lua_gettop(vm));
		}
		void afterBatch() {
			auto const vm = lua_vm.back();
			lua_settop(vm, game_object_tables_index.back().value - 1);
			game_object_tables_index.pop_back();
		}

		static GameObjectManagerCallbacks& getInstance() {
			static GameObjectManagerCallbacks instance;
			return instance;
		}
	};

	struct GameObjectCallbacks : luastg::IGameObjectCallbacks {
		std::string_view getCallbacksName(luastg::GameObject*) const noexcept override {
			return "lua"sv;
		}
		void onQueueToDestroy(luastg::GameObject* self, std::string_view const reason) override {
			auto const vm = GameObjectManagerCallbacks::getInstance().lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = GameObjectManagerCallbacks::getInstance().game_object_tables_index.back();
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
		void onLateUpdate(luastg::GameObject*) override {}
		void onRender(luastg::GameObject* self) override {
			call(self, LGOBJ_CC_RENDER);
		}
		void onTrigger(luastg::GameObject* self, luastg::GameObject* other) override {
			auto const vm = GameObjectManagerCallbacks::getInstance().lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = GameObjectManagerCallbacks::getInstance().game_object_tables_index.back();
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
			auto const vm = GameObjectManagerCallbacks::getInstance().lua_vm.back();
			lua::stack_t const ctx(vm);

			auto const table = GameObjectManagerCallbacks::getInstance().game_object_tables_index.back();
			auto const lua_index = static_cast<int32_t>(self->id + 1);

			auto const object = ctx.get_array_value<lua::stack_index_t>(table, lua_index); // ... t ... object
			auto const object_class = ctx.get_array_value<lua::stack_index_t>(object, 1); // ... t ... object class
			std::ignore = ctx.get_array_value<lua::stack_index_t>(object_class, type);	 // ... t ... object class callback
			ctx.push_value(object); // ... t ... object class callback object
			lua_call(vm, 1, 0); // ... t ... object class
			ctx.pop_value(2); // ... t ...
		}

		static GameObjectCallbacks& getInstance() {
			static GameObjectCallbacks instance;
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
				lua_pushinteger(vm, self->world); // interesting
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
				updateGameObjectFeatures(self->features, vm, 3); // 刷新对象的 class
				if (!self->features.is_class)
					return luaL_error(vm, "invalid argument for property 'class', required luastg object class.");
			#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
				if (!self->features.is_render_class)
					releaseParticlePoolBinding(self, vm, 1);
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
					self->vertex_color.a = static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(3), 0, 255));
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_R:
				if (self->features.is_render_class) {
					self->vertex_color.r = static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(3), 0, 255));
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_G:
				if (self->features.is_render_class) {
					self->vertex_color.g = static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(3), 0, 255));
					return 0;
				}
				break;
			case LuaSTG::GameObjectMember::_B:
				if (self->features.is_render_class) {
					self->vertex_color.b = static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(3), 0, 255));
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
						releaseParticlePoolBinding(self, vm, 1);
					#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
						self->ReleaseResource();
						if (!self->ChangeResource(resource_name))
							return luaL_error(vm, "can't find resource '%s' in image/animation/particle pool.", resource_name.data());
					#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
						changeParticlePoolBinding(self, vm, 1);
					#endif // LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
					}
				}
				else {
				#ifdef LUASTG_GAME_OBJECT_PARTICLE_SYSTEM_OBJECT
					releaseParticlePoolBinding(self, vm, 1);
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
			#ifdef LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
			case LuaSTG::GameObjectMember::PAUSE:
				self->pause = luaL_checkinteger(vm, 3);
				return 0;
			case LuaSTG::GameObjectMember::RESOLVEMOVE:
				self->resolve_move = ctx.get_value<bool>(3);
				return 0;
			#endif // LUASTG_ENABLE_GAME_OBJECT_PROPERTY_PAUSE
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
		static int setResourceRenderState(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const blend = TranslateBlendMode(vm, 2);
			core::Color4B const color(
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(4), 0, 255)),
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(5), 0, 255)),
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(6), 0, 255)),
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(3), 0, 255))// 这个才是 a 通道
			);
			self->setResourceRenderState(blend, color);
			return 0;
		}
		static int setParticleRenderState(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const blend = TranslateBlendMode(vm, 2);
			core::Color4B const color(
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(4), 0, 255)),
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(5), 0, 255)),
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(6), 0, 255)),
				static_cast<uint8_t>(std::clamp(ctx.get_value<int32_t>(3), 0, 255))// 这个才是 a 通道
			);
			self->setParticleRenderState(blend, color);
			return 0;
		}
		static int stopParticle(lua_State* const vm) {
			auto const self = as(vm, 1);
			self->stopParticle();
			return 0;
		}
		static int startParticle(lua_State* const vm) {
			auto const self = as(vm, 1);
			self->startParticle();
			return 0;
		}
		static int getParticleCount(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(static_cast<int32_t>(self->getParticleCount()));
			return 1;
		}
		static int getParticleEmission(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->getParticleEmission());
			return 1;
		}
		static int setParticleEmission(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const value = std::max<int32_t>(0, ctx.get_value<int32_t>(2));
			self->setParticleEmission(value);
			return 0;
		}
		static int isInRect(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const left = ctx.get_value<double>(2);
			auto const right = ctx.get_value<double>(3);
			auto const bottom = ctx.get_value<double>(4);
			auto const top = ctx.get_value<double>(5);
			ctx.push_value(self->isInRect(left, right, bottom, top));
			return 1;
		}
		static int isIntersect(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const other = as(vm, 2);
		#ifdef USING_MULTI_GAME_WORLD
			if (auto const ignore_world_mask = ctx.get_value<bool>(3)) {
			#endif // USING_MULTI_GAME_WORLD
				ctx.push_value(self->isIntersect(other));
			#ifdef USING_MULTI_GAME_WORLD
			}
			else {
				ctx.push_value(LPOOL.CheckWorlds(self->world, other->world) && self->isIntersect(other));
			}
		#endif // USING_MULTI_GAME_WORLD
			return 1;
		}

		// static methods

		static int allocateAndManage(lua_State* const vm) {
			auto const object = LPOOL.allocateWithCallbacks(&GameObjectCallbacks::getInstance());
			if (object == nullptr) {
				return luaL_error(vm, "failed to allocate object, object pool has been exhausted.");
			}

			lua::stack_t const ctx(vm);

			GameObjectFeatures features{};
			updateGameObjectFeatures(features, vm, 1);
			if (!features.is_class) {
				return luaL_error(vm, "invalid argument #1, luastg object class required for 'New'.");
			}

			object->features = features;

			auto const table = ctx.create_array(3);			// class object
			ctx.set_array_value(table, 1, lua::stack_index_t(1));
			ctx.set_array_value(table, 2, static_cast<int32_t>(object->id));
			ctx.set_array_value(table, 3, static_cast<void*>(object));

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
		static int dirtyReset(lua_State* const vm) {
			auto const self = as(vm, 1);
			LPOOL.DirtResetObject(self);
			return 0;
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
					GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
					LPOOL.updateNext();
					GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
					return 0;
				}
			}
			// version 1
			GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
			LPOOL.updateNextLegacy();
			GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
			return 0;
		}
		static int resetGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
			LPOOL.ResetPool();
		#if (defined(_DEBUG) && defined(LuaSTG_enable_GameObjectManager_Debug))
			for (int i = 1; i <= LOBJPOOL_SIZE; i += 1) {
				// 确保所有 lua 侧对象都被正确回收
				lua_rawgeti(vm, GameObjectManagerCallbacks::getInstance().game_object_tables_index.back().value, i);
				assert(!lua_istable(vm, -1));
				lua_pop(vm, 1);
			}
		#endif
			GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
			return 0;
		}
		static int updateGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			// version 2
			if (lua::stack_t const ctx(vm); ctx.is_number(1)) {
				if (auto const version = ctx.get_value<int32_t>(1); version == 2) {
					GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
					LPOOL.updateMovements();
					GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
					return 0;
				}
			}
			// version 1
			GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
			LPOOL.updateMovementsLegacy();
			GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
			return 0;
		}
		static int renderGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
			LPOOL.render();
			GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
			return 0;
		}
		static int boundCheckGameObjectManager(lua_State* const vm) {
			// TODO: 移动到 GameObjectManager 绑定
			// version 2
			if (lua::stack_t const ctx(vm); ctx.is_number(1)) {
				if (auto const version = ctx.get_value<int32_t>(1); version == 2) {
					GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
					LPOOL.detectOutOfWorldBound();
					GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
					return 0;
				}
			}
			// version 1
			GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
			LPOOL.detectOutOfWorldBoundLegacy();
			GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
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
				GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
				LPOOL.detectIntersectionLegacy(group1, group2);
				GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
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
				GameObjectManagerCallbacks::getInstance().lua_vm.push_back(vm);
				LPOOL.detectIntersection(group_pairs);
				GameObjectManagerCallbacks::getInstance().lua_vm.pop_back();
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
		static int isValid(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			ctx.push_value(is(vm, 1));
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
		ctx.set_map_value(lstg_table, "SetImgState"sv, &GameObjectBinding::setResourceRenderState);
		ctx.set_map_value(lstg_table, "SetParState"sv, &GameObjectBinding::setParticleRenderState);
		ctx.set_map_value(lstg_table, "ParticleStop"sv, &GameObjectBinding::stopParticle);
		ctx.set_map_value(lstg_table, "ParticleFire"sv, &GameObjectBinding::startParticle);
		ctx.set_map_value(lstg_table, "ParticleGetn"sv, &GameObjectBinding::getParticleCount);
		ctx.set_map_value(lstg_table, "ParticleGetEmission"sv, &GameObjectBinding::getParticleEmission);
		ctx.set_map_value(lstg_table, "ParticleSetEmission"sv, &GameObjectBinding::setParticleEmission);
		ctx.set_map_value(lstg_table, "BoxCheck"sv, &GameObjectBinding::isInRect);
		ctx.set_map_value(lstg_table, "ColliCheck"sv, &GameObjectBinding::isIntersect);
		ctx.set_map_value(lstg_table, "_New"sv, &GameObjectBinding::allocateAndManage);
		ctx.set_map_value(lstg_table, "ResetObject"sv, &GameObjectBinding::dirtyReset); // TODO: WTF?
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
		ctx.set_map_value(lstg_table, "IsValid"sv, &GameObjectBinding::isValid);
		ctx.set_map_value(lstg_table, "ObjTable"sv, &pushGameObjectTable);

		LPOOL.addCallbacks(&GameObjectManagerCallbacks::getInstance());
		GameObjectManagerCallbacks::getInstance().lua_vm.reserve(16);
		GameObjectManagerCallbacks::getInstance().lua_vm.push_back(LAPP.GetLuaEngine());
	}
}
