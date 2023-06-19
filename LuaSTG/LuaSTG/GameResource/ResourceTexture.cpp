﻿#include "GameResource/ResourceTexture.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	bool ResTexture::ResizeRenderTarget(Core::Vector2U size)
	{
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
			if (!m_ds)
			{
				if (!LAPP.GetAppModel()->getDevice()->createDepthStencilBuffer(size, ~m_ds))
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

	ResTexture::ResTexture(const char* name, Core::Graphics::ITexture2D* p_texture)
		: ResourceBase(ResourceType::Texture, name)
		, m_texture(p_texture)
		, m_is_rendertarget(false)
		, m_is_auto_resize(false)
		, m_enable_depthbuffer(false)
	{
	}
	// 渲染附件容器
	ResTexture::ResTexture(const char* name, int w, int h, bool ds)
		: ResourceBase(ResourceType::Texture, name)
		, m_is_rendertarget(true)
		, m_is_auto_resize(false)
		, m_enable_depthbuffer(ds)
	{
		auto const size = Core::Vector2U((uint32_t)w, (uint32_t)h);
		if (!ResizeRenderTarget(size))
		{
			throw std::runtime_error("ResTexture::ResTexture createRenderTarget");
		}
	}
	// 自动调整大小的渲染附件容器
	ResTexture::ResTexture(const char* name, bool ds)
		: ResourceBase(ResourceType::Texture, name)
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
	ResTexture::~ResTexture()
	{
		if (m_is_auto_resize)
			LAPP.GetRenderTargetManager()->RemoveAutoSizeRenderTarget(this);
	}
}
