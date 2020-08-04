#include "LuaInternalSource.hpp"

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
)";

namespace LuaSTGPlus {
	const std::string& LuaInternalSource_1() {
		return _InternalSource_1;
	}
}
