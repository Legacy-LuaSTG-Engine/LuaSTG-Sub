#include "AppFrame.h"

namespace LuaSTGPlus
{
    bool AppFrame::CheckRenderTargetInUse(ResTexture* rt)noexcept
    {
        if (!rt || !rt->IsRenderTarget() || m_stRenderTargetStack.empty())
            return false;
        return rt == *(m_stRenderTargetStack.back());
    }
    
    bool AppFrame::PushRenderTarget(ResTexture* rt)noexcept
    {
        if (!rt || !rt->IsRenderTarget())
        {
            assert(false);  // 这不该发生
            return false;
        }
        if (!m_bRenderStarted)
        {
            spdlog::error("[luastg] PushRenderTarget: 无效调用");
            return false;
        }
        
        GetRenderer2D()->setRenderAttachment(
            rt->GetRenderTarget(),
            rt->GetDepthStencilBuffer()
        );

        m_stRenderTargetStack.push_back(rt);

        return true;
    }
    bool AppFrame::PopRenderTarget()noexcept
    {
        if (!m_bRenderStarted)
        {
            spdlog::error("[luastg] PopRenderTarget: 无效调用");
            return false;
        }
        
        if (m_stRenderTargetStack.empty())
        {
            spdlog::error("[luastg] PopRenderTarget: RenderTarget 栈已为空");
            return false;
        }
        
        m_stRenderTargetStack.pop_back();

        if (!m_stRenderTargetStack.empty())
        {
            ResTexture* rt = *(m_stRenderTargetStack.back());
            GetRenderer2D()->setRenderAttachment(
                rt->GetRenderTarget(),
                rt->GetDepthStencilBuffer()
            );
        }
        else
        {
            GetAppModel()->getSwapChain()->applyRenderAttachment();
        }

        return true;
    }
    
    Core::Vector2U AppFrame::GetCurrentRenderTargetSize()
    {
        if (!m_stRenderTargetStack.empty())
        {
            ResTexture* rt = *(m_stRenderTargetStack.back());
            return rt->GetTexture()->getSize();
        }
        else
        {
            return Core::Vector2U(
                GetAppModel()->getSwapChain()->getWidth(),
                GetAppModel()->getSwapChain()->getHeight());
        }
    }

