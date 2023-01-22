#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Device.hpp"

namespace LuaSTGPlus
{
	struct IResourceTexture : public IResourceBase
	{
		virtual bool ResizeRenderTarget(Core::Vector2U size) = 0;

		virtual Core::Graphics::ITexture2D* GetTexture() = 0;
		virtual Core::Graphics::IRenderTarget* GetRenderTarget() = 0;
		virtual Core::Graphics::IDepthStencilBuffer* GetDepthStencilBuffer() = 0;
		virtual bool IsRenderTarget() = 0;
		virtual bool HasDepthStencilBuffer() = 0;
	};
};
