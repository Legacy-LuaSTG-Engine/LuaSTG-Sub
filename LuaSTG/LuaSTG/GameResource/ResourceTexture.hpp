#pragma once
#include "GameResource/ResourceBase.hpp"
#include "core/GraphicsDevice.hpp"

namespace luastg
{
	struct IResourceTexture : public IResourceBase
	{
		virtual bool ResizeRenderTarget(core::Vector2U size) = 0;

		virtual core::ITexture2D* GetTexture() = 0;
		virtual core::Graphics::IRenderTarget* GetRenderTarget() = 0;
		virtual core::IDepthStencilBuffer* GetDepthStencilBuffer() = 0;
		virtual bool IsRenderTarget() = 0;
		virtual bool HasDepthStencilBuffer() = 0;
	};
};

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceTexture
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceTexture>() { return UUID::parse("33f4401f-8c9e-59b0-b9e5-98e4bd60b662"); }
}