    void AppFrame::DebugSetGeometryRenderState()
    {
        using namespace Core::Graphics;
        auto* r2d = GetRenderer2D();
        r2d->setBlendState(IRenderer::BlendState::Alpha);
        r2d->setDepthState(IRenderer::DepthState::Disable);
        r2d->setFogState(IRenderer::FogState::Disable, {}, 0.0f, 0.0f);
        r2d->setVertexColorBlendState(IRenderer::VertexColorBlendState::One);
        r2d->setTexture(nullptr);
    }
    void AppFrame::DebugDrawCircle(float const x, float const y, float const r, Core::Color4B const color)
    {
        if (std::abs(r) >= std::numeric_limits<float>::min())
        {
            using namespace Core;
            using namespace Core::Graphics;
            auto* r2d = GetRenderer2D();
            // 分割 32 份，圆周上 32 个点以及中心点，共 32 个三角形，需要 32 * 3 个索引
            IRenderer::DrawVertex* vert = nullptr;
            IRenderer::DrawIndex* vidx = nullptr;
            uint16_t vidx_offset = 0;
            r2d->drawRequest(32 + 1, 32 * 3, &vert, &vidx, &vidx_offset);
            // 计算顶点
            vert[0] = IRenderer::DrawVertex(x, y, 0.5f, 0.0f, 0.0f, color.color());
            for (size_t i = 1; i <= 32; i += 1)
            {
                constexpr float const da = std::numbers::pi_v<float> *2.0f / 32.0f;
                float const angle = (float)(i - 1) * da;
                vert[i] = IRenderer::DrawVertex(x + r * std::cosf(angle), y + r * std::sinf(angle), 0.5f, 0.0f, 0.0f, color.color());
            }
            // 计算索引
            IRenderer::DrawIndex* p_vidx = vidx;
            for (size_t i = 1; i < 32; i += 1)
            {
                p_vidx[0] = vidx_offset; // + 0;
                p_vidx[1] = vidx_offset + (IRenderer::DrawIndex)i;
                p_vidx[2] = vidx_offset + (IRenderer::DrawIndex)(i + 1);
                p_vidx += 3;
            }
            p_vidx[0] = vidx_offset; // + 0;
            p_vidx[1] = vidx_offset + 32;
            p_vidx[2] = vidx_offset + 1;
            // p_vidx += 3;
        }
    }
    void AppFrame::DebugDrawRect(float const x, float const y, float const a, float const b, float const rot, Core::Color4B const color)
    {
        if (std::abs(a) >= std::numeric_limits<float>::min() && std::abs(b) >= std::numeric_limits<float>::min())
        {
            using namespace Core;
            using namespace Core::Graphics;
            auto* r2d = GetRenderer2D();
            // 计算出矩形的4个顶点
            IRenderer::DrawVertex vert[4] = {
                IRenderer::DrawVertex(-a, -b, 0.5f, 0.0f, 0.0f, color.color()),
                IRenderer::DrawVertex( a, -b, 0.5f, 0.0f, 1.0f, color.color()),
                IRenderer::DrawVertex( a,  b, 0.5f, 1.0f, 1.0f, color.color()),
                IRenderer::DrawVertex(-a,  b, 0.5f, 1.0f, 0.0f, color.color()),
            };
            // 变换
            float const cos_v = std::cosf(rot);
            float const sin_v = std::sinf(rot);
            for (size_t i = 0; i < 4; i += 1)
            {
                float const tx = vert[i].x * cos_v - vert[i].y * sin_v;
                float const ty = vert[i].x * sin_v + vert[i].y * cos_v;
                vert[i].x = x + tx;
                vert[i].y = y + ty;
            }
            //绘制
            r2d->drawQuad(vert);
        }
    }
    void AppFrame::DebugDrawEllipse(float const x, float const y, float const a, float const b, float const rot, Core::Color4B const color)
    {
        if (std::abs(a) >= std::numeric_limits<float>::min() && std::abs(b) >= std::numeric_limits<float>::min())
        {
            using namespace Core;
            using namespace Core::Graphics;
            auto* r2d = GetRenderer2D();
            // 分割 36 份，椭圆周上 36 个点以及中心点，共 36 个三角形，需要 36 * 3 个索引
            IRenderer::DrawVertex* vert = nullptr;
            IRenderer::DrawIndex* vidx = nullptr;
            uint16_t vidx_offset = 0;
            r2d->drawRequest(36 + 1, 36 * 3, &vert, &vidx, &vidx_offset);
            // 计算顶点
            vert[0] = IRenderer::DrawVertex(x, y, 0.5f, 0.0f, 0.0f, color.color());
            for (size_t i = 1; i <= 36; i += 1)
            {
                constexpr float const da = std::numbers::pi_v<float> * 2.0f / 36.0f;
                float const angle = (float)(i - 1) * da;
                vert[i] = IRenderer::DrawVertex(
                    a * std::cosf(angle),
                    b * std::sinf(angle),
                    0.5f, 0.0f, 0.0f, color.color());
            }
            // 变换
            float const cos_v = std::cosf(rot);
            float const sin_v = std::sinf(rot);
            for (size_t i = 1; i <= 36; i += 1)
            {
                float const tx = vert[i].x * cos_v - vert[i].y * sin_v;
                float const ty = vert[i].x * sin_v + vert[i].y * cos_v;
                vert[i].x = x + tx;
                vert[i].y = y + ty;
            }
            // 计算索引
            IRenderer::DrawIndex* p_vidx = vidx;
            for (size_t i = 1; i < 36; i += 1)
            {
                p_vidx[0] = vidx_offset; // + 0;
                p_vidx[1] = vidx_offset + (IRenderer::DrawIndex)i;
                p_vidx[2] = vidx_offset + (IRenderer::DrawIndex)(i + 1);
                p_vidx += 3;
            }
            p_vidx[0] = vidx_offset; // + 0;
            p_vidx[1] = vidx_offset + 36;
            p_vidx[2] = vidx_offset + 1;
            // p_vidx += 3;
        }
    }
};
