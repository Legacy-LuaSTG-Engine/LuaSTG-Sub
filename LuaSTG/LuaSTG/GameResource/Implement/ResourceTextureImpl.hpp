#pragma once
#include "GameResource/ResourceTexture.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceTextureImpl : public ResourceBaseImpl<IResourceTexture>
	{
	private:
		core::ScopeObject<core::Graphics::ITexture2D> m_texture;
		core::ScopeObject<core::Graphics::IRenderTarget> m_rt;
		core::ScopeObject<core::Graphics::IDepthStencilBuffer> m_ds;
		bool m_is_rendertarget{ false };
		bool m_is_auto_resize{ false };
		bool m_enable_depthbuffer{ false };
	public:
		bool ResizeRenderTarget(core::Vector2U size);
	public:
		core::Graphics::ITexture2D* GetTexture() { return m_texture.get(); }
		core::Graphics::IRenderTarget* GetRenderTarget() { return m_rt.get(); }
		core::Graphics::IDepthStencilBuffer* GetDepthStencilBuffer() { return m_ds.get(); }
		bool IsRenderTarget() { return m_is_rendertarget; }
		bool HasDepthStencilBuffer() { return m_enable_depthbuffer; }
	public:
		// 纹理容器
		ResourceTextureImpl(const char* name, core::Graphics::ITexture2D* p_texture);
		// 渲染附件容器
		ResourceTextureImpl(const char* name, int w, int h, bool ds);
		// 自动调整大小的渲染附件容器
		ResourceTextureImpl(const char* name, bool ds);
		~ResourceTextureImpl();
	};
}
