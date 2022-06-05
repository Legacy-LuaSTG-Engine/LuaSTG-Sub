#include "RenderDev/f2dTexture11.h"
#include "RenderDev/f2dRenderDevice11.h"

// Texture2D

f2dTexture2D11::f2dTexture2D11(f2dRenderDevice* pDev, fuInt Width, fuInt Height)
{
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ITexture2D> tex_;
	if (static_cast<f2dRenderDevice11*>(pDev)->GetDevice()->createTexture(LuaSTG::Core::Vector2U(Width, Height), ~tex_))
		m_tex = static_cast<LuaSTG::Core::Graphics::Texture2D_D3D11*>(tex_.get());
	else
		throw std::runtime_error("f2dTexture2D11 (1)");
}
f2dTexture2D11::f2dTexture2D11(f2dRenderDevice* pDev, fcStr path, fBool bMipmap)
{
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ITexture2D> tex_;
	if (static_cast<f2dRenderDevice11*>(pDev)->GetDevice()->createTextureFromFile(path, bMipmap, ~tex_))
		m_tex = static_cast<LuaSTG::Core::Graphics::Texture2D_D3D11*>(tex_.get());
	else
		throw std::runtime_error("f2dTexture2D11 (2)");
}

/*

// RenderTarget

f2dRenderTarget11::f2dRenderTarget11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize)
	: m_dev((f2dRenderDevice11*)pDev)
	, m_Width(Width)
	, m_Height(Height)
	, m_bAutoResize(AutoResize)
{
	OnRenderSizeDependentResourcesCreate();
	m_dev->AttachListener(this);
}
f2dRenderTarget11::~f2dRenderTarget11()
{
	m_dev->RemoveListener(this);
}

void f2dRenderTarget11::OnRenderSizeDependentResourcesCreate()
{
	if (m_bAutoResize)
	{
		m_Width = m_dev->GetBufferWidth();
		m_Height = m_dev->GetBufferHeight();
	}
	else if (m_Width == 0 || m_Height == 0)
	{
		throw fcyException("f2dRenderTarget11::f2dRenderTarget11", "width or height is 0.");
	}
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IRenderTarget> rt_;
	if (m_dev->GetDevice()->createRenderTarget(LuaSTG::Core::Vector2U(m_Width, m_Height), ~rt_))
		m_rt = static_cast<LuaSTG::Core::Graphics::RenderTarget_D3D11*>(rt_.get());
	else
		throw std::runtime_error("f2dRenderTarget11");
	SetPremultipliedAlpha(m_PremultipliedAlpha);
}

// DepthStencil

f2dDepthStencil11::f2dDepthStencil11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize)
	: m_dev((f2dRenderDevice11*)pDev)
	, m_Width(Width)
	, m_Height(Height)
	, m_bAutoResize(AutoResize)
{
	OnRenderSizeDependentResourcesCreate();
	m_dev->AttachListener(this);
}
f2dDepthStencil11::~f2dDepthStencil11()
{
	m_dev->RemoveListener(this);
}

void f2dDepthStencil11::OnRenderSizeDependentResourcesCreate()
{
	if (m_bAutoResize)
	{
		m_Width = m_dev->GetBufferWidth();
		m_Height = m_dev->GetBufferHeight();
	}
	else if (m_Width == 0 || m_Height == 0)
	{
		throw fcyException("f2dRenderTarget11::f2dRenderTarget11", "width or height is 0.");
	}
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IDepthStencilBuffer> ds_;
	if (m_dev->GetDevice()->createDepthStencilBuffer(LuaSTG::Core::Vector2U(m_Width, m_Height), ~ds_))
		m_ds = static_cast<LuaSTG::Core::Graphics::DepthStencilBuffer_D3D11*>(ds_.get());
	else
		throw std::runtime_error("f2dDepthStencil11");
}

//*/
