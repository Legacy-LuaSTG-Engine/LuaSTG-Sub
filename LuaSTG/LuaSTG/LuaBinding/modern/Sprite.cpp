#include "Sprite.hpp"
#include "Texture2D.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

namespace luastg::binding {
	std::string_view const Sprite::class_name{ "lstg.Sprite" };

	struct SpriteBinding : Sprite {
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

		static int setTexture(lua_State* vm) {
			auto const self = as(vm, 1);
			auto const texture = Texture2D::as(vm, 1 + 1);
			self->data->setTexture(texture->data);
			return 0;
		}
		static int getTexture(lua_State* vm) {
			auto const self = as(vm, 1);
			auto const texture = Texture2D::create(vm);
			texture->data = self->data->getTexture();
			if (texture->data) {
				texture->data->retain();
			}
			return 1;
		}
		static int setTextureRect(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const x = ctx.get_value<float>(1 + 1);
			auto const y = ctx.get_value<float>(1 + 2);
			auto const width = ctx.get_value<float>(1 + 3);
			auto const height = ctx.get_value<float>(1 + 4);
			self->data->setTextureRect(core::RectF(x, y, x + width, x + height));
			return 0;
		}
		static int getTextureRect(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const rect = self->data->getTextureRect();
			ctx.push_value(rect.a.x);
			ctx.push_value(rect.a.y);
			ctx.push_value(rect.b.x - rect.a.x);
			ctx.push_value(rect.b.y - rect.a.y);
			return 4;
		}
		static int setCenter(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const x = ctx.get_value<float>(1 + 1);
			auto const y = ctx.get_value<float>(1 + 2);
			self->data->setTextureCenter(core::Vector2F(x, y));
			return 0;
		}
		static int getCenter(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const center = self->data->getTextureCenter();
			ctx.push_value(center.x);
			ctx.push_value(center.y);
			return 2;
		}
		static int setUnitPerPixel(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const unit_per_pixel = ctx.get_value<float>(1 + 1);
			self->data->setUnitsPerPixel(unit_per_pixel);
			return 0;
		}
		static int getUnitPerPixel(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getUnitsPerPixel());
			return 0;
		}

		// static method

		static int create(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const texture = Texture2D::as(vm, 1);
			auto const x = ctx.get_value<float>(2);
			auto const y = ctx.get_value<float>(3);
			auto const width = ctx.get_value<float>(4);
			auto const height = ctx.get_value<float>(5);
			if (ctx.index_of_top() >= 7) {
				auto const center_x = ctx.get_value<float>(6);
				auto const center_y = ctx.get_value<float>(7);
				auto const unit_per_pixel = ctx.get_value<float>(8, 1.0f);
				auto const self = Sprite::create(vm);
				if (!core::Graphics::ISprite::create(LAPP.GetAppModel()->getRenderer(), texture->data, &self->data)) {
					return luaL_error(vm, "create Sprite failed");
				}
				self->data->setTexture(texture->data);
				self->data->setTextureRect(core::RectF(x, y, x + width, x + height));
				self->data->setTextureCenter(core::Vector2F(center_x, center_y));
				self->data->setUnitsPerPixel(unit_per_pixel);
			}
			else {
				auto const unit_per_pixel = ctx.get_value<float>(6, 1.0f);
				auto const self = Sprite::create(vm);
				if (!core::Graphics::ISprite::create(LAPP.GetAppModel()->getRenderer(), texture->data, &self->data)) {
					return luaL_error(vm, "create Sprite failed");
				}
				self->data->setTexture(texture->data);
				self->data->setTextureRect(core::RectF(x, y, x + width, x + height));
				self->data->setUnitsPerPixel(unit_per_pixel);
			}
			return 1;
		}
	};

	bool Sprite::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	Sprite* Sprite::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<Sprite>(index);
	}
	Sprite* Sprite::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<Sprite>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void Sprite::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "setTexture", &SpriteBinding::setTexture);
		ctx.set_map_value(method_table, "getTexture", &SpriteBinding::getTexture);
		ctx.set_map_value(method_table, "setTextureRect", &SpriteBinding::setTextureRect);
		ctx.set_map_value(method_table, "getTextureRect", &SpriteBinding::getTextureRect);
		ctx.set_map_value(method_table, "setCenter", &SpriteBinding::setCenter);
		ctx.set_map_value(method_table, "getCenter", &SpriteBinding::getCenter);
		ctx.set_map_value(method_table, "setUnitPerPixel", &SpriteBinding::setUnitPerPixel);
		ctx.set_map_value(method_table, "getUnitPerPixel", &SpriteBinding::getUnitPerPixel);
		ctx.set_map_value(method_table, "create", &SpriteBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &SpriteBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &SpriteBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &SpriteBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
