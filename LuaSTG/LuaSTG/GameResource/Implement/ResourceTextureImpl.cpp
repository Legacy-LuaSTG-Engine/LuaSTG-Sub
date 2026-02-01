#include "GameResource/Implement/ResourceTextureImpl.hpp"
#include "AppFrame.h"

namespace luastg
{
	bool ResourceTextureImpl::ResizeRenderTarget(core::Vector2U size)
	{
		if (!m_rt)
		{
			if (!LAPP.getGraphicsDevice()->createRenderTarget(size, m_rt.put()))
			{
				return false;
			}
			m_texture = m_rt->getTexture();
		}
		else
		{
			if (!m_rt->setSize(size))
			{
				return false;
			}
		}

		if (m_enable_depthbuffer)
		{
			if (!m_ds)
			{
				if (!LAPP.getGraphicsDevice()->createDepthStencilBuffer(size, m_ds.put()))
				{
					return false;
				}
			}
			else
			{
				if (!m_ds->setSize(size))
				{
					return false;
				}
			}
		}

		return true;
	}

	ResourceTextureImpl::ResourceTextureImpl(const char* name, core::ITexture2D* p_texture)
		: ResourceBaseImpl(ResourceType::Texture, name)
		, m_texture(p_texture)
		, m_is_rendertarget(false)
		, m_is_auto_resize(false)
		, m_enable_depthbuffer(false)
	{
	}
	// 渲染附件容器
	ResourceTextureImpl::ResourceTextureImpl(const char* name, int w, int h, bool ds)
		: ResourceBaseImpl(ResourceType::Texture, name)
		, m_is_rendertarget(true)
		, m_is_auto_resize(false)
		, m_enable_depthbuffer(ds)
	{
		auto const size = core::Vector2U((uint32_t)w, (uint32_t)h);
		if (!ResizeRenderTarget(size))
		{
			throw std::runtime_error("ResTexture::ResTexture createRenderTarget");
		}
	}
	// 自动调整大小的渲染附件容器
	ResourceTextureImpl::ResourceTextureImpl(const char* name, bool ds)
		: ResourceBaseImpl(ResourceType::Texture, name)
		, m_is_rendertarget(true)
		, m_is_auto_resize(true)
		, m_enable_depthbuffer(ds)
	{
		if (!ResizeRenderTarget(LAPP.GetRenderTargetManager()->GetAutoSizeRenderTargetSize()))
		{
			throw std::runtime_error("ResTexture::ResTexture createRenderTarget");
		}
		LAPP.GetRenderTargetManager()->AddAutoSizeRenderTarget(this);
	}
	ResourceTextureImpl::~ResourceTextureImpl()
	{
		if (m_is_auto_resize)
			LAPP.GetRenderTargetManager()->RemoveAutoSizeRenderTarget(this);
	}
}

namespace luastg {
	RenderTargetStackResourceTextureImpl::RenderTargetStackResourceTextureImpl(core::IRenderTarget* const rt, core::IDepthStencilBuffer* const ds) : m_rt(rt), m_ds(ds) {
	}
	RenderTargetStackResourceTextureImpl::~RenderTargetStackResourceTextureImpl() = default;
}
