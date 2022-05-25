#pragma once
#include "Core/Type.hpp"

namespace LuaSTG::Core::Graphics
{
	struct IDevice : public IObject
	{
	public:
		static bool create(StringView prefered_gpu, IDevice** p_device);
	};
}
