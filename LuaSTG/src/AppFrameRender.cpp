#include "AppFrame.h"
#include "LConfig.h"
#include <d3d9.h>

namespace LuaSTGPlus
{
    void AppFrame::updateGraph2DBlendMode(BlendMode m)
    {
        auto& ctx = m_NewRenderer2D;
        switch (m)
        {
        default:
        case LuaSTGPlus::BlendMode::MulAlpha:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Alpha);
            break;
        case LuaSTGPlus::BlendMode::MulAdd:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Add);
            break;
        case LuaSTGPlus::BlendMode::MulRev:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::RevSub);
            break;
        case LuaSTGPlus::BlendMode::MulSub:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Sub);
            break;
        case LuaSTGPlus::BlendMode::AddAlpha:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Alpha);
            break;
        case LuaSTGPlus::BlendMode::AddAdd:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Add);
            break;
        case LuaSTGPlus::BlendMode::AddRev:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::RevSub);
            break;
        case LuaSTGPlus::BlendMode::AddSub:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Sub);
            break;
        case LuaSTGPlus::BlendMode::AlphaBal:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Inv);
            break;
        case LuaSTGPlus::BlendMode::MulMin:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Min);
            break;
        case LuaSTGPlus::BlendMode::MulMax:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Max);
            break;
        case LuaSTGPlus::BlendMode::MulMutiply:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Mul);
            break;
        case LuaSTGPlus::BlendMode::MulScreen:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::Screen);
            break;
        case LuaSTGPlus::BlendMode::AddMin:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Min);
            break;
        case LuaSTGPlus::BlendMode::AddMax:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Max);
            break;
        case LuaSTGPlus::BlendMode::AddMutiply:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Mul);
            break;
        case LuaSTGPlus::BlendMode::AddScreen:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Add);
            ctx.setBlendState(LuaSTG::Core::BlendState::Screen);
            break;
        case LuaSTGPlus::BlendMode::One:
            ctx.setVertexColorBlendState(LuaSTG::Core::VertexColorBlendState::Mul);
            ctx.setBlendState(LuaSTG::Core::BlendState::One);
            break;
        }
    }
    
    bool AppFrame::Render(ResSprite* p, float x, float y, float rot, float hscale, float vscale, float z) noexcept
    {
        assert(p);
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] Render: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        f2dSprite* pSprite = p->GetSprite();
        pSprite->SetZ(z);
        pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
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
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] Render: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        f2dSprite* pSprite = p->GetSpriteByTimer(ani_timer);
        pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
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
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] Render: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        p->Render(m_Graph2D, hscale, vscale);
        return true;
    }
    bool AppFrame::RenderRect(const char* name, float x1, float y1, float x2, float y2) noexcept
    {
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] RenderRect: 只有2D渲染器可以执行该方法");
        //    return false;
        //}

        fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
        if (!p)
        {
            spdlog::error("[luastg] RenderRect: 找不到图片精灵'{}'", name);
            return false;
        }
        
        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        // 渲染
        f2dSprite* pSprite = p->GetSprite();
        pSprite->SetZ(0.5f);
        pSprite->Draw(m_Graph2D, fcyRect(x1, y1, x2, y2), false);
        return true;
    }
    bool AppFrame::Render4V(
        const char* name,
        float x1, float y1, float z1,
        float x2, float y2, float z2, 
        float x3, float y3, float z3,
        float x4, float y4, float z4) noexcept
    {
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] Render4V: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
        if (!p)
        {
            spdlog::error("[luastg] Render4V: 找不到图片精灵'{}'", name);
            return false;
        }
        
        // 设置混合
        updateGraph2DBlendMode(p->GetBlendMode());
        
        f2dSprite* pSprite = p->GetSprite();
        pSprite->SetZ(0.5f);
        pSprite->Draw(m_Graph2D, fcyVec3(x1, y1, z1), fcyVec3(x2, y2, z2), fcyVec3(x3, y3, z3), fcyVec3(x4, y4, z4), false);
        return true;
    }
    bool AppFrame::RenderTexture(ResTexture* tex, BlendMode blend, const f2dGraphics2DVertex vertex[]) noexcept
    {
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] RenderTexture: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        // 设置混合
        updateGraph2DBlendMode(blend);
        
        // 复制坐标，修正UV到[0,1]区间
        f2dGraphics2DVertex tVertex[4] = {
            vertex[0],
            vertex[1],
            vertex[2],
            vertex[3],
        };
        float const du_ = 1.0f / (float)tex->GetTexture()->GetWidth();
        float const dv_ = 1.0f / (float)tex->GetTexture()->GetHeight();
        for (int i = 0; i < 4; ++i)
        {
            tVertex[i].u *= du_;
            tVertex[i].v *= dv_;
        }
        
        m_Graph2D->DrawQuad(tex->GetTexture(), tVertex, false);
        return true;
    }
    bool AppFrame::RenderTexture(const char* name, BlendMode blend, f2dGraphics2DVertex vertex[]) noexcept
    {
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] RenderTexture: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
        if (!p)
        {
            spdlog::error("[luastg] RenderTexture: 找不到纹理'{}'", name);
            return false;
        }
        
        // 设置混合
        updateGraph2DBlendMode(blend);
        
        // 修正UV到[0,1]区间
        float const du_ = 1.0f / (float)p->GetTexture()->GetWidth();
        float const dv_ = 1.0f / (float)p->GetTexture()->GetHeight();
        for (int i = 0; i < 4; ++i)
        {
            vertex[i].u *= du_;
            vertex[i].v *= dv_;
        }
        
        m_Graph2D->DrawQuad(p->GetTexture(), vertex, false);
        return true;
    }
    bool AppFrame::RenderTexture(
        const char* name, BlendMode blend,
        int vcount, const f2dGraphics2DVertex vertex[],
        int icount, const unsigned short indexs[]) noexcept
    {
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] RenderTexture: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
        
        fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
        if (!p)
        {
            spdlog::error("[luastg] RenderTexture: 找不到纹理'{}'", name);
            return false;
        }
        
        // 设置混合
        updateGraph2DBlendMode(blend);
        
        m_Graph2D->DrawRaw(p->GetTexture(), vcount, icount, vertex, indexs, false);
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
            const std::wstring wpath = fcyStringHelper::MultiByteToWideChar(path);
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
    void AppFrame::SaveTexture(f2dTexture2D* Tex, const char* path) noexcept
    {
        assert(Tex);
        if (!LAPP.GetRenderDev())
        {
            spdlog::error("[luastg] SaveTexture: f2dRenderDevice未准备好");
            return;
        }
        
        try
        {
            const char* path = luaL_checkstring(L, 2);
            const std::wstring wpath = fcyStringHelper::MultiByteToWideChar(path);
            fResult fr = LAPP.GetRenderDev()->SaveTexture(wpath.c_str(), Tex);
            if (FCYFAILED(fr))
                spdlog::error("[fancy2d] [f2dRenderDevice::SaveTexture] 保存纹理到'{}'失败(fResult={})", path, fr);
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] SaveTexture: 内存不足");
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
        return SaveTexture(resTex->GetTexture(), path);
    }

    // 废弃
    void AppFrame::updateGraph3DBlendMode(BlendMode m)
    {
        updateGraph2DBlendMode(m);
    }
    bool AppFrame::BeginScene() noexcept
    {
        if (!m_bRenderStarted)
        {
            spdlog::error("[luastg] 不能在RenderFunc以外的地方执行渲染");
            return false;
        }

        if (m_GraphType == GraphicsType::Graph2D)
        {
            fResult fr = m_Graph2D->Begin();
            if (FCYFAILED(fr))
            {
                spdlog::error("[fancy2d] [f2dGraphics2D::Begin] 失败(fResult={})", fr);
                return false;
            }
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

        if (m_GraphType == GraphicsType::Graph2D)
        {
            fResult fr = m_Graph2D->End();
            if (FCYFAILED(fr))
            {
                spdlog::error("[fancy2d] [f2dGraphics2D::End] 失败(fResult={})", fr);
                return false;
            }
        }

        return true;
    }
    void AppFrame::ClearScreen(const fcyColor& c) noexcept
    {
        m_pRenderDev->Clear(c);
    }
    bool AppFrame::SetViewport(double left, double right, double bottom, double top) noexcept
    {
        const auto rect = fcyRect(
            (float)((int)left),
            (float)((int)m_pRenderDev->GetBufferHeight() - (int)top),
            (float)((int)right),
            (float)((int)m_pRenderDev->GetBufferHeight() - (int)bottom)
        );
        fResult fr = m_pRenderDev->SetViewport(rect);
        if (FCYFAILED(fr))
        {
            spdlog::error("[fancy2d] [f2dRenderDevice::SetViewport] 设置视口(left:{}, right:{}, bottom:{}, top:{})失败(fResult={})",
                left, right, bottom, top, fr);
            return false;
        }
        return true;
    }
    bool AppFrame::SetScissorRect(double left, double right, double bottom, double top) noexcept
    {
        const auto rect = fcyRect(
            (float)((int)left),
            (float)((int)m_pRenderDev->GetBufferHeight() - (int)top),
            (float)((int)right),
            (float)((int)m_pRenderDev->GetBufferHeight() - (int)bottom)
        );
        fResult fr = m_pRenderDev->SetScissorRect(rect);
        if (FCYFAILED(fr))
        {
            spdlog::error("[fancy2d] [f2dRenderDevice::SetScissorRect] 设置裁剪矩形(left:{}, right:{}, bottom:{}, top:{})失败(fResult={})",
                left, right, bottom, top, fr);
            return false;
        }
        return true;
    }
    void AppFrame::SetOrtho(
        float left, float right, float bottom, float top,
        float znear, float zfar) noexcept
    {
        if (m_GraphType == GraphicsType::Graph2D)
        {
            // luastg的lua部分已经做了坐标修正
            // m_Graph2D->SetWorldTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(-0.5f, -0.5f, 0.f)));
            m_Graph2D->SetWorldTransform(fcyMatrix4::GetIdentity());

            //允许正交投影下下可以饶原点3D旋转图片精灵
            //m_Graph2D->SetViewTransform(fcyMatrix4::GetIdentity());
            m_Graph2D->SetViewTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(0.0f, 0.0f, znear + (zfar - znear) / 2.0f)));

            m_Graph2D->SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(left, right, bottom, top, znear, zfar));
        }
    }
    void AppFrame::SetPerspective(
        float eyeX, float eyeY, float eyeZ,
        float atX, float atY, float atZ,
        float upX, float upY, float upZ,
        float fovy, float aspect,
        float zn, float zf) noexcept
    {
        if (m_GraphType == GraphicsType::Graph2D)
        {
            m_Graph2D->SetWorldTransform(fcyMatrix4::GetIdentity());
            m_Graph2D->SetViewTransform(fcyMatrix4::GetLookAtLH(fcyVec3(eyeX, eyeY, eyeZ), fcyVec3(atX, atY, atZ), fcyVec3(upX, upY, upZ)));
            m_Graph2D->SetProjTransform(fcyMatrix4::GetPespctiveLH(aspect, fovy, zn, zf));
        }
    }
    void AppFrame::SetFog(float start, float end, fcyColor color)
    {
        if (m_Graph2D->IsInRender())
        {
            m_Graph2D->Flush();
        }

        // 从f2dRenderDevice中取出D3D设备
        IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pRenderDev->GetHandle();

        if (start != end)
        {
            pDev->SetRenderState(D3DRS_FOGENABLE, TRUE);
            pDev->SetRenderState(D3DRS_FOGCOLOR, color.argb);
            if (start == -1.0f)
            {
                pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);
                pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)(&end));
            }
            else if (start == -2.0f)
            {
                pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);
                pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)(&end));
            }
            else
            {
                pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
                pDev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&start));
                pDev->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&end));
            }
        }
        else
        {
            pDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
        }
    }
    void AppFrame::SetZBufferEnable(bool enable) noexcept
    {
        if (m_GraphType == GraphicsType::Graph2D)
        {
            m_Graph2D->Flush();
            m_pRenderDev->SetZBufferEnable(enable);
        }
    }
    void AppFrame::ClearZBuffer(float z) noexcept
    {
        if (m_GraphType == GraphicsType::Graph2D)
        {
            m_pRenderDev->ClearZBuffer(z);
        }
    }
};
