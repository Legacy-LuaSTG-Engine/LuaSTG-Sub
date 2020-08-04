////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSpriteAnimationImpl.h
/// @brief fancy2D 精灵动画实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "f2dRenderer.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 精灵动画实现
////////////////////////////////////////////////////////////////////////////////
class f2dSpriteAnimationImpl :
	public fcyRefObjImpl<f2dSpriteAnimation>
{
	// 帧信息
	struct Frame
	{
		fuInt FrameTime;
		fcyRefPointer<f2dSprite> FrameSprite;

		Frame()
			: FrameTime(0), FrameSprite(NULL) {}
		Frame(fuInt Time, f2dSprite* Sprite)
			: FrameTime(Time), FrameSprite(Sprite) {}
		Frame(const Frame& Org)
			: FrameTime(Org.FrameTime), FrameSprite(Org.FrameSprite) {}
	};
protected:
	std::vector<Frame> m_AnimationList;
	fInt m_LoopStart;
	fInt m_LoopEnd;
	fBool m_bLoop;
public:
	f2dSprite* GetSprite(fuInt Index)
	{
		if(Index >= m_AnimationList.size())
			return NULL;
		else
			return m_AnimationList[Index].FrameSprite;
	}
	fInt GetKeyFrameTime(fuInt Index)const
	{
		if(Index >= m_AnimationList.size())
			return NULL;
		else
			return m_AnimationList[Index].FrameTime;
	}
	fResult SetSprite(fuInt Index, f2dSprite* pSprite)
	{
		if(!pSprite)
			return FCYERR_INVAILDPARAM;

		if(Index >= m_AnimationList.size())
			return FCYERR_OUTOFRANGE;
		else
		{
			m_AnimationList[Index].FrameSprite = pSprite;
			return FCYERR_OK;
		}
	}
	fResult SetKeyFrameTime(fuInt Index, fuInt Time)
	{
		if(Index >= m_AnimationList.size())
			return FCYERR_OUTOFRANGE;
		else
		{
			m_AnimationList[Index].FrameTime = Time;
			return FCYERR_OK;
		}
	}

	fuInt GetFrameCount()const
	{
		fuInt tRet = 0;

		std::vector<Frame>::const_iterator i = m_AnimationList.begin();
		while(i != m_AnimationList.end())
		{
			tRet += i->FrameTime;

			++i;
		}

		return tRet;
	}
	fuInt GetKeyFrameCount()const
	{
		return m_AnimationList.size();
	}

	fuInt Append(fuInt FrameTime, f2dTexture2D* FrameTex);
	fuInt Append(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect);
	fuInt Append(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, const fcyVec2& HotPos);
	void SplitAndAppend(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount);
	void SplitAndAppend(fuInt* FrameTimeArr, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount);
	void ReverseSplitAndAppend(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount);
	void ReverseSplitAndAppend(fuInt* FrameTimeArr, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount);
	void Clear()
	{
		m_AnimationList.clear();
	}

	fInt GetLoopStart()const { return m_LoopStart;}
	void SetLoopStart(fInt Index) { m_LoopStart = Index; }
	fInt GetLoopEnd()const { return m_LoopEnd; }
	void SetLoopEnd(fInt Index) { m_LoopEnd = Index; }
	fBool IsLoop()const { return m_bLoop; }
	void SetLoop(fBool bLoop) { m_bLoop = bLoop; }

	void InitInstance(f2dSpriteAnimationInstance& Instance, bool bResetBlendColor, bool bResetFlipType)const;
	void InitInstanceToEnd(f2dSpriteAnimationInstance& Instance, bool bResetBlendColor, bool bResetFlipType)const;
	fBool StepInstance(f2dSpriteAnimationInstance& Instance)const;
	fBool StepbackInstance(f2dSpriteAnimationInstance& Instance)const;
	fResult JumpTo(f2dSpriteAnimationInstance& Instance, fuInt FrameIndex)const;
	fResult JumpToKeyFrame(f2dSpriteAnimationInstance& Instance, fuInt FrameIndex)const;

	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyRect& Dest);
	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyRect& Dest, const fcyRect& SubTex);
	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center);
	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale);
	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex);
	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation);
	fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex);
public:
	f2dSpriteAnimationImpl();
};
