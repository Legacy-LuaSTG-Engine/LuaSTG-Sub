#include "SpriteRenderer.hpp"
#include "Sprite.hpp"
#include "lua/plus.hpp"
#include "LuaWrapper.hpp"
#include "LuaWrapperMisc.hpp"
#include "AppFrame.h"
#include "GameResource/LegacyBlendStateHelper.hpp"

namespace luastg::binding {
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

		static int setTransform(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->position.x = ctx.get_value<float>(1 + 1);
			self->position.y = ctx.get_value<float>(1 + 2);
			self->rotation = ctx.get_value<float>(1 + 3, 0.0f);
			self->scale.x = ctx.get_value<float>(1 + 4, 1.0f);
			self->scale.y = ctx.get_value<float>(1 + 5, self->scale.x);
			self->data->setTransform(self->position, self->scale, self->rotation);  // TODO: TBD
			self->data->setZ(0.5); // TODO: allow custom
			self->is_dirty = false;
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setPosition(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->position.x = ctx.get_value<float>(1 + 1);
			self->position.y = ctx.get_value<float>(1 + 2);
			self->is_dirty = true;
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setScale(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->scale.x = ctx.get_value<float>(1 + 1);
			self->scale.y = ctx.get_value<float>(1 + 2, self->scale.x);
			self->is_dirty = true;
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setRotation(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			self->rotation = ctx.get_value<float>(1 + 1);
			self->is_dirty = true;
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
				auto const c1 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				auto const c2 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 2);
				auto const c3 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 3);
				auto const c4 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 4);
				self->data->setColor(c1, c2, c3, c4);
			}
			else {
				auto const color = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				self->data->setColor(color);
			}
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setLegacyBlendState(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const blend = luastg::TranslateBlendMode(vm, 1 + 1);
			[[maybe_unused]] auto const [v, b] = luastg::translateLegacyBlendState(blend);
			self->data->setLegacyBlendState(v, b);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int draw(lua_State* vm) {
			auto const self = as(vm, 1);
			if (self->is_dirty) {
				self->data->setTransform(self->position, self->scale, self->rotation);  // TODO: TBD
				self->data->setZ(0.5); // TODO: allow custom
			}
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
		ctx.set_map_value(method_table, "setTransform", &SpriteRendererBinding::setTransform);
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

namespace luastg::binding {
	std::string_view const SpriteRectRenderer::class_name{ "lstg.SpriteRectRenderer" };

	struct SpriteRectRendererBinding : SpriteRectRenderer {
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

		static int setRect(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const left = ctx.get_value<float>(1 + 1);
			auto const right = ctx.get_value<float>(1 + 2);
			auto const bottom = ctx.get_value<float>(1 + 3);
			auto const top = ctx.get_value<float>(1 + 4);
			self->data->setTransform(Core::RectF(left, top, right, bottom));
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
				auto const c1 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				auto const c2 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 2);
				auto const c3 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 3);
				auto const c4 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 4);
				self->data->setColor(c1, c2, c3, c4);
			}
			else {
				auto const color = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				self->data->setColor(color);
			}
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setLegacyBlendState(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const blend = luastg::TranslateBlendMode(vm, 1 + 1);
			[[maybe_unused]] auto const [v, b] = luastg::translateLegacyBlendState(blend);
			self->data->setLegacyBlendState(v, b);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int draw(lua_State* vm) {
			auto const self = as(vm, 1);
			self->data->draw(LAPP.GetAppModel()->getRenderer());
			return 0;
		}

		// static method

		static int create(lua_State* vm) {
			if (lua::stack_t const ctx(vm); ctx.index_of_top() >= 1) {
				auto const sprite = Sprite::as(vm, 1);
				auto const self = SpriteRectRenderer::create(vm);
				if (!Core::Graphics::ISpriteRenderer::create(&self->data)) {
					return luaL_error(vm, "create SpriteRectRenderer failed");
				}
				self->data->setSprite(sprite->data);
			}
			else {
				auto const self = SpriteRectRenderer::create(vm);
				if (!Core::Graphics::ISpriteRenderer::create(&self->data)) {
					return luaL_error(vm, "create SpriteRectRenderer failed");
				}
			}
			return 1;
		}
	};

	bool SpriteRectRenderer::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	SpriteRectRenderer* SpriteRectRenderer::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<SpriteRectRenderer>(index);
	}
	SpriteRectRenderer* SpriteRectRenderer::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<SpriteRectRenderer>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void SpriteRectRenderer::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "setRect", &SpriteRectRendererBinding::setRect);
		ctx.set_map_value(method_table, "setSprite", &SpriteRectRendererBinding::setSprite);
		ctx.set_map_value(method_table, "setColor", &SpriteRectRendererBinding::setColor);
		ctx.set_map_value(method_table, "setLegacyBlendState", &SpriteRectRendererBinding::setLegacyBlendState);
		ctx.set_map_value(method_table, "draw", &SpriteRectRendererBinding::draw);
		ctx.set_map_value(method_table, "create", &SpriteRectRendererBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &SpriteRectRendererBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &SpriteRectRendererBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &SpriteRectRendererBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}

