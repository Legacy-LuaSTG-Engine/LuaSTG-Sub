#include "GameResource/ResourceTexture.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	bool ResTexture::createResources()
	{
		assert(m_swapchain);
		m_texture = nullptr;
		m_rt = nullptr;
		m_ds = nullptr;

		auto const size = Core::Vector2U(m_swapchain->getWidth(), m_swapchain->getHeight());
		if (!m_rt)
		{
			if (!LAPP.GetAppModel()->getDevice()->createRenderTarget(size, ~m_rt))
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
			if (!LAPP.GetAppModel()->getDevice()->createDepthStencilBuffer(size, ~m_ds))
			{
				return false;
			}
		}

		return true;
	}
	void ResTexture::onSwapChainCreate()
	{
		createResources();
	}
	void ResTexture::onSwapChainDestroy()
	{
	}

	ResTexture::ResTexture(const char* name, Core::Graphics::ITexture2D* p_texture)
		: Resource(ResourceType::Texture, name)
		, m_texture(p_texture)
		, m_is_rendertarget(false)
		, m_is_auto_resize(false)
		, m_enable_depthbuffer(false)
	{
	}
	// 渲染附件容器
	ResTexture::ResTexture(const char* name, int w, int h, bool ds)
		: Resource(ResourceType::Texture, name)
		, m_is_rendertarget(true)
		, m_is_auto_resize(false)
		, m_enable_depthbuffer(ds)
	{
		auto const size = Core::Vector2U((uint32_t)w, (uint32_t)h);
		if (!LAPP.GetAppModel()->getDevice()->createRenderTarget(size, ~m_rt))
		{
			throw std::runtime_error("ResTexture::ResTexture createRenderTarget");
		}
		m_texture = m_rt->getTexture();
		if (m_enable_depthbuffer)
		{
			if (!LAPP.GetAppModel()->getDevice()->createDepthStencilBuffer(size, ~m_ds))
			{
				throw std::runtime_error("ResTexture::ResTexture createDepthStencilBuffer");
			}
		}
	}
	// 自动调整大小的渲染附件容器
	ResTexture::ResTexture(const char* name, bool ds)
		: Resource(ResourceType::Texture, name)
		, m_is_rendertarget(true)
		, m_is_auto_resize(true)
		, m_enable_depthbuffer(ds)
	{
		m_swapchain = LAPP.GetAppModel()->getSwapChain();
		if (!createResources())
		{
			throw std::runtime_error("ResTexture::ResTexture createRenderTarget");
		}
		m_swapchain->addEventListener(this);
	}
	ResTexture::~ResTexture()
	{
		if (m_swapchain && m_is_auto_resize)
			m_swapchain->removeEventListener(this);
	}
}
