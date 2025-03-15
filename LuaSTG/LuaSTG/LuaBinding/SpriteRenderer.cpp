#include "SpriteRenderer.hpp"
#include "Sprite.hpp"
#include "lua/plus.hpp"
#include "LuaWrapper.hpp"
#include "LuaWrapperMisc.hpp"
#include "AppFrame.h"

namespace LuaSTG::Sub::LuaBinding {
	std::string_view const SpriteRenderer::class_name{ "lstg.SpriteRenderer" };

	struct SpriteRendererBinding : SpriteRenderer {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __gc(lua_State* vm) {
			if (auto const self = as(vm, 1); self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}
		static int __tostring(lua_State* vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
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

		// NOLINTEND(*-reserved-identifier)

		// method

		static int setPosition(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->position.x = ctx.get_value<float>(1 + 1);
			self->position.y = ctx.get_value<float>(1 + 2);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setScale(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->scale.x = ctx.get_value<float>(1 + 1);
			self->scale.y = ctx.get_value<float>(1 + 2, self->scale.x);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setRotation(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->rotation = ctx.get_value<float>(1 + 1);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setSprite(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const sprite = Sprite::as(vm, 1 + 1);
			self->data->setSprite(sprite->data);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setColor(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (ctx.index_of_top() >= 4) {
				auto const c1 = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				auto const c2 = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 2);
				auto const c3 = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 3);
				auto const c4 = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 4);
				self->data->setColor(c1, c2, c3, c4);
			}
			else {
				auto const color = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				self->data->setColor(color);
			}
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setLegacyBlendState(lua_State* vm) {
			using VertexColorBlendState = Core::Graphics::IRenderer::VertexColorBlendState;
			using BlendState = Core::Graphics::IRenderer::BlendState;

			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			switch (LuaSTGPlus::TranslateBlendMode(vm, 1 + 1)) {  // NOLINT(clang-diagnostic-switch-enum)
			default:
			case LuaSTGPlus::BlendMode::MulAlpha:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Alpha);
				break;
			case LuaSTGPlus::BlendMode::MulAdd:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Add);
				break;
			case LuaSTGPlus::BlendMode::MulRev:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::RevSub);
				break;
			case LuaSTGPlus::BlendMode::MulSub:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Sub);
				break;
			case LuaSTGPlus::BlendMode::AddAlpha:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Alpha);
				break;
			case LuaSTGPlus::BlendMode::AddAdd:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Add);
				break;
			case LuaSTGPlus::BlendMode::AddRev:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::RevSub);
				break;
			case LuaSTGPlus::BlendMode::AddSub:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Sub);
				break;
			case LuaSTGPlus::BlendMode::AlphaBal:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Inv);
				break;
			case LuaSTGPlus::BlendMode::MulMin:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Min);
				break;
			case LuaSTGPlus::BlendMode::MulMax:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Max);
				break;
			case LuaSTGPlus::BlendMode::MulMutiply:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Mul);
				break;
			case LuaSTGPlus::BlendMode::MulScreen:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::Screen);
				break;
			case LuaSTGPlus::BlendMode::AddMin:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Min);
				break;
			case LuaSTGPlus::BlendMode::AddMax:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Max);
				break;
			case LuaSTGPlus::BlendMode::AddMutiply:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Mul);
				break;
			case LuaSTGPlus::BlendMode::AddScreen:
				self->data->setLegacyBlendState(VertexColorBlendState::Add, BlendState::Screen);
				break;
			case LuaSTGPlus::BlendMode::One:
				self->data->setLegacyBlendState(VertexColorBlendState::Mul, BlendState::One);
				break;
			}

			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int draw(lua_State* vm) {
			auto const self = as(vm, 1);
			self->data->setTransform(self->position, self->scale, self->rotation);  // TODO: TBD
			self->data->setZ(0.5); // TODO: allow custom
			self->data->draw(LAPP.GetAppModel()->getRenderer());
			return 0;
		}

		// static method

		static int create(lua_State* vm) {
			if (lua::stack_t const ctx(vm); ctx.index_of_top() >= 1) {
				auto const sprite = Sprite::as(vm, 1);
				auto const self = SpriteRenderer::create(vm);
				if (!Core::Graphics::ISpriteRenderer::create(&self->data)) {
					return luaL_error(vm, "create SpriteRenderer failed");
				}
				self->data->setSprite(sprite->data);
			}
			else {
				auto const self = SpriteRenderer::create(vm);
				if (!Core::Graphics::ISpriteRenderer::create(&self->data)) {
					return luaL_error(vm, "create SpriteRenderer failed");
				}
			}
			return 1;
		}
	};

	bool SpriteRenderer::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	SpriteRenderer* SpriteRenderer::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<SpriteRenderer>(index);
	}
	SpriteRenderer* SpriteRenderer::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<SpriteRenderer>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void SpriteRenderer::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "setPosition", &SpriteRendererBinding::setPosition);
		ctx.set_map_value(method_table, "setScale", &SpriteRendererBinding::setScale);
		ctx.set_map_value(method_table, "setRotation", &SpriteRendererBinding::setRotation);
		ctx.set_map_value(method_table, "setSprite", &SpriteRendererBinding::setSprite);
		ctx.set_map_value(method_table, "setColor", &SpriteRendererBinding::setColor);
		ctx.set_map_value(method_table, "setLegacyBlendState", &SpriteRendererBinding::setLegacyBlendState);
		ctx.set_map_value(method_table, "draw", &SpriteRendererBinding::draw);
		ctx.set_map_value(method_table, "create", &SpriteRendererBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &SpriteRendererBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &SpriteRendererBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &SpriteRendererBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
