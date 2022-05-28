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

	struct ITexture2D : public IObject
	{
		virtual void* getNativeHandle() = 0;

		virtual bool isDynamic() = 0;
		virtual bool isPremultipliedAlpha() = 0;

		virtual void setPremultipliedAlpha(bool v) = 0;

		virtual Vector2U getSize() = 0;

		virtual bool uploadPixelData(RectU rc, void const* data, uint32_t pitch) = 0;
	};

	struct IRenderTarget : public IObject
	{
		virtual void* getNativeHandle() = 0;

		virtual ITexture2D* getTexture() = 0;
	};

	struct IDepthStencilBuffer : public IObject
	{
		virtual void* getNativeHandle() = 0;

		virtual Vector2U getSize() = 0;
	};

	struct IDevice : public IObject
	{
		virtual void addEventListener(IDeviceEventListener* e) = 0;
		virtual void removeEventListener(IDeviceEventListener* e) = 0;

		virtual DeviceMemoryUsageStatistics getMemoryUsageStatistics() = 0;

		virtual bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** pp_texutre) = 0;
		//virtual bool createTextureFromMemory(void const* data, size_t size, bool mipmap, ITexture2D** pp_texutre) = 0;
		virtual bool createTexture(Vector2U size, ITexture2D** pp_texutre) = 0;

		virtual bool createRenderTarget(Vector2U size, IRenderTarget** pp_rt) = 0;
		virtual bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** pp_ds) = 0;

		static bool create(StringView prefered_gpu, IDevice** p_device);
	};
}
