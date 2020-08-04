#include "Renderer/f2dSpriteImpl.h"

////////////////////////////////////////////////////////////////////////////////

f2dSpriteImpl::f2dSpriteImpl(f2dTexture2D* pTex)
	: m_pTex(NULL), m_FlipType(F2DSPRITEFLIP_NONE), m_ZValue(1.f)
{
	m_pTex = pTex;
	m_pTex->AddRef();

	m_Org.b.x = (fFloat)pTex->GetWidth();
	m_Org.b.y = (fFloat)pTex->GetHeight();

	m_HotSpot = m_Org.GetCenter();

	recalcu();
}

f2dSpriteImpl::f2dSpriteImpl(f2dTexture2D* pTex, fcyRect SrcRect)
	: m_pTex(NULL), m_FlipType(F2DSPRITEFLIP_NONE), m_ZValue(1.f)
{
	m_pTex = pTex;
	m_pTex->AddRef();

	m_Org = SrcRect;

	m_HotSpot = m_Org.GetCenter();

	recalcu();
}

f2dSpriteImpl::f2dSpriteImpl(f2dTexture2D* pTex, fcyRect SrcRect, fcyVec2 HotSpot)
	: m_pTex(NULL), m_FlipType(F2DSPRITEFLIP_NONE), m_ZValue(1.f)
{
	m_pTex = pTex;
	m_pTex->AddRef();

	m_Org = SrcRect;

	m_HotSpot = HotSpot;

	recalcu();
}

f2dSpriteImpl::f2dSpriteImpl(const f2dSpriteImpl* pOrg)
	: m_pTex(pOrg->m_pTex), m_FlipType(pOrg->m_FlipType), m_Org(pOrg->m_Org), 
	m_HotSpot(pOrg->m_HotSpot), m_ZValue(pOrg->m_ZValue), m_UV(pOrg->m_UV), m_SpriteRect(pOrg->m_SpriteRect)
{
	m_Color[0] = pOrg->m_Color[0];
	m_Color[1] = pOrg->m_Color[1];
	m_Color[2] = pOrg->m_Color[2];
	m_Color[3] = pOrg->m_Color[3];

	if(m_pTex)
		m_pTex->AddRef();
}

f2dSpriteImpl::~f2dSpriteImpl()
{
	FCYSAFEKILL(m_pTex);
}

void f2dSpriteImpl::recalcu()
{
	fuInt tWidth = m_pTex->GetWidth();
	fuInt tHeight = m_pTex->GetHeight();

	// 计算UV
	switch(m_FlipType)
	{
	case F2DSPRITEFLIP_H:  // 水平
		m_UV.a.x = m_Org.b.x / tWidth;
		m_UV.a.y = m_Org.a.y / tHeight;
		m_UV.b.x = m_Org.a.x / tWidth;
		m_UV.b.y = m_Org.b.y / tHeight;
		break;
	case F2DSPRITEFLIP_V:  // 垂直
		m_UV.a.x = m_Org.a.x / tWidth;
		m_UV.a.y = m_Org.b.y / tHeight;
		m_UV.b.x = m_Org.b.x / tWidth;
		m_UV.b.y = m_Org.a.y / tHeight;
		break;
	case F2DSPRITEFLIP_HV: // 水平&垂直
		m_UV.a.x = m_Org.b.x / tWidth;
		m_UV.a.y = m_Org.b.y / tHeight;
		m_UV.b.x = m_Org.a.x / tWidth;
		m_UV.b.y = m_Org.a.y / tHeight;
		break;
	default:
		m_UV.a.x = m_Org.a.x / tWidth;
		m_UV.a.y = m_Org.a.y / tHeight;
		m_UV.b.x = m_Org.b.x / tWidth;
		m_UV.b.y = m_Org.b.y / tHeight;
		break;
	}

	// 计算参考SpriteRect
	m_SpriteRect.a = m_Org.a - m_HotSpot;
	m_SpriteRect.b = m_Org.b - m_HotSpot;
}

f2dTexture2D* f2dSpriteImpl::GetTexture()
{
	return m_pTex;
}

fResult f2dSpriteImpl::SetTexture(f2dTexture2D* pTex)
{
	if(!pTex)
		return FCYERR_INVAILDPARAM;

	f2dTexture* tOrg = m_pTex;
	m_pTex = pTex;
	m_pTex->AddRef();
	FCYSAFEKILL(tOrg);

	recalcu();

	return FCYERR_OK;
}

const fcyRect& f2dSpriteImpl::GetTexRect()const
{
	return m_Org;
}

fResult f2dSpriteImpl::SetTexRect(const fcyRect& Rect)
{
	m_Org = Rect;

	recalcu();

	return FCYERR_OK;
}

const fcyVec2& f2dSpriteImpl::GetHotSpot()const
{
	return m_HotSpot;
}

fResult f2dSpriteImpl::SetHotSpot(const fcyVec2& Point)
{
	m_HotSpot = Point;

	recalcu();

	return FCYERR_OK;
}

F2DSPRITEFLIP f2dSpriteImpl::GetFlipType()const
{
	return m_FlipType;
}

