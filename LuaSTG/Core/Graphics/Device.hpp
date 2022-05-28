#pragma once
#include "Core/Type.hpp"

namespace LuaSTG::Core::Graphics
{
	struct IDeviceEventListener
	{
		virtual void onDeviceCreate() = 0;
		virtual void onDeviceDestroy() = 0;
	};

	struct DeviceMemoryUsageStatistics
	{
		struct DeviceMemoryUsage
		{
			uint64_t budget;
			uint64_t current_usage;
			uint64_t available_for_reservation;
			uint64_t current_reservation;
		};
		DeviceMemoryUsage local{};
		DeviceMemoryUsage non_local{};
	};

	struct IDevice : public IObject
	{
		virtual void addEventListener(IDeviceEventListener* e) = 0;
		virtual void removeEventListener(IDeviceEventListener* e) = 0;

		virtual DeviceMemoryUsageStatistics getMemoryUsageStatistics() = 0;

		static bool create(StringView prefered_gpu, IDevice** p_device);
	};
}
