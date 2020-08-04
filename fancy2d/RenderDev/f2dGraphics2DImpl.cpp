#include "RenderDev/f2dGraphics2DImpl.h"

#include "Engine/f2dEngineImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dGraphics2DImpl::f2dGraphics2DImpl(f2dRenderDeviceImpl* pParent, fuInt VertexBufferCount, fuInt IndexBufferCount)
	: f2dGraphicsImpl(pParent),
	m_pVB(NULL), m_pIB(NULL),
	m_VBMaxCount(VertexBufferCount), m_IBMaxCount(IndexBufferCount),
	m_VBUsedCount(0), m_IBUsedCount(0), m_VBAlloced(0), m_IBAlloced(0),
	m_pVBData(NULL), m_pIBData(NULL),
	m_ColorBlendType(F2DGRAPH2DBLENDTYPE_ADD)
{
	// 设置默认的投影矩阵
	SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(
		0.f, 
		(float)pParent->GetBufferWidth(), 
		(float)pParent->GetBufferHeight(), 
		0.f, 0.f, 100.f
		));

	// 创建缓存
	HRESULT tHR;
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();

	if(FAILED(tHR = pDev->CreateVertexBuffer(
		m_VBMaxCount * sizeof(f2dGraphics2DVertex),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		FVF,
		D3DPOOL_DEFAULT,
		&m_pVB,
		NULL)))
	{
		throw fcyWin32COMException("f2dGraphics2DImpl::f2dGraphics2DImpl", "IDirect3DDevice9::CreateVertexBuffer Failed.", tHR);
	}
	if(FAILED(tHR = pDev->CreateIndexBuffer(
		m_IBMaxCount * sizeof(fuShort),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, 
		&m_pIB,
		NULL)))
	{
		FCYSAFEKILL(m_pVB);
		throw fcyWin32COMException("f2dGraphics2DImpl::f2dGraphics2DImpl", "IDirect3DDevice9::CreateIndexBuffer Failed.", tHR);
	}

	// 注册监听器
	m_pParent->AttachListener(this);
}

f2dGraphics2DImpl::~f2dGraphics2DImpl()
{
	// 安全措施
	if(IsInRender())
	{
		m_pParent->GetEngine()->ThrowException(fcyException("f2dGraphics2DImpl::~f2dGraphics2DImpl", "f2dGraphics2DImpl::End is not invoked before destroy."));

		End();
	}

	FCYSAFEKILL(m_pVB);
	FCYSAFEKILL(m_pIB);

	// 移除监听器
	m_pParent->RemoveListener(this);
}

