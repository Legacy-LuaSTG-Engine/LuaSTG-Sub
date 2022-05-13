////////////////////////////////////////////////////////////////////////////////
/// @file  f2dGeometryRendererImpl.h
/// @brief fancy2D 2D形状绘制实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "f2dRenderer.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D几何体渲染器实现
////////////////////////////////////////////////////////////////////////////////
class f2dGeometryRendererImpl :
	public fcyRefObjImpl<f2dGeometryRenderer>
{
	friend class f2dRendererImpl;
private:
	fcyColor m_PenColor[3];
	fFloat m_PenSize;
	fFloat m_ZValue;

	std::vector<f2dGraphics2DVertex> m_CacheVerts;
	std::vector<fuShort> m_CacheIndex;
public:
	fFloat GetZ();
	void SetZ(fFloat ZValue);
	fcyColor GetPenColor(fuInt Index);
	void SetPenColor(fuInt Index, const fcyColor& Color);
	fFloat GetPenSize();
	void SetPenSize(fFloat Size);

	fResult DrawLine(f2dGraphics2D* pGraph, const fcyVec2& Begin, const fcyVec2& End);
	fResult DrawRectangle(f2dGraphics2D* pGraph, const fcyRect& Rect);
	fResult DrawCircle(f2dGraphics2D* pGraph, const fcyVec2& Center, float R, fuInt Divide);

	fResult FillRectangle(f2dGraphics2D* pGraph, const fcyRect& Rect, const fcyColor* pColorArr);
	fResult FillCircle(f2dGraphics2D* pGraph, const fcyVec2& Center, float R, const fcyColor& CenterColor, const fcyColor& BoundColor, fuInt Divide);
protected:
	f2dGeometryRendererImpl();
	~f2dGeometryRendererImpl();
};
