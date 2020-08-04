#include "Renderer/f2dGeometryRendererImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dGeometryRendererImpl::f2dGeometryRendererImpl()
	: m_ZValue(1.f), m_PenSize(1.f)
{}

f2dGeometryRendererImpl::~f2dGeometryRendererImpl()
{}

fFloat f2dGeometryRendererImpl::GetZ()
{
	return m_ZValue;
}

void f2dGeometryRendererImpl::SetZ(fFloat ZValue)
{
	m_ZValue = ZValue;
}

fcyColor f2dGeometryRendererImpl::GetPenColor(fuInt Index)
{
	if(Index >= 3)
		return 0;
	else
		return m_PenColor[Index];
}

void f2dGeometryRendererImpl::SetPenColor(fuInt Index, const fcyColor& Color)
{
	if(Index < 3)
		m_PenColor[Index] = Color;
}

fFloat f2dGeometryRendererImpl::GetPenSize()
{
	return m_PenSize;
}

void f2dGeometryRendererImpl::SetPenSize(fFloat Size)
{
	m_PenSize = Size;
}

fResult f2dGeometryRendererImpl::DrawLine(f2dGraphics2D* pGraph, const fcyVec2& Begin, const fcyVec2& End)
{
	if(!pGraph || (Begin.x == End.x && Begin.y == End.y))
		return FCYERR_INVAILDPARAM;

	// 计算方向向量
	fcyVec2 tDirect = End - Begin;
	tDirect.Normalize();
	tDirect.Rotation(-3.1415926535f/2.f);
	tDirect *= m_PenSize / 2;

	// 准备顶点
	f2dGraphics2DVertex tVerts[6] = 
	{
		{ Begin.x + tDirect.x, Begin.y + tDirect.y, m_ZValue, m_PenColor[0].argb, 0.f, 0.f },
		{ Begin.x            , Begin.y            , m_ZValue, m_PenColor[1].argb, 0.f, 0.f },
		{ Begin.x - tDirect.x, Begin.y - tDirect.y, m_ZValue, m_PenColor[2].argb, 0.f, 0.f },
		{ End.x + tDirect.x  , End.y + tDirect.y  , m_ZValue, m_PenColor[0].argb, 0.f, 0.f },
		{ End.x              , End.y              , m_ZValue, m_PenColor[1].argb, 0.f, 0.f },
		{ End.x - tDirect.x  , End.y - tDirect.y  , m_ZValue, m_PenColor[2].argb, 0.f, 0.f }
	};

	// 准备索引
	static const fuShort tIndex[12] = 
	{
		0,3,1,
		3,4,1,
		1,4,5,
		1,5,2
	};

	// 绘制
	return pGraph->DrawRaw(NULL, 6, 12, tVerts, tIndex, false);
}

