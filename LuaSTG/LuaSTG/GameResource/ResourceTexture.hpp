#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Device.hpp"

namespace luastg
{
	struct IResourceTexture : public IResourceBase
	{
		virtual bool ResizeRenderTarget(core::Vector2U size) = 0;

		virtual core::Graphics::ITexture2D* GetTexture() = 0;
		virtual core::Graphics::IRenderTarget* GetRenderTarget() = 0;
		virtual core::Graphics::IDepthStencilBuffer* GetDepthStencilBuffer() = 0;
		virtual bool IsRenderTarget() = 0;
		virtual bool HasDepthStencilBuffer() = 0;
	};
};
