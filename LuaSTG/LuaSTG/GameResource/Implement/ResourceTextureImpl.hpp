#pragma once
#include "core/SmartReference.hpp"
#include "GameResource/ResourceTexture.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceTextureImpl : public ResourceBaseImpl<IResourceTexture>
	{
	private:
		core::SmartReference<core::Graphics::ITexture2D> m_texture;
		core::SmartReference<core::Graphics::IRenderTarget> m_rt;
		core::SmartReference<core::Graphics::IDepthStencilBuffer> m_ds;
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

	class RenderTargetStackResourceTextureImpl : public core::implement::ReferenceCounted<IResourceTexture> {
	public:
		// IResourceBase

		ResourceType GetType() const noexcept override { return ResourceType::Texture; }
		std::string_view GetResName() const noexcept override { return "auto"; }

		// IResourceTexture

		bool ResizeRenderTarget(core::Vector2U) override { return false; }

		core::Graphics::ITexture2D* GetTexture() override { return m_rt->getTexture(); }
		core::Graphics::IRenderTarget* GetRenderTarget() override { return m_rt.get(); }
		core::Graphics::IDepthStencilBuffer* GetDepthStencilBuffer() override { return m_ds.get(); }
		bool IsRenderTarget() override { return true; }
		bool HasDepthStencilBuffer() override { return !!m_ds; }

		// RenderTargetStackResourceTextureImpl

		RenderTargetStackResourceTextureImpl(core::Graphics::IRenderTarget* rt, core::Graphics::IDepthStencilBuffer* ds);
		RenderTargetStackResourceTextureImpl(RenderTargetStackResourceTextureImpl const&) = delete;
		RenderTargetStackResourceTextureImpl(RenderTargetStackResourceTextureImpl&&) = delete;
		~RenderTargetStackResourceTextureImpl();

		RenderTargetStackResourceTextureImpl& operator=(RenderTargetStackResourceTextureImpl const&) = delete;
		RenderTargetStackResourceTextureImpl& operator=(RenderTargetStackResourceTextureImpl&&) = delete;
	private:
		core::SmartReference<core::Graphics::IRenderTarget> m_rt;
		core::SmartReference<core::Graphics::IDepthStencilBuffer> m_ds;
	};
}
