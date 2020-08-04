#pragma once
#include "ResourceBase.hpp"

namespace LuaSTGPlus {
	// 纹理资源
	class ResTexture :
		public Resource
	{
	private:
		fcyRefPointer<f2dTexture2D> m_Texture;
	public:
		f2dTexture2D* GetTexture() { return m_Texture; }
		bool IsRenderTarget() { return m_Texture->IsRenderTarget(); }
	public:
		ResTexture(const char* name, fcyRefPointer<f2dTexture2D> tex)
			: Resource(ResourceType::Texture, name), m_Texture(tex) {}
	};
}
