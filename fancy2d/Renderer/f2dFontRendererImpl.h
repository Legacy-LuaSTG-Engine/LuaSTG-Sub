////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFontRendererImpl.h
/// @brief fancy2D 字体渲染器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "f2dRenderer.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief 字体渲染器实现
////////////////////////////////////////////////////////////////////////////////
class f2dFontRendererImpl :
	public fcyRefObjImpl<f2dFontRenderer>
{
	friend class f2dRendererImpl;
protected:
	f2dFontRendererListener* m_pListener;
	f2dFontProvider* m_pProvider;

	F2DSPRITEFLIP m_FlipType;
	fcyColor m_BlendColor[4];
	fFloat m_ZValue;
	fcyVec2 m_Scale;
protected:
	// 复制UV并翻转
	void copyAndFlipUV(const f2dGlyphInfo& Info, f2dGraphics2DVertex DestArr[]);
public: // 接口实现
	f2dFontRendererListener* GetListener();
	void SetListener(f2dFontRendererListener* pListener);

	f2dFontProvider* GetFontProvider();
	fResult SetFontProvider(f2dFontProvider* pProvider);

	fFloat GetZ();
	void SetZ(fFloat ZValue);
	fcyColor GetColor(fuInt Index);
	void SetColor(const fcyColor& Color);
	void SetColor(fuInt Index, const fcyColor& Color);
	F2DSPRITEFLIP GetFlipType();
	void SetFlipType(F2DSPRITEFLIP Type);
	fcyVec2 GetScale();
	void SetScale(fcyVec2 Scale);

	fcyRect MeasureString(fcStrW String, bool bStrictWidth);
	fFloat MeasureStringWidth(fcStrW String);

	fResult DrawTextW(f2dGraphics2D* pGraph, fcStrW Text, const fcyVec2& StartPos);
	fResult DrawTextW(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, const fcyVec2& StartPos, fcyVec2* PosOut);
	fResult DrawTextW(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, fFloat Bias, const fcyVec2& StartPos, fcyVec2* PosOut);

	fResult DrawTextW2(f2dGraphics2D* pGraph, fcStrW Text, const fcyVec2& StartPos);
	fResult DrawTextW2(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, const fcyVec2& StartPos, fcyVec2* PosOut);
	fResult DrawTextW2(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, fFloat Bias, const fcyVec2& StartPos, fcyVec2* PosOut);
protected:
	f2dFontRendererImpl(f2dFontProvider* pProvider);
	~f2dFontRendererImpl();
};
