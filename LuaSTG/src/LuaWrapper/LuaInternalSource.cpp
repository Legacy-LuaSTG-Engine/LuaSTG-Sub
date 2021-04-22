#include "LuaWrapper/LuaInternalSource.hpp"

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
package.path = package.path .. "src/?.lua;"
package.path = package.path .. ".\\?.lua;"
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

local function _cjson()
	cjson = require("cjson")
end
local function _lfs()
	lfs = require("lfs")
end
pcall(_cjson)
pcall(_lfs)

)";
#pragma endregion

#pragma region api
static const std::string _InternalSource_API = R"(

local lstg = require("lstg")

function lstg.ShowSplashWindow()
end

local _render_target = "__RENDER_TARGET__"

local _lstg_CheckRes = lstg.CheckRes
local function _check_render_target()
    if not _lstg_CheckRes(1, _render_target) then
        lstg.CreateRenderTarget(_render_target)
    end
end

function lstg.PostEffectCapture()
    _check_render_target()
    lstg.PushRenderTarget(_render_target)
end

function lstg.PostEffectApply(fxname, blend, args)
    _check_render_target()
    lstg.PopRenderTarget()
    lstg.PostEffect(_render_target, fxname, blend, args)
end

function lstg.GetLastChar()
    return ""
end

)";
#pragma endregion

namespace LuaSTGPlus {
	std::string LuaInternalSource_1() {
		if (sizeof(void*) >= 8)
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
