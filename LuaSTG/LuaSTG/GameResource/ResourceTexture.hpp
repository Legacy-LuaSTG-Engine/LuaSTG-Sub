#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/SwapChain.hpp"

namespace LuaSTGPlus
{
	// 纹理资源
	class ResTexture
		: public Resource
	{
	private:
		Core::ScopeObject<Core::Graphics::ITexture2D> m_texture;
		Core::ScopeObject<Core::Graphics::IRenderTarget> m_rt;
		Core::ScopeObject<Core::Graphics::IDepthStencilBuffer> m_ds;
		bool m_is_rendertarget{ false };
		bool m_is_auto_resize{ false };
		bool m_enable_depthbuffer{ false };
	public:
		bool ResizeRenderTarget(Core::Vector2U size);
	public:
		Core::Graphics::ITexture2D* GetTexture() { return m_texture.get(); }
		Core::Graphics::IRenderTarget* GetRenderTarget() { return m_rt.get(); }
		Core::Graphics::IDepthStencilBuffer* GetDepthStencilBuffer() { return m_ds.get(); }
		bool IsRenderTarget() { return m_is_rendertarget; }
		bool HasDepthStencilBuffer() { return m_enable_depthbuffer; }
	public:
		// 纹理容器
		ResTexture(const char* name, Core::Graphics::ITexture2D* p_texture);
		// 渲染附件容器
		ResTexture(const char* name, int w, int h, bool ds);
		// 自动调整大小的渲染附件容器
		ResTexture(const char* name, bool ds);
		~ResTexture();
	};
};
