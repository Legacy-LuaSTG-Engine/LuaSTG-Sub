#include "AppFrame.h"
#include "LConfig.h"
#include "utility/encoding.hpp"

namespace LuaSTGPlus
{
    void AppFrame::updateGraph2DBlendMode(BlendMode blend)
    {
        using namespace LuaSTG::Core::Graphics;
        auto* ctx = m_pAppModel->getRenderer();
        switch (blend)
        {
        default:
        case LuaSTGPlus::BlendMode::MulAlpha:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Alpha);
            break;
        case LuaSTGPlus::BlendMode::MulAdd:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Add);
            break;
        case LuaSTGPlus::BlendMode::MulRev:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::RevSub);
            break;
        case LuaSTGPlus::BlendMode::MulSub:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Sub);
            break;
        case LuaSTGPlus::BlendMode::AddAlpha:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Alpha);
            break;
        case LuaSTGPlus::BlendMode::AddAdd:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Add);
            break;
        case LuaSTGPlus::BlendMode::AddRev:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::RevSub);
            break;
        case LuaSTGPlus::BlendMode::AddSub:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Sub);
            break;
        case LuaSTGPlus::BlendMode::AlphaBal:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Inv);
            break;
        case LuaSTGPlus::BlendMode::MulMin:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Min);
            break;
        case LuaSTGPlus::BlendMode::MulMax:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Max);
            break;
        case LuaSTGPlus::BlendMode::MulMutiply:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Mul);
            break;
        case LuaSTGPlus::BlendMode::MulScreen:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::Screen);
            break;
        case LuaSTGPlus::BlendMode::AddMin:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Min);
            break;
        case LuaSTGPlus::BlendMode::AddMax:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Max);
            break;
        case LuaSTGPlus::BlendMode::AddMutiply:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Mul);
            break;
        case LuaSTGPlus::BlendMode::AddScreen:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Add);
            ctx->setBlendState(IRenderer::BlendState::Screen);
            break;
        case LuaSTGPlus::BlendMode::One:
            ctx->setVertexColorBlendState(IRenderer::VertexColorBlendState::Mul);
            ctx->setBlendState(IRenderer::BlendState::One);
            break;
        }
    }
    
    bool AppFrame::Render(ResSprite* p, float x, float y, float rot, float hscale, float vscale, float z) noexcept
    {
        assert(p);

        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        LuaSTG::Core::Graphics::ISprite* pSprite = p->GetSprite();
        pSprite->setZ(z);
        pSprite->draw(LuaSTG::Core::Vector2F(x, y), LuaSTG::Core::Vector2F(hscale, vscale), rot);
        return true;
    }
    bool AppFrame::Render(const char* name, float x, float y, float rot, float hscale, float vscale, float z) noexcept
    {
        fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
        if (!p)
        {
            spdlog::error("[luastg] Render: 找不到图片精灵'{}'", name);
            return false;
        }
        return Render(p, x, y, rot, hscale, vscale, z);
    }
    bool AppFrame::Render(ResAnimation* p, int ani_timer, float x, float y, float rot, float hscale, float vscale) noexcept
    {
        assert(p);

        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        LuaSTG::Core::Graphics::ISprite* pSprite = p->GetSpriteByTimer(ani_timer);
        pSprite->draw(LuaSTG::Core::Vector2F(x, y), LuaSTG::Core::Vector2F(hscale, vscale), rot);
        return true;
    }
    bool AppFrame::RenderAnimation(const char* name, int timer, float x, float y, float rot, float hscale, float vscale) noexcept
    {
        fcyRefPointer<ResAnimation> p = m_ResourceMgr.FindAnimation(name);
        if (!p)
        {
            spdlog::error("[luastg] Render: 找不到动画精灵'{}'", name);
            return false;
        }
        return Render(p, timer, x, y, rot, hscale, vscale);
    }
    bool AppFrame::Render(ResParticle::ParticlePool* p, float hscale, float vscale) noexcept
    {
        assert(p);

        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        p->Render(hscale, vscale);
        return true;
    }
    bool AppFrame::RenderTexture(ResTexture* tex, BlendMode blend, const f2dGraphics2DVertex vertex[]) noexcept
    {
        // 设置混合
        updateGraph2DBlendMode(blend);
        
        // 复制坐标，修正UV到[0,1]区间
        LuaSTG::Core::Vector2U const tSize = tex->GetTexture()->getSize();
        float const us = 1.0f / (float)tSize.x;
        float const vs = 1.0f / (float)tSize.y;
        using VTX = LuaSTG::Core::Graphics::IRenderer::DrawVertex;
        VTX tVertex[4] = {
            VTX(vertex[0].x, vertex[0].y, vertex[0].z, vertex[0].u * us, vertex[0].v * vs, vertex[0].color),
            VTX(vertex[1].x, vertex[1].y, vertex[1].z, vertex[1].u * us, vertex[1].v * vs, vertex[1].color),
            VTX(vertex[2].x, vertex[2].y, vertex[2].z, vertex[2].u * us, vertex[2].v * vs, vertex[2].color),
            VTX(vertex[3].x, vertex[3].y, vertex[3].z, vertex[3].u * us, vertex[3].v * vs, vertex[3].color),
        };
        
        GetRenderer2D()->setTexture(tex->GetTexture());
        GetRenderer2D()->drawQuad(tVertex);
        return true;
    }
    bool AppFrame::RenderTexture(const char* name, BlendMode blend, f2dGraphics2DVertex vertex[]) noexcept
    {
        fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
        if (!p)
        {
            spdlog::error("[luastg] RenderTexture: 找不到纹理'{}'", name);
            return false;
        }
        return RenderTexture(*p, blend, vertex);
    }
    bool AppFrame::RenderTexture(
        const char* name, BlendMode blend,
        int vcount, const f2dGraphics2DVertex vertex[],
        int icount, const unsigned short indexs[]) noexcept
    {
        fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
        if (!p)
        {
            spdlog::error("[luastg] RenderTexture: 找不到纹理'{}'", name);
            return false;
        }
        
        // 设置混合
        updateGraph2DBlendMode(blend);
        
        using namespace LuaSTG::Core::Graphics;
        GetRenderer2D()->setTexture(p->GetTexture());
        GetRenderer2D()->drawRaw(
            (IRenderer::DrawVertex*)vertex,
            (uint16_t)vcount,
            indexs,
            (uint16_t)icount);
        return true;
    }
    
    void AppFrame::SnapShot(const char* path) noexcept
    {
        if (!LAPP.GetRenderDev())
        {
            spdlog::error("[luastg] Snapshot: f2dRenderDevice未准备好");
            return;
        }
        
        try
        {
            const std::wstring wpath = std::move(utility::encoding::to_wide(path));
            fResult fr = LAPP.GetRenderDev()->SaveScreen(wpath.c_str());
            if (FCYFAILED(fr))
                spdlog::error("[fancy2d] [f2dRenderDevice::SaveScreen] 保存截图到'{}'失败(fResult={})", path, fr);
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] Snapshot: 内存不足");
        }
        catch (const fcyException& e)
        {
            spdlog::error("[fancy2d] [{}] {}", e.GetSrc(), e.GetDesc());
        }
    }
    void AppFrame::SaveTexture(const char* tex_name, const char* path) noexcept
    {
        fcyRefPointer<ResTexture> resTex = LRES.FindTexture(tex_name);
        if (!resTex)
        {
            spdlog::error("[luastg] SaveTexture: 找不到纹理资源'{}'", tex_name);
            return;
        }
        resTex->GetTexture()->saveToFile(path);
    }

    // 废弃
    bool AppFrame::BeginScene() noexcept
    {
        if (!m_bRenderStarted)
        {
            spdlog::error("[luastg] 不能在RenderFunc以外的地方执行渲染");
            return false;
        }

        fResult fr = m_Graph2D->Begin();
        if (FCYFAILED(fr))
        {
            spdlog::error("[fancy2d] [f2dGraphics2D::Begin] 失败(fResult={})", fr);
            return false;
        }
        
        return true;
    }
    bool AppFrame::EndScene() noexcept
    {
        if (!m_bRenderStarted)
        {
            spdlog::error("[luastg] 不能在RenderFunc以外的地方执行渲染");
            return false;
        }

        fResult fr = m_Graph2D->End();
        if (FCYFAILED(fr))
        {
            spdlog::error("[fancy2d] [f2dGraphics2D::End] 失败(fResult={})", fr);
            return false;
        }

        return true;
    }
};