void f2dGraphics2DImpl::flush(bool Discard)
{
	if(m_Commands.empty())
	{
		if(Discard)
		{
			// 解锁缓冲区
			m_pVB->Unlock();
			m_pIB->Unlock();

			m_pVB->Lock(0, m_VBMaxCount * sizeof(f2dGraphics2DVertex), (void**)&m_pVBData, D3DLOCK_DISCARD);
			m_pIB->Lock(0, m_IBMaxCount * sizeof(fuShort), (void**)&m_pIBData, D3DLOCK_DISCARD);
			m_VBUsedCount = 0;
			m_IBUsedCount = 0;
			m_VBAlloced = 0;
			m_IBAlloced = 0;
		}
		else
			return;
	}

	// 解锁缓冲区
	m_pVB->Unlock();
	m_pIB->Unlock();

	// 绘制并统计有效顶点个数
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();
	vector<RenderCommand>::iterator i = m_Commands.begin();
	f2dTexture2D *last_tex=i->pTex;
	int last_vb = m_VBUsedCount;
	int last_id = m_IBUsedCount;


	while(i != m_Commands.end())
	{
		
		if (i->pTex != last_tex && last_vb != m_VBUsedCount && last_id != m_IBUsedCount){
			if (last_tex)
			{
				pDev->SetTexture(0, (IDirect3DBaseTexture9*)last_tex->GetHandle());
				last_tex->Release();
			}
			else
				pDev->SetTexture(0, NULL);
			pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, last_vb, 0, m_VBUsedCount - last_vb, last_id, (m_IBUsedCount - last_id) / 3);
			last_vb = m_VBUsedCount;
			last_id = m_IBUsedCount;
			last_tex = i->pTex;
		}
		
		/*
		if(i->pTex)
		{
			pDev->SetTexture(0, (IDirect3DBaseTexture9*)i->pTex->GetHandle());
			i->pTex->Release();
		}
		else
			pDev->SetTexture(0, NULL);*/

		//pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_VBUsedCount, 0, i->VertCount, m_IBUsedCount, i->IndexCount/3);

		m_VBUsedCount += i->VertCount;
		m_IBUsedCount += i->IndexCount;

		++i;
	}
	if (last_vb != m_VBUsedCount){
		if (last_tex)
		{
			pDev->SetTexture(0, (IDirect3DBaseTexture9*)last_tex->GetHandle());
			last_tex->Release();
		}
		else
			pDev->SetTexture(0, NULL);
		pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, last_vb, 0, m_VBUsedCount - last_vb, last_id, (m_IBUsedCount - last_id) / 3);
	}

	m_Commands.clear();

	// 重新锁定缓冲区
	if(m_VBUsedCount == m_VBMaxCount || m_IBUsedCount == m_IBMaxCount)
		Discard = true;

	if(Discard)
	{
		m_pVB->Lock(0, m_VBMaxCount * sizeof(f2dGraphics2DVertex), (void**)&m_pVBData, D3DLOCK_DISCARD);
		m_pIB->Lock(0, m_IBMaxCount * sizeof(fuShort), (void**)&m_pIBData, D3DLOCK_DISCARD);
		m_VBUsedCount = 0;
		m_IBUsedCount = 0;
		m_VBAlloced = 0;
		m_IBAlloced = 0;
	}
	else
	{
		m_pVB->Lock(m_VBUsedCount * sizeof(f2dGraphics2DVertex), (m_VBMaxCount - m_VBUsedCount) * sizeof(f2dGraphics2DVertex), (void**)&m_pVBData, D3DLOCK_NOOVERWRITE);
		m_pIB->Lock(m_IBUsedCount * sizeof(fuShort), (m_IBMaxCount - m_IBUsedCount) * sizeof(fuShort), (void**)&m_pIBData, D3DLOCK_NOOVERWRITE);

		m_VBAlloced = 0;
		m_IBAlloced = 0;
	}
}

bool f2dGraphics2DImpl::alloc(fuInt VertCountToAlloc, fuInt IndexCountToAlloc, f2dGraphics2DVertex** pVertexOut, fuShort** pIndexOut)
{
	if(VertCountToAlloc > m_VBMaxCount || IndexCountToAlloc > m_IBMaxCount)
		return false;

	if((m_VBMaxCount - m_VBUsedCount - m_VBAlloced) < VertCountToAlloc || (m_IBMaxCount - m_IBUsedCount - m_IBAlloced) < IndexCountToAlloc)
		flush(true);

	*pVertexOut = (m_pVBData + m_VBAlloced);
	*pIndexOut = (m_pIBData + m_IBAlloced);

	m_VBAlloced += VertCountToAlloc;
	m_IBAlloced += IndexCountToAlloc;

	return true;
}

void f2dGraphics2DImpl::pushCommand(f2dTexture2D* pTex, fuInt VertCount, fuInt IndexCount)
{
	if(!m_Commands.empty())
	{
		RenderCommand* pCmd = &m_Commands[m_Commands.size()-1];

		if(pCmd->pTex == pTex)
		{
			// 调整索引
			fuShort* pIndex = (m_pIBData + m_IBAlloced - IndexCount);
			for(fuInt i = 0; i<IndexCount; ++i)
				pIndex[i] += pCmd->VertCount;

			pCmd->VertCount += VertCount;
			pCmd->IndexCount += IndexCount;
			return;
		}
	}

	RenderCommand tCmd = { pTex, VertCount, IndexCount };
	
	if(pTex)
		pTex->AddRef();
	m_Commands.push_back(tCmd);
}

