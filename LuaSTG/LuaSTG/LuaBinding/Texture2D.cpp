// ReSharper disable CppClangTidyBugproneReservedIdentifier
// ReSharper disable CppClangTidyClangDiagnosticReservedIdentifier

#include "Texture2D.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

namespace LuaSTG::Sub::LuaBinding {
	std::string_view Texture2D::class_name{ "lstg.Window" };

	struct Texture2DBinding : Texture2D {
		// meta methods

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
			} else {
				ctx.push_value(false);
			}
			return 1;
		}

		// method

		static int getWidth(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getSize().x);
			return 1;
		}
		static int getHeight(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getSize().y);
			return 1;
		}

		// static method

		static int createFromFile(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const path = ctx.get_value<std::string_view>(1);
			auto const mipmap_levels = ctx.get_value<uint32_t>(2, 1ui32);
			Core::ScopeObject<Core::Graphics::ITexture2D> texture;
			if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(path, mipmap_levels != 1, ~texture)) {
				auto const error_message = std::format(
					"create Texture2D from file '{}' failed", path);
				return luaL_error(vm, error_message.c_str());
			}
			auto const self = create(vm);
			self->data = texture.detach();
			return 1;
		}
	};

	bool Texture2D::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	Texture2D* Texture2D::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<Texture2D>(index);
	}
	Texture2D* Texture2D::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<Texture2D>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void Texture2D::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.push_module(class_name);
		ctx.set_map_value(method_table, "getWidth", &Texture2DBinding::getWidth);
		ctx.set_map_value(method_table, "getHeight", &Texture2DBinding::getHeight);
		ctx.set_map_value(method_table, "createFromFile", &Texture2DBinding::createFromFile);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &Texture2DBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &Texture2DBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &Texture2DBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