fResult f2dSpriteImpl::SetFlipType(F2DSPRITEFLIP Type)
{
	m_FlipType = Type;

	recalcu();

	return FCYERR_OK;
}

fFloat f2dSpriteImpl::GetZ()const
{
	return m_ZValue;
}

fResult f2dSpriteImpl::SetZ(fFloat Value)
{
	m_ZValue = Value;

	return FCYERR_OK;
}

fcyColor f2dSpriteImpl::GetColor(fuInt Index)const
{
	if(Index>=4)
		return 0;
	else
		return m_Color[Index];
}

void f2dSpriteImpl::GetColor(fcyColor* pOut)const
{
	if(!pOut)
		return;

	memcpy(pOut, m_Color, sizeof(fcyColor)*4);
}

fResult f2dSpriteImpl::SetColor(fcyColor Color)
{
	m_Color[0] = Color;
	m_Color[1] = Color;
	m_Color[2] = Color;
	m_Color[3] = Color;

	return FCYERR_OK;
}

fResult f2dSpriteImpl::SetColor(fuInt Index, fcyColor Color)
{
	if(Index>=4)
		return FCYERR_INVAILDPARAM;

	m_Color[Index] = Color;

	return FCYERR_OK;
}

fResult f2dSpriteImpl::SetColor(fcyColor* pArr)
{
	if(!pArr)
		return FCYERR_INVAILDPARAM;

	memcpy(m_Color, pArr, sizeof(fcyColor)*4);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyRect& Dest, fBool bAutoFixCoord)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ Dest.a.x, Dest.a.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ Dest.b.x, Dest.a.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ Dest.b.x, Dest.b.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ Dest.a.x, Dest.b.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts, bAutoFixCoord);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyRect& Dest, const fcyRect& SubTex)
{
	float tUVWidth = m_UV.b.x - m_UV.a.x;
	float tUVHeight = m_UV.b.y - m_UV.a.y;

	fcyRect tUV(
		m_UV.a.x + tUVWidth * SubTex.a.x,
		m_UV.a.y + tUVHeight * SubTex.a.y,
		m_UV.a.x + tUVWidth * SubTex.b.x,
		m_UV.a.y + tUVHeight * SubTex.b.y
		);

	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ Dest.a.x, Dest.a.y, m_ZValue, m_Color[0].argb, tUV.a.x, tUV.a.y },
		{ Dest.b.x, Dest.a.y, m_ZValue, m_Color[1].argb, tUV.b.x, tUV.a.y },
		{ Dest.b.x, Dest.b.y, m_ZValue, m_Color[2].argb, tUV.b.x, tUV.b.y },
		{ Dest.a.x, Dest.b.y, m_ZValue, m_Color[3].argb, tUV.a.x, tUV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, fcyVec3 v1, fcyVec3 v2, fcyVec3 v3, fcyVec3 v4, fBool bAutoFixCoord)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] =
	{
		{ v1.x, v1.y, v1.z, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ v2.x, v2.y, v2.z, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ v3.x, v3.y, v3.z, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ v4.x, v4.y, v4.z, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts, bAutoFixCoord);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyVec2& Center)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ m_SpriteRect.a.x + Center.x, m_SpriteRect.a.y + Center.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ m_SpriteRect.b.x + Center.x, m_SpriteRect.a.y + Center.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ m_SpriteRect.b.x + Center.x, m_SpriteRect.b.y + Center.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ m_SpriteRect.a.x + Center.x, m_SpriteRect.b.y + Center.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ m_SpriteRect.a.x * Scale.x + Center.x, m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ m_SpriteRect.a.x * Scale.x + Center.x, m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex)
{
	float tUVWidth = m_UV.b.x - m_UV.a.x;
	float tUVHeight = m_UV.b.y - m_UV.a.y;

	fcyRect tUV(
		m_UV.a.x + tUVWidth * SubTex.a.x,
		m_UV.a.y + tUVHeight * SubTex.a.y,
		m_UV.a.x + tUVWidth * SubTex.b.x,
		m_UV.a.y + tUVHeight * SubTex.b.y
		);

	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ m_SpriteRect.a.x * Scale.x + Center.x, m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[0].argb, tUV.a.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[1].argb, tUV.b.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[2].argb, tUV.b.x, tUV.b.y },
		{ m_SpriteRect.a.x * Scale.x + Center.x, m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[3].argb, tUV.a.x, tUV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, fBool bAutoFixCoord)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ m_SpriteRect.a.x * Scale.x, m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x, m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x, m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ m_SpriteRect.a.x * Scale.x, m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	float tSin = sin(Rotation);
	float tCos = cos(Rotation);

	fcyVec2* p;
	for(int i = 0; i<4; ++i)
	{
		p = (fcyVec2*)&tVerts[i];
		p->RotationSC(tSin, tCos);
		(*p) += Center;
	}

	pGraph->DrawQuad(m_pTex, tVerts, bAutoFixCoord);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex)
{
	float tUVWidth = m_UV.b.x - m_UV.a.x;
	float tUVHeight = m_UV.b.y - m_UV.a.y;

	fcyRect tUV(
		m_UV.a.x + tUVWidth * SubTex.a.x,
		m_UV.a.y + tUVHeight * SubTex.a.y,
		m_UV.a.x + tUVWidth * SubTex.b.x,
		m_UV.a.y + tUVHeight * SubTex.b.y
		);

	// 准备顶点
	f2dGraphics2DVertex tVerts[4] = 
	{
		{ m_SpriteRect.a.x * Scale.x, m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[0].argb, tUV.a.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x, m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[1].argb, tUV.b.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x, m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[2].argb, tUV.b.x, tUV.b.y },
		{ m_SpriteRect.a.x * Scale.x, m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[3].argb, tUV.a.x, tUV.b.y }
	};

	float tSin = sin(Rotation);
	float tCos = cos(Rotation);

	fcyVec2* p;
	for(int i = 0; i<4; ++i)
	{
		p = (fcyVec2*)&tVerts[i];
		p->RotationSC(tSin, tCos);
		(*p) += Center;
	}

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] =
	{
		{ m_SpriteRect.a.x + Center.x, -m_SpriteRect.a.y + Center.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ m_SpriteRect.b.x + Center.x, -m_SpriteRect.a.y + Center.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ m_SpriteRect.b.x + Center.x, -m_SpriteRect.b.y + Center.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ m_SpriteRect.a.x + Center.x, -m_SpriteRect.b.y + Center.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] =
	{
		{ m_SpriteRect.a.x * Scale.x + Center.x, -m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, -m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, -m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ m_SpriteRect.a.x * Scale.x + Center.x, -m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex)
{
	float tUVWidth = m_UV.b.x - m_UV.a.x;
	float tUVHeight = m_UV.b.y - m_UV.a.y;

	fcyRect tUV(
		m_UV.a.x + tUVWidth * SubTex.a.x,
		m_UV.a.y + tUVHeight * SubTex.a.y,
		m_UV.a.x + tUVWidth * SubTex.b.x,
		m_UV.a.y + tUVHeight * SubTex.b.y
		);

	// 准备顶点
	f2dGraphics2DVertex tVerts[4] =
	{
		{ m_SpriteRect.a.x * Scale.x + Center.x, -m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[0].argb, tUV.a.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, -m_SpriteRect.a.y * Scale.y + Center.y, m_ZValue, m_Color[1].argb, tUV.b.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x + Center.x, -m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[2].argb, tUV.b.x, tUV.b.y },
		{ m_SpriteRect.a.x * Scale.x + Center.x, -m_SpriteRect.b.y * Scale.y + Center.y, m_ZValue, m_Color[3].argb, tUV.a.x, tUV.b.y }
	};

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, fBool bAutoFixCoord)
{
	// 准备顶点
	f2dGraphics2DVertex tVerts[4] =
	{
		{ m_SpriteRect.a.x * Scale.x, -m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[0].argb, m_UV.a.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x, -m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[1].argb, m_UV.b.x, m_UV.a.y },
		{ m_SpriteRect.b.x * Scale.x, -m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[2].argb, m_UV.b.x, m_UV.b.y },
		{ m_SpriteRect.a.x * Scale.x, -m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[3].argb, m_UV.a.x, m_UV.b.y }
	};

	float tSin = sin(Rotation);
	float tCos = cos(Rotation);

	fcyVec2* p;
	for (int i = 0; i<4; ++i)
	{
		p = (fcyVec2*)&tVerts[i];
		p->RotationSC(tSin, tCos);
		(*p) += Center;
	}

	pGraph->DrawQuad(m_pTex, tVerts, bAutoFixCoord);

	return FCYERR_OK;
}

fResult f2dSpriteImpl::Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex)
{
	float tUVWidth = m_UV.b.x - m_UV.a.x;
	float tUVHeight = m_UV.b.y - m_UV.a.y;

	fcyRect tUV(
		m_UV.a.x + tUVWidth * SubTex.a.x,
		m_UV.a.y + tUVHeight * SubTex.a.y,
		m_UV.a.x + tUVWidth * SubTex.b.x,
		m_UV.a.y + tUVHeight * SubTex.b.y
		);

	// 准备顶点
	f2dGraphics2DVertex tVerts[4] =
	{
		{ m_SpriteRect.a.x * Scale.x, -m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[0].argb, tUV.a.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x, -m_SpriteRect.a.y * Scale.y, m_ZValue, m_Color[1].argb, tUV.b.x, tUV.a.y },
		{ m_SpriteRect.b.x * Scale.x, -m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[2].argb, tUV.b.x, tUV.b.y },
		{ m_SpriteRect.a.x * Scale.x, -m_SpriteRect.b.y * Scale.y, m_ZValue, m_Color[3].argb, tUV.a.x, tUV.b.y }
	};

	float tSin = sin(Rotation);
	float tCos = cos(Rotation);

	fcyVec2* p;
	for (int i = 0; i<4; ++i)
	{
		p = (fcyVec2*)&tVerts[i];
		p->RotationSC(tSin, tCos);
		(*p) += Center;
	}

	pGraph->DrawQuad(m_pTex, tVerts);

	return FCYERR_OK;
}