fResult f2dGraphics2DImpl::setColorBlendType(F2DGRAPH2DBLENDTYPE Type)
{
	D3DTEXTUREOP tColorOP;
	switch(Type)
	{
	case F2DGRAPH2DBLENDTYPE_DISABLE:
		tColorOP = D3DTOP_DISABLE;
		break;
	case F2DGRAPH2DBLENDTYPE_SELECTCOLOR:
		tColorOP = D3DTOP_SELECTARG1;
		break;
	case F2DGRAPH2DBLENDTYPE_SELECTTEXTURE:
		tColorOP = D3DTOP_SELECTARG2;
		break;
	case F2DGRAPH2DBLENDTYPE_ADD:
		tColorOP = D3DTOP_ADD;
		break;
	case F2DGRAPH2DBLENDTYPE_SUBTRACT:
		tColorOP = D3DTOP_SUBTRACT;
		break;
	case F2DGRAPH2DBLENDTYPE_MODULATE:
		tColorOP = D3DTOP_MODULATE;
		break;
	case F2DGRAPH2DBLENDTYPE_MODULATE2X:
		tColorOP = D3DTOP_MODULATE2X;
		break;
	case F2DGRAPH2DBLENDTYPE_MODULATE4X:
		tColorOP = D3DTOP_MODULATE4X;
		break;
	case F2DGRAPH2DBLENDTYPE_ADDSIGNED:
		tColorOP = D3DTOP_ADDSIGNED;
		break;
	case F2DGRAPH2DBLENDTYPE_ADDSIGNED2X:
		tColorOP = D3DTOP_ADDSIGNED2X;
		break;
	case F2DGRAPH2DBLENDTYPE_ADDSMOOTH:
		tColorOP = D3DTOP_ADDSMOOTH;
		break;
	case F2DGRAPH2DBLENDTYPE_ALPHATEXTURE:
		tColorOP = D3DTOP_BLENDTEXTUREALPHA;
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}

	if(FCYOK(m_pParent->SubmitTextureBlendOP_Color(tColorOP)))
		return FCYERR_OK;
	else
		return FCYERR_INTERNALERR;
}

void f2dGraphics2DImpl::OnRenderDeviceLost()
{
	FCYSAFEKILL(m_pVB);
	FCYSAFEKILL(m_pIB);
}

void f2dGraphics2DImpl::OnRenderDeviceReset()
{
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();

	pDev->CreateVertexBuffer(
		m_VBMaxCount * sizeof(f2dGraphics2DVertex),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		FVF,
		D3DPOOL_DEFAULT,
		&m_pVB,
		NULL);
	pDev->CreateIndexBuffer(
		m_IBMaxCount * sizeof(fuShort), 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, 
		D3DPOOL_DEFAULT,
		&m_pIB,
		NULL);
}

fResult f2dGraphics2DImpl::Begin()
{
	fResult tRet = f2dGraphicsImpl::Begin();
	if(tRet != FCYERR_OK)
		return tRet;

	if(!(m_pIB && m_pVB))
		return FCYERR_INTERNALERR;

	m_pParent->SubmitVD(NULL);
	setColorBlendType(m_ColorBlendType);

	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();
	pDev->SetFVF(FVF);
	pDev->SetStreamSource(0, m_pVB, 0, sizeof(f2dGraphics2DVertex));
	pDev->SetIndices(m_pIB);

	if(FAILED(pDev->BeginScene()))
	{
		m_bInRender = false;
		return FCYERR_INTERNALERR;
	}
	else
	{
		// 准备缓冲区
		m_pVB->Lock(0, m_VBMaxCount*sizeof(f2dGraphics2DVertex), (void**)&m_pVBData, D3DLOCK_DISCARD);
		m_pIB->Lock(0, m_IBMaxCount*sizeof(fuShort), (void**)&m_pIBData, D3DLOCK_DISCARD);

		m_VBUsedCount = 0;
		m_IBUsedCount = 0;
		m_VBAlloced = 0;
		m_IBAlloced = 0;

		return FCYERR_OK;
	}
}

fResult f2dGraphics2DImpl::Flush()
{
	if(!m_bInRender)
		return FCYERR_ILLEGAL;

	flush();
	return FCYERR_OK;
}

