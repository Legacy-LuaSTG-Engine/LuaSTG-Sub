#include "AppFrame.h"
#include "LConfig.h"
#include <numbers>

namespace LuaSTGPlus {
    bool AppFrame::CheckRenderTargetInUse(ResTexture* rt)LNOEXCEPT
    {
        if (!rt || !rt->IsRenderTarget() || m_stRenderTargetStack.empty())
            return false;
        return rt == *(m_stRenderTargetStack.back());
    }
    
    bool AppFrame::PushRenderTarget(ResTexture* rt)LNOEXCEPT
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
    bool AppFrame::PopRenderTarget()LNOEXCEPT
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
    
    fcyVec2 AppFrame::GetCurrentRenderTargetSize()
    {
        if (!m_stRenderTargetStack.empty())
        {
            ResTexture* rt = *(m_stRenderTargetStack.back());
            return fcyVec2((float)rt->GetTexture()->getSize().x, (float)rt->GetTexture()->getSize().y);
        }
        else
        {
            return fcyVec2((float)GetAppModel()->getSwapChain()->getWidth(), (float)GetAppModel()->getSwapChain()->getHeight());
        }
    }

    void AppFrame::DebugSetGeometryRenderState()
    {
        using namespace LuaSTG::Core::Graphics;
        auto* r2d = GetRenderer2D();
        r2d->setBlendState(IRenderer::BlendState::Alpha);
        r2d->setDepthState(IRenderer::DepthState::Disable);
        r2d->setFogState(IRenderer::FogState::Disable, {}, 0.0f, 0.0f);
        r2d->setVertexColorBlendState(IRenderer::VertexColorBlendState::One);
        r2d->setTexture(nullptr);
    }
    void AppFrame::DebugDrawCircle(float const x, float const y, float const r, fcyColor const color)
    {
        if (std::abs(r) >= std::numeric_limits<float>::min())
        {
            using namespace LuaSTG::Core;
            using namespace LuaSTG::Core::Graphics;
            auto* r2d = GetRenderer2D();
            // 分割 32 份，圆周上 32 个点以及中心点，共 32 个三角形，需要 32 * 3 个索引
            IRenderer::DrawVertex* vert = nullptr;
            IRenderer::DrawIndex* vidx = nullptr;
            uint16_t vidx_offset = 0;
            r2d->drawRequest(32 + 1, 32 * 3, &vert, &vidx, &vidx_offset);
            // 计算顶点
            vert[0] = IRenderer::DrawVertex(x, y, 0.5f, 0.0f, 0.0f, color.argb);
            for (size_t i = 1; i <= 32; i += 1)
            {
                constexpr float const da = std::numbers::pi_v<float> *2.0f / 32.0f;
                float const angle = (float)(i - 1) * da;
                vert[i] = IRenderer::DrawVertex(x + r * std::cosf(angle), y + r * std::sinf(angle), 0.5f, 0.0f, 0.0f, color.argb);
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
    void AppFrame::DebugDrawRect(float const x, float const y, float const a, float const b, float const rot, fcyColor const color)
    {
        if (std::abs(a) >= std::numeric_limits<float>::min() && std::abs(b) >= std::numeric_limits<float>::min())
        {
            using namespace LuaSTG::Core;
            using namespace LuaSTG::Core::Graphics;
            auto* r2d = GetRenderer2D();
            // 计算出矩形的4个顶点
            IRenderer::DrawVertex vert[4] = {
                IRenderer::DrawVertex(-a, -b, 0.5f, 0.0f, 0.0f, color.argb),
                IRenderer::DrawVertex( a, -b, 0.5f, 0.0f, 1.0f, color.argb),
                IRenderer::DrawVertex( a,  b, 0.5f, 1.0f, 1.0f, color.argb),
                IRenderer::DrawVertex(-a,  b, 0.5f, 1.0f, 0.0f, color.argb),
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
    void AppFrame::DebugDrawEllipse(float const x, float const y, float const a, float const b, float const rot, fcyColor const color)
    {
        if (std::abs(a) >= std::numeric_limits<float>::min() && std::abs(b) >= std::numeric_limits<float>::min())
        {
            using namespace LuaSTG::Core;
            using namespace LuaSTG::Core::Graphics;
            auto* r2d = GetRenderer2D();
            // 分割 36 份，椭圆周上 36 个点以及中心点，共 36 个三角形，需要 36 * 3 个索引
            IRenderer::DrawVertex* vert = nullptr;
            IRenderer::DrawIndex* vidx = nullptr;
            uint16_t vidx_offset = 0;
            r2d->drawRequest(36 + 1, 36 * 3, &vert, &vidx, &vidx_offset);
            // 计算顶点
            vert[0] = IRenderer::DrawVertex(x, y, 0.5f, 0.0f, 0.0f, color.argb);
            for (size_t i = 1; i <= 36; i += 1)
            {
                constexpr float const da = std::numbers::pi_v<float> * 2.0f / 36.0f;
                float const angle = (float)(i - 1) * da;
                vert[i] = IRenderer::DrawVertex(
                    a * std::cosf(angle),
                    b * std::sinf(angle),
                    0.5f, 0.0f, 0.0f, color.argb);
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

    // 废弃
    //static f2dGraphics2DVertex vcache[2048];
    //static fuShort             icache[8192];
    bool AppFrame::RenderSector(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
        fcyVec2 pos, float rot, float exp, float r1, float r2, int div)
    {
        //// 计算顶点
        //if (div < 1) {
        //    return true; // 不需要绘制
        //}
        //int total = div + 1;
        //int total_vertex = total * 2;
        //int total_index = div * 6;
        //if (total_vertex > 2048 || total_index > 8192) {
        //    spdlog::error("[luastg] RenderSector: 顶点数量过多");
        //    return false;
        //}
        //// 寻找纹理
        //fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
        //if (!p)
        //{
        //    spdlog::error("[luastg] RenderSector: 找不到纹理'{}'", name);
        //    return false;
        //}
        //// 更新混合模式
        //updateGraph2DBlendMode(blend);
        //// 计算顶点
        //float rotv = rot, rotd = -exp / (float)div, cosv = 0.0f, sinv = 0.0f;
        //float texw = (float)p->GetTexture()->GetWidth(), texh = (float)p->GetTexture()->GetHeight();
        //float uvcu = uv.a.x, uvdu = (uv.b.x - uv.a.x) / (float)div;
        //float uvcv = uv.a.y, uvdv = (uv.b.y - uv.a.y) / (float)div;
        //for (int i = 0; i <= div; i++) {
        //    f2dGraphics2DVertex& vt1 = vcache[i * 2]; // 内圈顶点
        //    f2dGraphics2DVertex& vt2 = vcache[i * 2 + 1]; // 外圈顶点
        //    // XY
        //    cosv = cosf(rotv); sinv = sinf(rotv);
        //    rotv += rotd;
        //    vt1.x = pos.x + cosv * r1; vt1.y = pos.y + sinv * r1;
        //    vt2.x = pos.x + cosv * r2; vt2.y = pos.y + sinv * r2;
        //    // Z
        //    vt1.z = 0.5f; vt2.z = 0.5f;
        //    // UV
        //    if (tran) {
        //        vt1.u = vt2.u = uvcu / texw;
        //        vt1.v = uv.b.y / texh; vt2.v = uv.a.y / texh;
        //        uvcu += uvdu;
        //    }
        //    else {
        //        vt1.u = uv.a.x / texw; vt2.u = uv.b.x / texw;
        //        vt1.v = vt2.v = uvcv / texh;
        //        uvcv += uvdv;
        //    }
        //    // 颜色
        //    vt1.color = color1.argb; vt2.color = color2.argb;
        //}
        //// 计算索引
        //fuShort indv = 0;
        //for (int i = 0; i < div; i++) {
        //    // 第一个三角形
        //    icache[i * 6 + 0] = indv + 1;
        //    icache[i * 6 + 1] = indv + 3;
        //    icache[i * 6 + 2] = indv + 2;
        //    // 第二个三角形
        //    icache[i * 6 + 3] = indv + 1;
        //    icache[i * 6 + 4] = indv + 2;
        //    icache[i * 6 + 5] = indv + 0;
        //    // 下一批
        //    indv += 2;
        //}
        //// 绘制
        //auto ret = m_Graph2D->DrawRaw(p->GetTexture(), total_vertex, total_index, vcache, icache, false);
        //return (FCYERR_OK == ret);
        return true;
    }
    bool AppFrame::RenderAnnulus(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
        fcyVec2 pos, float rot, float r1, float r2, int div, int rep)
    {
        //bool ret = true;
        //float rotv = rot;
        //float persector = (2.0f * (float)LPI_FULL) / (float)rep;
        //for (int i = 0; i < rep; i++) {
        //    ret = ret && RenderSector(name, uv, tran, blend, color1, color2, pos, rotv, persector, r1, r2, div);
        //    rotv += persector;
        //}
        return true;
    }
};
