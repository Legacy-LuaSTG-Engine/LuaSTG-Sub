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

if cjson then
    package.loaded["cjson"] = cjson -- fuck you cjson
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