namespace luastg::binding {
	std::string_view const SpriteQuadRenderer::class_name{ "lstg.SpriteQuadRenderer" };

	struct SpriteQuadRendererBinding : SpriteQuadRenderer {
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

		static int setQuad(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (ctx.index_of_top() >= 1 + 12) {
				auto const x1 = ctx.get_value<float>(1 + 1);
				auto const y1 = ctx.get_value<float>(1 + 2);
				auto const z1 = ctx.get_value<float>(1 + 3);
				auto const x2 = ctx.get_value<float>(1 + 4);
				auto const y2 = ctx.get_value<float>(1 + 5);
				auto const z2 = ctx.get_value<float>(1 + 6);
				auto const x3 = ctx.get_value<float>(1 + 7);
				auto const y3 = ctx.get_value<float>(1 + 8);
				auto const z3 = ctx.get_value<float>(1 + 9);
				auto const x4 = ctx.get_value<float>(1 + 10);
				auto const y4 = ctx.get_value<float>(1 + 11);
				auto const z4 = ctx.get_value<float>(1 + 12);
				self->data->setTransform(
					Core::Vector3F(x1, y1, z1),
					Core::Vector3F(x2, y2, z2),
					Core::Vector3F(x3, y3, z3),
					Core::Vector3F(x4, y4, z4)
				);
			}
			else {
				auto const x1 = ctx.get_value<float>(1 + 1);
				auto const y1 = ctx.get_value<float>(1 + 2);
				auto const x2 = ctx.get_value<float>(1 + 3);
				auto const y2 = ctx.get_value<float>(1 + 4);
				auto const x3 = ctx.get_value<float>(1 + 5);
				auto const y3 = ctx.get_value<float>(1 + 6);
				auto const x4 = ctx.get_value<float>(1 + 7);
				auto const y4 = ctx.get_value<float>(1 + 8);
				self->data->setTransform(
					Core::Vector2F(x1, y1),
					Core::Vector2F(x2, y2),
					Core::Vector2F(x3, y3),
					Core::Vector2F(x4, y4)
				);
			}
			self->data->setZ(0.5); // TODO: allow custom
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
				auto const c1 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				auto const c2 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 2);
				auto const c3 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 3);
				auto const c4 = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 4);
				self->data->setColor(c1, c2, c3, c4);
			}
			else {
				auto const color = *luastg::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				self->data->setColor(color);
			}
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int setLegacyBlendState(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const blend = luastg::TranslateBlendMode(vm, 1 + 1);
			[[maybe_unused]] auto const [v, b] = luastg::translateLegacyBlendState(blend);
			self->data->setLegacyBlendState(v, b);
			ctx.push_value(lua::stack_index_t(1));
			return 1;
		}
		static int draw(lua_State* vm) {
			auto const self = as(vm, 1);
			self->data->draw(LAPP.GetAppModel()->getRenderer());
			return 0;
		}

		// static method

		static int create(lua_State* vm) {
			if (lua::stack_t const ctx(vm); ctx.index_of_top() >= 1) {
				auto const sprite = Sprite::as(vm, 1);
				auto const self = SpriteQuadRenderer::create(vm);
				if (!Core::Graphics::ISpriteRenderer::create(&self->data)) {
					return luaL_error(vm, "create SpriteQuadRenderer failed");
				}
				self->data->setSprite(sprite->data);
			}
			else {
				auto const self = SpriteQuadRenderer::create(vm);
				if (!Core::Graphics::ISpriteRenderer::create(&self->data)) {
					return luaL_error(vm, "create SpriteQuadRenderer failed");
				}
			}
			return 1;
		}
	};

	bool SpriteQuadRenderer::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	SpriteQuadRenderer* SpriteQuadRenderer::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<SpriteQuadRenderer>(index);
	}
	SpriteQuadRenderer* SpriteQuadRenderer::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<SpriteQuadRenderer>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void SpriteQuadRenderer::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "setQuad", &SpriteQuadRendererBinding::setQuad);
		ctx.set_map_value(method_table, "setSprite", &SpriteQuadRendererBinding::setSprite);
		ctx.set_map_value(method_table, "setColor", &SpriteQuadRendererBinding::setColor);
		ctx.set_map_value(method_table, "setLegacyBlendState", &SpriteQuadRendererBinding::setLegacyBlendState);
		ctx.set_map_value(method_table, "draw", &SpriteQuadRendererBinding::draw);
		ctx.set_map_value(method_table, "create", &SpriteQuadRendererBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &SpriteQuadRendererBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &SpriteQuadRendererBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &SpriteQuadRendererBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
