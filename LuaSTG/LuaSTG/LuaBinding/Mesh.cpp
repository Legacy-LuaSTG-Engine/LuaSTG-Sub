#include "Mesh.hpp"
#include "lua/plus.hpp"
#include "LuaWrapper.hpp"
#include "AppFrame.h"

using std::string_view_literals::operator ""sv;

namespace {
	constexpr auto embedded_script = R"(-- LuaSTG Sub built-in script
local Mesh = require("lstg.Mesh")
function Mesh:createVertexWriter()
	local assert = assert
	local select = select
	local mesh = self
	local vertex_count = mesh:getVertexCount()
	local M = {}
	local p = -1
	function M:seek(vertex_index)
		assert(vertex_index >= 0 and vertex_index < vertex_count, "vertex out of bounds")
		p = vertex_index - 1
		return M
	end
	function M:vertex(...)
		p = p + 1
		assert(p < vertex_count, "vertex out of bounds")
		if select("#", ...) > 0 then
			mesh:setVertex(p, ...)
		end
		return M
	end
	function M:position(...)
		assert(p >= 0, "forget to call the 'vertex' method?")
		mesh:setPosition(p, ...)
		return M
	end
	function M:uv(...)
		assert(p >= 0, "forget to call the 'vertex' method?")
		mesh:setUv(p, ...)
		return M
	end
	function M:color(...)
		assert(p >= 0, "forget to call the 'vertex' method?")
		mesh:setColor(p, ...)
		return M
	end
	function M:commit()
		mesh:commit()
		return M
	end
	return M
end
function Mesh:createIndexWriter()
	local assert = assert
	local ipairs = ipairs
	local mesh = self
	local index_count = mesh:getIndexCount()
	local M = {}
	local p = -1
	function M:seek(index_index)
		assert(index_index >= 0 and index_index < index_count, "index out of bounds")
		p = index_index - 1
		return M
	end
	function M:index(...)
		local args = {...}
		assert(p + #args < index_count, "index out of bounds")
		for _, i in ipairs(args) do
			p = p + 1
			mesh:setIndex(p, i)
		end
		return M
	end
	function M:commit()
		mesh:commit()
		return M
	end
	return M
end
function Mesh:createPrimitiveWriter()
	local assert = assert
	local select = select
	local mesh = self
	local vertex_count = mesh:getVertexCount()
	local index_count = mesh:getIndexCount()
	local M = {}
	local vertex_index = -1
	local index_index = -1
	local req_vertex_n = 0
	local cur_vertex_n = 0
	function M:vertex(...)
		vertex_index = vertex_index + 1
		cur_vertex_n = cur_vertex_n + 1
		assert(vertex_index < vertex_count, "vertex out of bounds")
		assert(req_vertex_n == 0 or cur_vertex_n <= req_vertex_n, "vertex out of primitive scope")
		if select("#", ...) > 0 then
			mesh:setVertex(vertex_index, ...)
		end
		return M
	end
	function M:position(...)
		assert(vertex_index >= 0, "forget to call the 'vertex' method?")
		mesh:setPosition(vertex_index, ...)
		return M
	end
	function M:uv(...)
		assert(vertex_index >= 0, "forget to call the 'vertex' method?")
		mesh:setUv(vertex_index, ...)
		return M
	end
	function M:color(...)
		assert(vertex_index >= 0, "forget to call the 'vertex' method?")
		mesh:setColor(vertex_index, ...)
		return M
	end
	function M:begin()
		assert(req_vertex_n == 0 or cur_vertex_n == req_vertex_n, "previous primitive is incomplete")
		req_vertex_n = 0
		cur_vertex_n = 0
		return M
	end
	function M:index(...)
		local args = {...}
		assert(index_index + #args < index_count, "index out of bounds")
		local vi = vertex_index + 1
		local ii = index_index + 1
		for i, v in ipairs(args) do
			mesh:setIndex(ii + i - 1, vi + v)
		end
		index_index = index_index + #args
		return M
	end
	function M:triangle(a, b, c)
		return M:index(a, b, c)
	end
	function M:quad(a, b, c, d)
		return M:index(a, b, c, a, c, d)
	end
	function M:beginTriangle()
		assert(req_vertex_n == 0 or cur_vertex_n == req_vertex_n, "previous primitive is incomplete")
		local vi = vertex_index + 1
		local ii = index_index + 1
		mesh:setIndex(ii, vi)
		mesh:setIndex(ii + 1, vi + 1)
		mesh:setIndex(ii + 2, vi + 2)
		index_index = index_index + 3
		req_vertex_n = 3
		cur_vertex_n = 0
		return M
	end
	function M:beginQuad()
		assert(req_vertex_n == 0 or cur_vertex_n == req_vertex_n, "previous primitive is incomplete")
		local vi = vertex_index + 1
		local ii = index_index + 1
		mesh:setIndex(ii, vi)
		mesh:setIndex(ii + 1, vi + 1)
		mesh:setIndex(ii + 2, vi + 2)
		mesh:setIndex(ii + 3, vi)
		mesh:setIndex(ii + 4, vi + 2)
		mesh:setIndex(ii + 5, vi + 3)
		index_index = index_index + 6
		req_vertex_n = 4
		cur_vertex_n = 0
		return M
	end
	function M:execute(callback)
		callback(M)
		return M
	end
	function M:commit()
		mesh:commit()
		return M
	end
	return M
end
)"sv;
}

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
				auto const color = LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(vm, 1 + 2);
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

