#include "LuaWrapper/LuaInternalSource.hpp"

static const std::string _InternalSource_1 = R"(
	
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

namespace LuaSTGPlus {
	const std::string& LuaInternalSource_1() {
		return _InternalSource_1;
	}
}
