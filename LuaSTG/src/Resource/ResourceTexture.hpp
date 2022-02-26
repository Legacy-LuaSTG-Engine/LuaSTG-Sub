#pragma once
#include "ResourceBase.hpp"
#include "f2dRenderDevice.h"

namespace LuaSTGPlus
{
	// 纹理资源
	class ResTexture :
		public Resource
	{
	private:
		fcyRefPointer<f2dTexture2D> m_Texture;
		fcyRefPointer<f2dDepthStencilSurface> m_DepthStencil;
	public:
		f2dTexture2D* GetTexture() { return *m_Texture; }
		f2dDepthStencilSurface* GetDepthStencilSurface() { return *m_DepthStencil; }
		bool IsRenderTarget() { return m_Texture->IsRenderTarget(); }
		bool HasDepthStencilBuffer() { return m_DepthStencil; }
	public:
		ResTexture(const char* name, fcyRefPointer<f2dTexture2D> tex)
			: Resource(ResourceType::Texture, name), m_Texture(tex) {}
		ResTexture(const char* name, fcyRefPointer<f2dTexture2D> tex, fcyRefPointer<f2dDepthStencilSurface> ds)
			: Resource(ResourceType::Texture, name), m_Texture(tex), m_DepthStencil(ds) {}
	};
};
