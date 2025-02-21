#include "Mesh.hpp"
#include "lua/plus.hpp"
#include "LuaWrapper.hpp"
#include "AppFrame.h"

using std::string_view_literals::operator ""sv;

namespace LuaSTG::Sub::LuaBinding {
	std::string_view Mesh::class_name{ "lstg.Mesh" };

	struct MeshBinding : Mesh {
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

		static int getVertexCount(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getVertexCount());
			return 1;
		}
		static int getIndexCount(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getIndexCount());
			return 1;
		}
		static int getPrimitiveTopology(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(static_cast<int32_t>(self->data->getPrimitiveTopology()));
			return 1;
		}
		static int isReadOnly(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->isReadOnly());
			return 1;
		}

		static int setVertex(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const vertex_index = ctx.get_value<uint32_t>(1 + 1);
			auto const x = ctx.get_value<float>(1 + 2);
			auto const y = ctx.get_value<float>(1 + 3);

			if (ctx.index_of_top() >= 1 + 10) {
				auto const z = ctx.get_value<float>(1 + 4);
				auto const u = ctx.get_value<float>(1 + 5);
				auto const v = ctx.get_value<float>(1 + 6);
				auto const r = ctx.get_value<float>(1 + 7);
				auto const g = ctx.get_value<float>(1 + 8);
				auto const b = ctx.get_value<float>(1 + 9);
				auto const a = ctx.get_value<float>(1 + 10);
				self->data->setVertex(vertex_index, Core::Vector3F(x, y, z), Core::Vector2F(u, v), Core::Vector4F(r, g, b, a));
			}
			else if (ctx.index_of_top() >= 1 + 9) {
				auto const u = ctx.get_value<float>(1 + 4);
				auto const v = ctx.get_value<float>(1 + 5);
				auto const r = ctx.get_value<float>(1 + 6);
				auto const g = ctx.get_value<float>(1 + 7);
				auto const b = ctx.get_value<float>(1 + 8);
				auto const a = ctx.get_value<float>(1 + 9);
				self->data->setVertex(vertex_index, Core::Vector2F(x, y), Core::Vector2F(u, v), Core::Vector4F(r, g, b, a));
			}
			else if (ctx.index_of_top() >= 1 + 7) {
				auto const z = ctx.get_value<float>(1 + 4);
				auto const u = ctx.get_value<float>(1 + 5);
				auto const v = ctx.get_value<float>(1 + 6);
				auto const color = LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 7);
				self->data->setVertex(vertex_index, Core::Vector3F(x, y, z), Core::Vector2F(u, v), *color);
			}
			else /* if (ctx.index_of_top() == 1 + 6) */ {
				auto const u = ctx.get_value<float>(1 + 4);
				auto const v = ctx.get_value<float>(1 + 5);
				auto const color = LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 6);
				self->data->setVertex(vertex_index, Core::Vector2F(x, y), Core::Vector2F(u, v), *color);
			}

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setPosition(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const vertex_index = ctx.get_value<uint32_t>(1 + 1);
			auto const x = ctx.get_value<float>(1 + 2);
			auto const y = ctx.get_value<float>(1 + 3);
			if (ctx.index_of_top() >= 1 + 4) {
				auto const z = ctx.get_value<float>(1 + 4);
				self->data->setPosition(vertex_index, Core::Vector3F(x, y, z));
			}
			else {
				self->data->setPosition(vertex_index, Core::Vector2F(x, y));
			}

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setUv(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const vertex_index = ctx.get_value<uint32_t>(1 + 1);
			auto const u = ctx.get_value<float>(1 + 2);
			auto const v = ctx.get_value<float>(1 + 3);
			self->data->setUv(vertex_index, Core::Vector2F(u, v));

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}
		static int setColor(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const vertex_index = ctx.get_value<uint32_t>(1 + 1);
			if (ctx.index_of_top() >= 1 + 5) {
				auto const r = ctx.get_value<float>(1 + 2);
				auto const g = ctx.get_value<float>(1 + 3);
				auto const b = ctx.get_value<float>(1 + 4);
				auto const a = ctx.get_value<float>(1 + 5);
				self->data->setColor(vertex_index, Core::Vector4F(r, g, b, a));
			}
			else {
				auto const color = LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 1);
				self->data->setColor(vertex_index, *color);
			}

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}

		static int setIndex(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const index_index = ctx.get_value<uint32_t>(1 + 1);
			auto const vertex_index = ctx.get_value<uint32_t>(2 + 1);
			self->data->setIndex(index_index, vertex_index);

			ctx.push_value(lua::stack_index_t(1)); // return self
			return 1;
		}

		static int commit(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);

			auto const result = self->data->commit();

			ctx.push_value(result);
			return 1;
		}
		static int setReadOnly(lua_State* vm) {
			auto const self = as(vm, 1);
			self->data->setReadOnly();
			return 0;
		}

		// static method

		static int create(lua_State* vm) {
			lua::stack_t const ctx(vm);

			constexpr lua::stack_index_t options_table(1);
			Core::Graphics::MeshOptions options;

			options.vertex_count = ctx.get_map_value<uint32_t>(options_table, "vertex_count");
			options.index_count = ctx.get_map_value<uint32_t>(options_table, "index_count", 0);
			options.vertex_index_compression = ctx.get_map_value<bool>(options_table, "vertex_index_compression", true);
			options.vertex_color_compression = ctx.get_map_value<bool>(options_table, "vertex_color_compression", true);

			auto const device = LAPP.GetAppModel()->getDevice();
			auto const self = Mesh::create(vm);
			if (!Core::Graphics::IMesh::create(device, options, &self->data)) {
				return luaL_error(vm, "create Mesh failed.");
			}

			return 1;
		}
	};

	bool Mesh::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	Mesh* Mesh::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<Mesh>(index);
	}
	Mesh* Mesh::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<Mesh>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void Mesh::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.push_module(class_name);
		ctx.set_map_value(method_table, "getVertexCount", &MeshBinding::getVertexCount);
		ctx.set_map_value(method_table, "getIndexCount", &MeshBinding::getIndexCount);
		ctx.set_map_value(method_table, "getPrimitiveTopology", &MeshBinding::getPrimitiveTopology);
		ctx.set_map_value(method_table, "isReadOnly", &MeshBinding::isReadOnly);
		ctx.set_map_value(method_table, "setVertex", &MeshBinding::setVertex);
		ctx.set_map_value(method_table, "setPosition", &MeshBinding::setPosition);
		ctx.set_map_value(method_table, "setUv", &MeshBinding::setPosition);
		ctx.set_map_value(method_table, "setColor", &MeshBinding::setColor);
		ctx.set_map_value(method_table, "setIndex", &MeshBinding::setIndex);
		ctx.set_map_value(method_table, "commit", &MeshBinding::commit);
		ctx.set_map_value(method_table, "setReadOnly", &MeshBinding::setReadOnly);
		ctx.set_map_value(method_table, "create", &MeshBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &MeshBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &MeshBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &MeshBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
