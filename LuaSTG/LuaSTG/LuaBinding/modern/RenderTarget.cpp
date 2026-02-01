#include "Texture2D.hpp"
#include "RenderTarget.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

namespace luastg::binding {
	std::string_view RenderTarget::class_name{ "lstg.RenderTarget" };

	struct RenderTargetBinding : RenderTarget {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __gc(lua_State* const vm) {
			if (auto const self = as(vm, 1); self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}
		static int __tostring(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}
		static int __eq(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(self->data == other->data);
			} else {
				ctx.push_value(false);
			}
			return 1;
		}

		// NOLINTEND(*-reserved-identifier)

		// method

		static int getWidth(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getTexture()->getSize().x);
			return 1;
		}
		static int getHeight(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getTexture()->getSize().y);
			return 1;
		}
		static int getTexture(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (self->data->getTexture() == nullptr) {
				ctx.push_value(std::nullopt);
				return 1;
			}
			auto const texture = Texture2D::create(vm);
			texture->data = self->data->getTexture();
			texture->data->retain();
			return 1;
		}

		// static method

		static int create(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const width = ctx.get_value<uint32_t>(1);
			auto const height = ctx.get_value<uint32_t>(2);
			core::SmartReference<core::IRenderTarget> render_target;
			if (!LAPP.getGraphicsDevice()->createRenderTarget(core::Vector2U(width, height), render_target.put())) {
				auto const error_message = std::format(
					"create RenderTarget ({}x{}) failed", width, height);
				return luaL_error(vm, error_message.c_str());
			}
			auto const self = RenderTarget::create(vm);
			self->data = render_target.detach();
			return 1;
		}
	};

	bool RenderTarget::is(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	RenderTarget* RenderTarget::as(lua_State* const vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<RenderTarget>(index);
	}
	RenderTarget* RenderTarget::create(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<RenderTarget>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void RenderTarget::registerClass(lua_State* const vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "getWidth", &RenderTargetBinding::getWidth);
		ctx.set_map_value(method_table, "getHeight", &RenderTargetBinding::getHeight);
		ctx.set_map_value(method_table, "getTexture", &RenderTargetBinding::getTexture);
		ctx.set_map_value(method_table, "create", &RenderTargetBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &RenderTargetBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &RenderTargetBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &RenderTargetBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