fResult f2dGraphics2DImpl::End()
{
	fResult tRet = f2dGraphicsImpl::End();
	if(tRet != FCYERR_OK)
		return tRet;

	flush();

	m_pVB->Unlock();
	m_pIB->Unlock();

	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();
	if(FAILED(pDev->EndScene()))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

F2DGRAPH2DBLENDTYPE f2dGraphics2DImpl::GetColorBlendType()
{
	return m_ColorBlendType;
}

fResult f2dGraphics2DImpl::SetColorBlendType(F2DGRAPH2DBLENDTYPE Type)
{
	if(IsInRender())
		Flush();
	if(Type == m_ColorBlendType)
		return FCYERR_OK;

	fResult tRet = setColorBlendType(Type);
	if(FCYOK(tRet))
	{
		m_ColorBlendType = Type;
	}

	return tRet;
}

fResult f2dGraphics2DImpl::DrawQuad(
	f2dTexture2D* pTex,
	const f2dGraphics2DVertex& v1, 
	const f2dGraphics2DVertex& v2, 
	const f2dGraphics2DVertex& v3, 
	const f2dGraphics2DVertex& v4,
	fBool bAutoFixCoord
	)
{
	if(!m_bInRender)
		return FCYERR_ILLEGAL;

	f2dGraphics2DVertex* pVerts;
	fuShort* pIndex;
	if(!alloc(4, 6, &pVerts, &pIndex))
		return FCYERR_INVAILDPARAM;

	pVerts[0] = v1;
	pVerts[1] = v2;
	pVerts[2] = v3;
	pVerts[3] = v4;
	
	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;
	pIndex[3] = 0;
	pIndex[4] = 2;
	pIndex[5] = 3;

	if(bAutoFixCoord)
	{
		coordFix(pVerts[0].x);
		coordFix(pVerts[1].x);
		coordFix(pVerts[2].x);
		coordFix(pVerts[3].x);
		coordFix(pVerts[0].y);
		coordFix(pVerts[1].y);
		coordFix(pVerts[2].y);
		coordFix(pVerts[3].y);
	}

	pushCommand(pTex, 4, 6);

	return FCYERR_OK;
}

fResult f2dGraphics2DImpl::DrawQuad(f2dTexture2D* pTex, const f2dGraphics2DVertex* arr, fBool bAutoFixCoord)
{
	if(!m_bInRender)
		return FCYERR_ILLEGAL;

	f2dGraphics2DVertex* pVerts;
	fuShort* pIndex;
	if(!alloc(4, 6, &pVerts, &pIndex))
		return FCYERR_INVAILDPARAM;

	memcpy(pVerts, arr, sizeof(f2dGraphics2DVertex) * 4);

	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;
	pIndex[3] = 0;
	pIndex[4] = 2;
	pIndex[5] = 3;

	if(bAutoFixCoord)
	{
		coordFix(pVerts[0].x);
		coordFix(pVerts[1].x);
		coordFix(pVerts[2].x);
		coordFix(pVerts[3].x);
		coordFix(pVerts[0].y);
		coordFix(pVerts[1].y);
		coordFix(pVerts[2].y);
		coordFix(pVerts[3].y);
	}

	pushCommand(pTex, 4, 6);

	return FCYERR_OK;
}

fResult f2dGraphics2DImpl::DrawRaw(f2dTexture2D* pTex, fuInt VertCount, fuInt IndexCount, const f2dGraphics2DVertex* VertArr, const fuShort* IndexArr, fBool bAutoFixCoord)
{
	if(!m_bInRender)
		return FCYERR_ILLEGAL;

	f2dGraphics2DVertex* pVerts;
	fuShort* pIndex;
	if(!alloc(VertCount, IndexCount, &pVerts, &pIndex))
		return FCYERR_INVAILDPARAM;

	memcpy(pVerts, VertArr, sizeof(f2dGraphics2DVertex) * VertCount);
	memcpy(pIndex, IndexArr, sizeof(fuShort) * IndexCount);

	if(bAutoFixCoord)
	{
		for(fuInt i = 0; i<VertCount; ++i)
		{
			coordFix(pVerts[i].x);
			coordFix(pVerts[i].y);
		}
	}

	pushCommand(pTex, VertCount, IndexCount);

	return FCYERR_OK;
}