fResult f2dGeometryRendererImpl::DrawRectangle(f2dGraphics2D* pGraph, const fcyRect& Rect)
{
	if(!pGraph)
		return FCYERR_INVAILDPARAM;

	// 计算扩增向量
	fcyVec2 tDirect1(-0.7071067811f, -0.7071067811f);
	fcyVec2 tDirect2(0.7071067811f, -0.7071067811f);
	tDirect1 *= m_PenSize / 2;
	tDirect2 *= m_PenSize / 2;

	// 准备顶点
	f2dGraphics2DVertex tVerts[12] = 
	{
		{ Rect.a.x + tDirect1.x, Rect.a.y + tDirect1.y, m_ZValue, m_PenColor[0].argb, 0.f, 0.f },
		{ Rect.b.x + tDirect2.x, Rect.a.y + tDirect1.y, m_ZValue, m_PenColor[0].argb, 0.f, 0.f },
		{ Rect.b.x + tDirect2.x, Rect.b.y - tDirect1.y, m_ZValue, m_PenColor[0].argb, 0.f, 0.f },
		{ Rect.a.x + tDirect1.x, Rect.b.y - tDirect1.y, m_ZValue, m_PenColor[0].argb, 0.f, 0.f },

		{ Rect.a.x               , Rect.a.y             , m_ZValue, m_PenColor[1].argb, 0.f, 0.f },
		{ Rect.b.x               , Rect.a.y             , m_ZValue, m_PenColor[1].argb, 0.f, 0.f },
		{ Rect.b.x               , Rect.b.y             , m_ZValue, m_PenColor[1].argb, 0.f, 0.f },
		{ Rect.a.x               , Rect.b.y             , m_ZValue, m_PenColor[1].argb, 0.f, 0.f },
		
		{ Rect.a.x - tDirect1.x, Rect.a.y - tDirect1.y, m_ZValue, m_PenColor[2].argb, 0.f, 0.f },
		{ Rect.b.x - tDirect2.x, Rect.a.y - tDirect1.y, m_ZValue, m_PenColor[2].argb, 0.f, 0.f },
		{ Rect.b.x - tDirect2.x, Rect.b.y + tDirect1.y, m_ZValue, m_PenColor[2].argb, 0.f, 0.f },
		{ Rect.a.x - tDirect1.x, Rect.b.y + tDirect1.y, m_ZValue, m_PenColor[2].argb, 0.f, 0.f },
	};

	// 准备索引
	static const fuShort tIndex[48] = 
	{
		0,1,4,
		1,5,4,
		1,2,5,
		5,2,6,
		7,6,2,
		3,7,2,
		0,4,7,
		0,7,3,

		4,5,9,
		4,9,8,
		9,5,6,
		9,6,10,
		11,10,6,
		11,6,7,
		4,8,11,
		4,11,7
	};

	// 绘制
	return pGraph->DrawRaw(NULL, 12, 48, tVerts, tIndex);
}

fResult f2dGeometryRendererImpl::DrawCircle(f2dGraphics2D* pGraph, const fcyVec2& Center, float R, fuInt Divide)
{
	if(!pGraph)
		return FCYERR_INVAILDPARAM;
	if(Divide<10)
		Divide = 10;

	m_CacheVerts.resize(Divide * 3);
	m_CacheIndex.resize(Divide * 12);

	float tAng = 3.14f*2.f / (float)Divide;
	for(fuInt i = 0; i<Divide; ++i)
	{
		// 计算单位向量
		fcyVec2 tDirect(cos(tAng * i), sin(tAng * i));

		// 扩增量
		fcyVec2 tExpand = tDirect * (m_PenSize / 2.f);

		// 找到中心点
		fcyVec2 tCenter = tDirect * R;

		f2dGraphics2DVertex* pVerts = &m_CacheVerts[i * 3];
		// 设置顶点
		// 外圈
		pVerts->color = m_PenColor[0].argb;
		pVerts->z = m_ZValue;
		pVerts->u = pVerts->v = 0.f;
		pVerts->x = Center.x + tCenter.x + tExpand.x;
		pVerts->y = Center.y + tCenter.y + tExpand.y;
		pVerts++;

		// 中间圈
		pVerts->color = m_PenColor[1].argb;
		pVerts->z = m_ZValue;
		pVerts->u = pVerts->v = 0.f;
		pVerts->x = Center.x + tCenter.x;
		pVerts->y = Center.y + tCenter.y;
		pVerts++;

		// 内圈
		pVerts->color = m_PenColor[2].argb;
		pVerts->z = m_ZValue;
		pVerts->u = pVerts->v = 0.f;
		pVerts->x = Center.x + tCenter.x - tExpand.x;
		pVerts->y = Center.y + tCenter.y - tExpand.y;

		// 计算Index
		fuShort* pIndex = &m_CacheIndex[i*12+0];
		if(i == Divide-1)
		{
			*pIndex = i*3;	    pIndex++;
			*pIndex = 0;	    pIndex++;
			*pIndex = i*3 + 1;	pIndex++;

			*pIndex = i*3 + 1;  pIndex++;
			*pIndex = 0;		pIndex++;
			*pIndex = 1;		pIndex++;

			*pIndex = i*3 + 1;  pIndex++;
			*pIndex = 1;		pIndex++;
			*pIndex = 2;		pIndex++;

			*pIndex = i*3 + 1;  pIndex++;
			*pIndex = 2;		pIndex++;
			*pIndex = i*3 + 2;  pIndex++;
		}
		else
		{
			*pIndex = i*3;	    pIndex++;
			*pIndex = (i+1)*3;  pIndex++;
			*pIndex = i*3 + 1;	pIndex++;

			*pIndex = i*3 + 1;      pIndex++;
			*pIndex = (i+1)*3;      pIndex++;
			*pIndex = (i+1)*3 + 1;  pIndex++;

			*pIndex = i*3 + 1;     pIndex++;
			*pIndex = (i+1)*3 + 1; pIndex++;
			*pIndex = (i+1)*3 + 2; pIndex++;

			*pIndex = i*3 + 1;     pIndex++;
			*pIndex = (i+1)*3 + 2; pIndex++;
			*pIndex = i*3 + 2;     pIndex++;
		}
	}

	// 绘制
	fResult tFR = pGraph->DrawRaw(NULL, Divide * 3, Divide * 12, &m_CacheVerts[0], &m_CacheIndex[0], false);

	return tFR;
}

