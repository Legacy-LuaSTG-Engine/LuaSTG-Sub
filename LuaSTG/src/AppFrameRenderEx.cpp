﻿#include "AppFrame.h"
#include "LConfig.h"
#include <d3d9.h>

namespace LuaSTGPlus {
    bool AppFrame::CheckRenderTargetInUse(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT
    {
        if (!rt || !rt->IsRenderTarget() || m_stRenderTargetStack.empty())
            return false;
        
        return rt == m_stRenderTargetStack.back();
    }
    bool AppFrame::CheckRenderTargetInUse(ResTexture* rt)LNOEXCEPT
    {
        if (!rt || !rt->IsRenderTarget() || m_stRenderTargetStack.empty())
            return false;
        
        return rt->GetTexture() == *m_stRenderTargetStack.back();
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
        
        //return PushRenderTarget(rt->GetTexture());
        // 下面是新增的

        if (FCYFAILED(m_pRenderDev->SetRenderTargetAndDepthStencilSurface(rt->GetTexture(), rt->GetDepthStencilSurface()))) // 这个可能是空指针，如果传空指针进去代表换回 RenderDev 默认的 ds
        {
            spdlog::error("[luastg] PushRenderTarget: 内部错误 (f2dRenderDevice::SetRenderTargetAndDepthStencilSurface failed.)");
            return false;
        }
        
        m_stRenderTargetStack.push_back(rt->GetTexture());
        m_stDepthStencilStack.push_back(rt->GetDepthStencilSurface()); // 这个可能是空指针
        
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
        if (m_stDepthStencilStack.empty())
        {
            spdlog::error("[luastg] PopRenderTarget: DepthStencil 栈已为空");
            return false;
        }
        
        m_stRenderTargetStack.pop_back();
        m_stDepthStencilStack.pop_back();

        f2dTexture2D* pTex = nullptr;
        f2dDepthStencilSurface* pSurface = nullptr;
        if (!m_stRenderTargetStack.empty())
            pTex = *m_stRenderTargetStack.back();
        if (!m_stDepthStencilStack.empty())
            pSurface = *m_stDepthStencilStack.back();

        if (FCYFAILED(m_pRenderDev->SetRenderTargetAndDepthStencilSurface(pTex, pSurface))) // 这个可能是空指针，如果传空指针进去代表换回 RenderDev 默认的 ds
        {
            spdlog::error("[luastg] PushRenderTarget: 内部错误 (f2dRenderDevice::SetRenderTargetAndDepthStencilSurface failed.)");
            return false;
        }

        return true;
    }
    
    // 废弃
    bool AppFrame::PostEffect(fcyRefPointer<f2dTexture2D> rt, ResFX* shader, BlendMode blend)LNOEXCEPT
    {
        //2dEffectTechnique* pTechnique = shader->GetEffect()->GetTechnique(0U);
        //
        //f (!pTechnique)
        //
        //   spdlog::error("[luastg] PostEffect: 无效的后处理特效");
        //   return false;
        //
        //
        /// 纹理使用检查
        //f (CheckRenderTargetInUse(rt))
        //
        //   spdlog::error("[luastg] PostEffect: RenderTarget无法同时绑定为着色器纹理资源和渲染输出目标");
        //   return false;
        //
        //
        /// 终止渲染过程
        //ool bRestartRenderPeriod = false;
        //witch (m_GraphType)
        //
        //ase GraphicsType::Graph2D:
        //   if (m_Graph2D->IsInRender())
        //   {
        //       bRestartRenderPeriod = true;
        //       m_Graph2D->End();
        //   }
        //   break;
        //ase GraphicsType::Graph3D:
        //   if (m_Graph3D->IsInRender())
        //   {
        //       bRestartRenderPeriod = true;
        //       m_Graph3D->End();
        //   }
        //   break;
        //
        //
        /// 更新渲染状态
        //pdateGraph3DBlendMode(blend);
        //
        /// 关闭fog
        //Direct3DDevice9* pDev = (IDirect3DDevice9*)m_pRenderDev->GetHandle();
        //WORD iFogEnabled = FALSE;
        //Dev->GetRenderState(D3DRS_FOGENABLE, &iFogEnabled);
        //f (iFogEnabled == TRUE)
        //   pDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
        //
        /// 设置effect
        //hader->SetPostEffectTexture(rt);
        //hader->SetViewport(m_pRenderDev->GetViewport());
        //hader->SetScreenSize(fcyVec2((float)m_pRenderDev->GetBufferWidth(), (float)m_pRenderDev->GetBufferHeight()));
        //_Graph3D->SetEffect(shader->GetEffect());
        //f (FCYFAILED(m_Graph3D->Begin()))
        //
        //   // ！ 异常退出不可恢复渲染过程
        //   spdlog::error("[luastg] PostEffect: 内部错误 (f2dGraphics3D::Begin failed)");
        //   return false;
        //
        /// 执行所有的pass
        //or (fuInt i = 0; i < pTechnique->GetPassCount(); ++i)
        //
        //   m_Graph3D->BeginPass(i);
        //   m_Graph3D->RenderPostEffect();
        //   m_Graph3D->EndPass();
        //
        //_Graph3D->End();
        //hader->SetPostEffectTexture(NULL);
        //
        /// 检查是否开启了雾
        //f (iFogEnabled == TRUE)
        //   pDev->SetRenderState(D3DRS_FOGENABLE, TRUE);
        //
        /// 重启渲染过程
        //f (bRestartRenderPeriod)
        //
        //   switch (m_GraphType)
        //   {
        //   case GraphicsType::Graph2D:
        //       m_Graph2D->Begin();
        //       break;
        //   case GraphicsType::Graph3D:
        //       m_Graph3D->Begin();
        //       break;
        //   }
        //
        //
        return true;
    }
    bool AppFrame::PostEffect(ResTexture* rt, ResFX* shader, BlendMode blend)LNOEXCEPT
    {
        //if (!m_bRenderStarted)
        //{
        //    spdlog::error("[luastg] PostEffect: 无效调用");
        //    return false;
        //}
        //
        //return PostEffect(rt->GetTexture(), shader, blend);
        return true;
    }
    //static f2dGraphics2DVertex vcache[2048];
    //static fuShort             icache[8192];
    bool AppFrame::RenderSector(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
        fcyVec2 pos, float rot, float exp, float r1, float r2, int div)
    {
        //// 检查渲染器
        //if (m_GraphType != GraphicsType::Graph2D)
        //{
        //    spdlog::error("[luastg] RenderSector: 只有2D渲染器可以执行该方法");
        //    return false;
        //}
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
