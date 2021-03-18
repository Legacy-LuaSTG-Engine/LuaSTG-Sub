#include "LuaWrapper/LuaInternalSource.hpp"

static const std::string _InternalSource_x86 = R"(

package.cpath = ""
package.cpath = package.cpath .. ".\\x86\\?.dll;"
package.cpath = package.cpath .. ".\\?.dll;"

)";

static const std::string _InternalSource_amd64 = R"(

package.cpath = ""
package.cpath = package.cpath .. ".\\amd64\\?.dll;"
package.cpath = package.cpath .. ".\\?.dll;"

)";

static const std::string _InternalSource_1 = R"(

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

namespace LuaSTGPlus {
	std::string LuaInternalSource_1() {
		if (sizeof(void*) >= 8)
		{
			return _InternalSource_amd64 + _InternalSource_1;
		}
		else
		{
			return _InternalSource_x86 + _InternalSource_1;
		}
	}
}