			ctx.push_map_value(options_table, "index_count");
			if (auto const top_index = ctx.index_of_top();  ctx.is_number(top_index)) {
				options.index_count = ctx.get_value<uint32_t>(top_index);
			}
			ctx.pop_value();

			ctx.push_map_value(options_table, "vertex_position_no_z");
			if (auto const top_index = ctx.index_of_top(); ctx.is_boolean(top_index)) {
				options.vertex_position_no_z = ctx.get_value<bool>(top_index);
			}
			ctx.pop_value();

			ctx.push_map_value(options_table, "vertex_index_compression");
			if (auto const top_index = ctx.index_of_top(); ctx.is_boolean(top_index)) {
				options.vertex_index_compression = ctx.get_value<bool>(top_index);
			}
			ctx.pop_value();

			ctx.push_map_value(options_table, "vertex_color_compression");
			if (auto const top_index = ctx.index_of_top(); ctx.is_boolean(top_index)) {
				options.vertex_color_compression = ctx.get_value<bool>(top_index);
			}
			ctx.pop_value();

			ctx.push_map_value(options_table, "primitive_topology");
			if (auto const top_index = ctx.index_of_top(); ctx.is_number(top_index)) {
				options.primitive_topology = static_cast<Core::Graphics::PrimitiveTopology>(ctx.get_value<int32_t>(top_index));
			}
			ctx.pop_value();

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

		// lstg.Window.FrameStyle

		{
			auto const e = ctx.push_module("lstg.PrimitiveTopology");
			ctx.set_map_value(e, "triangle_list", static_cast<int32_t>(Core::Graphics::PrimitiveTopology::triangle_list));
			ctx.set_map_value(e, "triangle_strip", static_cast<int32_t>(Core::Graphics::PrimitiveTopology::triangle_strip));
		}

		// method

		auto const method_table = ctx.push_module(class_name);
		ctx.set_map_value(method_table, "getVertexCount", &MeshBinding::getVertexCount);
		ctx.set_map_value(method_table, "getIndexCount", &MeshBinding::getIndexCount);
		ctx.set_map_value(method_table, "getPrimitiveTopology", &MeshBinding::getPrimitiveTopology);
		ctx.set_map_value(method_table, "isReadOnly", &MeshBinding::isReadOnly);
		ctx.set_map_value(method_table, "setVertex", &MeshBinding::setVertex);
		ctx.set_map_value(method_table, "setPosition", &MeshBinding::setPosition);
		ctx.set_map_value(method_table, "setUv", &MeshBinding::setUv);
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

		// embedded script

		if (LUA_OK == luaL_loadbuffer(vm, embedded_script.data(), embedded_script.size(), "lstg/Mesh.lua")) {
			lua_call(vm, 0, 0);
		}
	}
}
