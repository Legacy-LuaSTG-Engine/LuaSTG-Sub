#include "LuaBinding/LuaInternalSource.hpp"

#pragma region x86 lib
static const std::string _InternalSource_x86 = R"(

package.cpath = ""
package.cpath = package.cpath .. ".\\x86\\?.dll;"
package.cpath = package.cpath .. ".\\?.dll;"

)";
#pragma endregion

#pragma region x64 lib
static const std::string _InternalSource_amd64 = R"(

package.cpath = ""
package.cpath = package.cpath .. ".\\amd64\\?.dll;"
package.cpath = package.cpath .. ".\\?.dll;"

)";
#pragma endregion

#pragma region main
static const std::string _InternalSource_Main = R"(

package.path = ""
package.path = package.path .. "?.lua;"
package.path = package.path .. "?/init.lua;"
package.path = package.path .. "src/?.lua;"

package.path = package.path .. ".\\?.lua;"
package.path = package.path .. ".\\?\\init.lua;"
package.path = package.path .. ".\\src\\?.lua;"

function GameInit()
end
function FrameFunc()
    return false
end
function RenderFunc()
end
function GameExit()
end
function FocusLoseFunc()
end
function FocusGainFunc()
end
function EventFunc(event, ...)
end

)";
#pragma endregion

#pragma region api
static const std::string _InternalSource_API = R"(

local lstg = require("lstg")

function lstg.ShowSplashWindow()
end

function lstg.PostEffectCapture()
end

function lstg.PostEffectApply()
end

function lstg.SystemLog(text)
    lstg.Log(2, text)
end

function lstg.Print(...)
    local args = {...}
    local argc = select('#', ...)
    for i = 1, argc do args[i] = tostring(args[i]) end
    lstg.Log(2, table.concat(args, '\t'))
end

print = lstg.Print

if cjson then
    package.loaded["cjson"] = cjson -- fuck you cjson
end

local rad = math.rad
local deg = math.deg
local sin = math.sin
local cos = math.cos
local tan = math.tan
local asin = math.asin
local acos = math.acos
local atan = math.atan
local atan2 = math.atan2 or math.atan
function lstg.sin(x) return sin(rad(x)) end
function lstg.cos(x) return cos(rad(x)) end
function lstg.tan(x) return tan(rad(x)) end
function lstg.asin(x) return deg(asin(x)) end
function lstg.acos(x) return deg(acos(x)) end
function lstg.atan(...) return deg(atan(...)) end
function lstg.atan2(y, x) return deg(atan2(y, x)) end

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

)";
#pragma endregion

namespace LuaSTGPlus
{
	std::string LuaInternalSource_1() {
		if constexpr (sizeof(void*) >= 8)
		{
			return _InternalSource_amd64 + _InternalSource_Main;
		}
		else
		{
			return _InternalSource_x86 + _InternalSource_Main;
		}
	}
	std::string LuaInternalSource_2() {
		return _InternalSource_API;
	}
}