fResult f2dGeometryRendererImpl::FillRectangle(f2dGraphics2D* pGraph, const fcyRect& Rect, const fcyColor* pColorArr)
{
	if(!pGraph || !pColorArr)
		return FCYERR_INVAILDPARAM;

	f2dGraphics2DVertex tVerts[4] = 
	{
		{ Rect.a.x, Rect.a.y, m_ZValue, pColorArr[0].argb, 0.f, 0.f },
		{ Rect.b.x, Rect.a.y, m_ZValue, pColorArr[1].argb, 0.f, 0.f },
		{ Rect.b.x, Rect.b.y, m_ZValue, pColorArr[2].argb, 0.f, 0.f },
		{ Rect.a.x, Rect.b.y, m_ZValue, pColorArr[3].argb, 0.f, 0.f }
	};

	return pGraph->DrawQuad(NULL, tVerts);
}

fResult f2dGeometryRendererImpl::FillCircle(f2dGraphics2D* pGraph, const fcyVec2& Center, float R, const fcyColor& CenterColor, const fcyColor& BoundColor, fuInt Divide)
{
	if(!pGraph)
		return FCYERR_INVAILDPARAM;
	if(Divide<10)
		Divide = 10;

	m_CacheVerts.resize(Divide + 1);
	m_CacheIndex.resize(Divide * 3);

	float tAng = 3.14f*2.f / (float)Divide;
	m_CacheVerts[0].color = CenterColor.argb;
	m_CacheVerts[0].x = Center.x;
	m_CacheVerts[0].y = Center.y;
	m_CacheVerts[0].u = m_CacheVerts[0].v = 0.f;
	m_CacheVerts[0].z = m_ZValue;
	for(fuInt i = 1; i<=Divide; ++i)
	{
		// 计算单位向量
		fcyVec2 tDirect(cos(tAng * i), sin(tAng * i));
		tDirect *= R;

		m_CacheVerts[i].color = BoundColor.argb;
		m_CacheVerts[i].u = m_CacheVerts[i].v = 0.f;
		m_CacheVerts[i].x = tDirect.x + Center.x;
		m_CacheVerts[i].y = tDirect.y + Center.y;
		m_CacheVerts[i].z = m_ZValue;
	}

	for(fuInt i = 0; i<Divide; ++i)
	{
		fuShort* pIndex = &m_CacheIndex[i*3];

		*pIndex = 0;   pIndex++;
		*pIndex = i+1; pIndex++;
		if(i == Divide-1)
			*pIndex = 1;
		else
			*pIndex = i+2;
	}

	// 绘制
	fResult tFR = pGraph->DrawRaw(NULL, Divide + 1, Divide * 3, &m_CacheVerts[0], &m_CacheIndex[0], false);

	return tFR;
}
