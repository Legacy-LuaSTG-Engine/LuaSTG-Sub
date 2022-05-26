#pragma once
#include "Core/Type.hpp"

namespace LuaSTG::Core::Graphics
{
	struct IDeviceEventListener
	{
		virtual void onDeviceCreate() = 0;
		virtual void onDeviceDestroy() = 0;
	};

	struct IDevice : public IObject
	{
		virtual void addEventListener(IDeviceEventListener* e) = 0;
		virtual void removeEventListener(IDeviceEventListener* e) = 0;

		static bool create(StringView prefered_gpu, IDevice** p_device);
	};
}
