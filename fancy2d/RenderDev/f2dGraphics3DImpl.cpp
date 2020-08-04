#include "RenderDev/f2dGraphics3DImpl.h"

#include "RenderDev/f2dEffectImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dGraphics3DImpl::f2dGraphics3DImpl(f2dRenderDeviceImpl* pParent, f2dEffectImpl* pEffect)
	: f2dGraphicsImpl(pParent), m_pCurEffect(NULL), m_bEffectStateDirty(true)
{
	SetEffect(pEffect);
}

f2dGraphics3DImpl::~f2dGraphics3DImpl()
{
	FCYSAFEKILL(m_pCurEffect);
}

fResult f2dGraphics3DImpl::Begin()
{
	if(m_pCurEffect == NULL)
		return FCYERR_ILLEGAL;
	fResult tRet = f2dGraphicsImpl::Begin();
	if(FAILED(tRet))
		return tRet;

	if(FAILED(((IDirect3DDevice9*)m_pParent->GetHandle())->BeginScene()))
	{
		f2dGraphicsImpl::End();
		return FCYERR_INTERNALERR;
	}

	// ×Ô¶¯ÉèÖÃäÖÈ¾×´Ì¬
	m_pCurEffect->SyncState(this);
	m_bEffectStateDirty = false;

	if(FAILED(((ID3DXEffect*)m_pCurEffect->GetHandle())->Begin(NULL, 0)))
	{
		f2dGraphicsImpl::End();
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dGraphics3DImpl::End()
{
	fResult tRet = f2dGraphicsImpl::End();
	if(FAILED(tRet))
		return tRet;

	((ID3DXEffect*)m_pCurEffect->GetHandle())->End();

	if(FAILED(((IDirect3DDevice9*)m_pParent->GetHandle())->EndScene()))
	{
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}

f2dEffect* f2dGraphics3DImpl::GetEffect()
{
	return m_pCurEffect;
}

fResult f2dGraphics3DImpl::SetEffect(f2dEffect* Effect)
{
	if(IsInRender())
		return FCYERR_ILLEGAL;
	if(m_pCurEffect == Effect)
		return FCYERR_OK;

	FCYSAFEKILL(m_pCurEffect);
	m_pCurEffect = (f2dEffectImpl*)Effect;
	m_pCurEffect->AddRef();

	return FCYERR_OK;
}

fResult f2dGraphics3DImpl::BeginPass(fuInt PassIndex)
{
	if(!IsInRender())
		return FCYERR_ILLEGAL;

	// ¼ì²é×´Ì¬
	if(m_bEffectStateDirty)
	{
		// ×Ô¶¯ÉèÖÃäÖÈ¾×´Ì¬
		m_pCurEffect->SyncState(this);
		m_bEffectStateDirty = false;
	}

	if(FAILED(((ID3DXEffect*)m_pCurEffect->GetHandle())->BeginPass(PassIndex)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult f2dGraphics3DImpl::EndPass()
{
	if(FAILED(((ID3DXEffect*)m_pCurEffect->GetHandle())->EndPass()))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult f2dGraphics3DImpl::CommitState()
{
	// ¼ì²é×´Ì¬
	if(m_bEffectStateDirty)
	{
		// ×Ô¶¯ÉèÖÃäÖÈ¾×´Ì¬
		m_pCurEffect->SyncState(this);
		m_bEffectStateDirty = false;
	}

	if(FAILED(((ID3DXEffect*)m_pCurEffect->GetHandle())->CommitChanges()))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult f2dGraphics3DImpl::RenderPostEffect()
{
	if(!IsInRender())
		return FCYERR_ILLEGAL;

	// ×¼±¸¶¥µã
	float tWidth = (float)m_pParent->GetBufferWidth();
	float tHeight = (float)m_pParent->GetBufferHeight();
	float tVPWidthFixed = 0.5f / tWidth;
	float tVPHeightFixed = 0.5f / tHeight;
	struct VertType
	{
		fcyVec4 Pos;
		fcyVec2 Tex;
	} tVertexArr[4] = 
	{
		{ fcyVec4(0.f, 0.f, 0.f, 1.f), fcyVec2(0.f + tVPWidthFixed, 0.f + tVPHeightFixed) },
		{ fcyVec4(tWidth, 0.f, 0.f, 1.f), fcyVec2(1.f + tVPWidthFixed, 0.f + tVPHeightFixed) },
		{ fcyVec4(tWidth, tHeight, 0.f, 1.f), fcyVec2(1.f + tVPWidthFixed, 1.f + tVPHeightFixed) },
		{ fcyVec4(0.f, tHeight, 0.f, 1.f), fcyVec2(0.f + tVPWidthFixed, 1.f + tVPHeightFixed) }
	};

	m_pParent->SubmitVD(NULL);
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();
	
	pDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	pDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, tVertexArr, sizeof(VertType));

	return FCYERR_OK;
}
