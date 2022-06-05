#pragma once
#include "ResourceBase.hpp"
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/SwapChain.hpp"

namespace LuaSTGPlus
{
	// 纹理资源
	class ResTexture
		: public Resource
		, public LuaSTG::Core::Graphics::ISwapChainEventListener
	{
	private:
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ISwapChain> m_swapchain; // 用于自动调整渲染目标大小的
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ITexture2D> m_texture;
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IRenderTarget> m_rt;
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IDepthStencilBuffer> m_ds;
		bool m_is_rendertarget{ false };
		bool m_is_auto_resize{ false };
		bool m_enable_depthbuffer{ false };
	public:
		bool createResources();
		void onSwapChainCreate();
		void onSwapChainDestroy();
	public:
		LuaSTG::Core::Graphics::ITexture2D* GetTexture() { return m_texture.get(); }
		LuaSTG::Core::Graphics::IRenderTarget* GetRenderTarget() { return m_rt.get(); }
		LuaSTG::Core::Graphics::IDepthStencilBuffer* GetDepthStencilBuffer() { return m_ds.get(); }
		bool IsRenderTarget() { return m_is_rendertarget; }
		bool HasDepthStencilBuffer() { return m_enable_depthbuffer; }
	public:
		// 纹理容器
		ResTexture(const char* name, LuaSTG::Core::Graphics::ITexture2D* p_texture);
		// 渲染附件容器
		ResTexture(const char* name, int w, int h, bool ds);
		// 自动调整大小的渲染附件容器
		ResTexture(const char* name, bool ds);
		~ResTexture();
	};
};
