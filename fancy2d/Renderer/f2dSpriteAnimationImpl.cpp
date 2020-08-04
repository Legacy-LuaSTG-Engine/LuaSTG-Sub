#include "Renderer/f2dSpriteAnimationImpl.h"

#include "Renderer/f2dSpriteImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dSpriteAnimationImpl::f2dSpriteAnimationImpl()
	: m_LoopStart(0), m_LoopEnd(-1), m_bLoop(false)
{}

fuInt f2dSpriteAnimationImpl::Append(fuInt FrameTime, f2dTexture2D* FrameTex)
{
	if(!FrameTex)
		return -1;

	f2dSprite* pSprite = new f2dSpriteImpl(FrameTex);
	Frame tFrame(FrameTime, pSprite);
	m_AnimationList.push_back(tFrame);
	pSprite->Release();

	return m_AnimationList.size() - 1;
}

fuInt f2dSpriteAnimationImpl::Append(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect)
{
	if(!FrameTex)
		return -1;

	f2dSprite* pSprite = new f2dSpriteImpl(FrameTex, TexRect);
	Frame tFrame(FrameTime, pSprite);
	m_AnimationList.push_back(tFrame);
	pSprite->Release();

	return m_AnimationList.size() - 1;
}

fuInt f2dSpriteAnimationImpl::Append(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, const fcyVec2& HotPos)
{
	if(!FrameTex)
		return -1;

	f2dSprite* pSprite = new f2dSpriteImpl(FrameTex, TexRect, HotPos);
	Frame tFrame(FrameTime, pSprite);
	m_AnimationList.push_back(tFrame);
	pSprite->Release();

	return m_AnimationList.size() - 1;
}

void f2dSpriteAnimationImpl::SplitAndAppend(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)
{
	float tPerSpriteWidth = TexRect.GetWidth() / (float)SplitCount;

	fcyRect tSplitRect(TexRect.a.x - tPerSpriteWidth, TexRect.a.y, TexRect.a.x, TexRect.b.y);
	for(fuInt i = 0; i<SplitCount; i++)
	{
		tSplitRect.a.x += tPerSpriteWidth;
		tSplitRect.b.x += tPerSpriteWidth;

		Append(FrameTime, FrameTex, tSplitRect);
	}
}

void f2dSpriteAnimationImpl::SplitAndAppend(fuInt* FrameTimeArr, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)
{
	float tPerSpriteWidth = TexRect.GetWidth() / (float)SplitCount;

	fcyRect tSplitRect(TexRect.a.x - tPerSpriteWidth, TexRect.a.y, TexRect.a.x, TexRect.b.y);
	for(fuInt i = 0; i<SplitCount; i++)
	{
		tSplitRect.a.x += tPerSpriteWidth;
		tSplitRect.b.x += tPerSpriteWidth;

		Append(FrameTimeArr[i], FrameTex, tSplitRect);
	}
}

void f2dSpriteAnimationImpl::ReverseSplitAndAppend(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)
{
	float tPerSpriteWidth = TexRect.GetWidth() / (float)SplitCount;

	fcyRect tSplitRect(TexRect.b.x, TexRect.a.y, TexRect.b.x + tPerSpriteWidth, TexRect.b.y);
	for(fuInt i = 0; i<SplitCount; i++)
	{
		tSplitRect.a.x -= tPerSpriteWidth;
		tSplitRect.b.x -= tPerSpriteWidth;

		Append(FrameTime, FrameTex, tSplitRect);
	}
}

void f2dSpriteAnimationImpl::ReverseSplitAndAppend(fuInt* FrameTimeArr, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)
{
	float tPerSpriteWidth = TexRect.GetWidth() / (float)SplitCount;

	fcyRect tSplitRect(TexRect.b.x, TexRect.a.y, TexRect.b.x + tPerSpriteWidth, TexRect.b.y);
	for(fuInt i = 0; i<SplitCount; i++)
	{
		tSplitRect.a.x -= tPerSpriteWidth;
		tSplitRect.b.x -= tPerSpriteWidth;

		Append(FrameTimeArr[i], FrameTex, tSplitRect);
	}
}

void f2dSpriteAnimationImpl::InitInstance(f2dSpriteAnimationInstance& Instance, bool bResetBlendColor, bool bResetFlipType)const
{
	Instance.KeyFrameIndex = 0;
	Instance.KeyFrameElapsedTime = 0;

	if(bResetFlipType)
		Instance.FlipType = F2DSPRITEFLIP_NONE;
	if(bResetBlendColor)
		Instance.BlendColor = 0xFF000000;
}

void f2dSpriteAnimationImpl::InitInstanceToEnd(f2dSpriteAnimationInstance& Instance, bool bResetBlendColor, bool bResetFlipType)const
{
	if(GetKeyFrameCount() > 0)
	{
		Instance.KeyFrameIndex = GetKeyFrameCount() - 1;
		Instance.KeyFrameElapsedTime = GetKeyFrameTime(Instance.KeyFrameIndex);
	}
	else
	{
		Instance.KeyFrameIndex = 0;
		Instance.KeyFrameElapsedTime = 0;
	}
	
	if(bResetFlipType)
		Instance.FlipType = F2DSPRITEFLIP_NONE;
	if(bResetBlendColor)
		Instance.BlendColor = 0xFF000000;
}

fBool f2dSpriteAnimationImpl::StepInstance(f2dSpriteAnimationInstance& Instance)const
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return false;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	if(Instance.KeyFrameElapsedTime >= tFrame.FrameTime)
	{
		// 继续下一帧
		// 检查是否循环并到循环尾
		if(m_bLoop)
		{
			fuInt tAbsoluteLoopStart = m_LoopStart;
			fuInt tAbsoluteLoopEnd = m_AnimationList.size() + m_LoopEnd;
			if(m_LoopStart < 0)
				tAbsoluteLoopStart += m_AnimationList.size();

			if(Instance.KeyFrameIndex == tAbsoluteLoopEnd)
			{
				// 跳到循环头
				Instance.KeyFrameIndex = tAbsoluteLoopStart;
				Instance.KeyFrameElapsedTime = 0;

				return true;
			}
		}

		// 否则继续下一帧
		if(Instance.KeyFrameIndex + 1 < m_AnimationList.size())
		{
			Instance.KeyFrameIndex++;
			Instance.KeyFrameElapsedTime = 0;
		}
		else
			return false;
	}
	else
		Instance.KeyFrameElapsedTime++;

	return true;
}

fBool f2dSpriteAnimationImpl::StepbackInstance(f2dSpriteAnimationInstance& Instance)const
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return false;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	if(Instance.KeyFrameElapsedTime == 0)
	{
		// 后退一帧
		// 检查是否循环并到循环尾
		if(m_bLoop)
		{
			fuInt tAbsoluteLoopStart = m_LoopStart;
			fuInt tAbsoluteLoopEnd = m_AnimationList.size() + m_LoopEnd;
			if(m_LoopStart < 0)
				tAbsoluteLoopStart += m_AnimationList.size();

			if(Instance.KeyFrameIndex == 0)
			{
				// 跳到循环尾
				Instance.KeyFrameIndex = tAbsoluteLoopEnd;
				Instance.KeyFrameElapsedTime = tFrame.FrameTime;

				return true;
			}
		}

		// 否则继续上一帧
		if(Instance.KeyFrameIndex > 0)
		{
			Instance.KeyFrameIndex--;
			Instance.KeyFrameElapsedTime = tFrame.FrameTime;
		}
		else
			return false;
	}
	else
		Instance.KeyFrameElapsedTime--;

	return true;
}

fResult f2dSpriteAnimationImpl::JumpTo(f2dSpriteAnimationInstance& Instance, fuInt FrameIndex)const
{
	fuInt tFrameCounter = 0;
	
	for(fuInt i = 0; i<m_AnimationList.size(); i++)
	{
		tFrameCounter += m_AnimationList[i].FrameTime;
		if(tFrameCounter > FrameIndex)
		{
			Instance.KeyFrameIndex = i;
			Instance.KeyFrameElapsedTime = m_AnimationList[i].FrameTime - (tFrameCounter - FrameIndex);
		}
	}

	if(m_AnimationList.size())
	{
		Instance.KeyFrameElapsedTime = m_AnimationList[m_AnimationList.size() - 1].FrameTime;
		Instance.KeyFrameIndex = m_AnimationList.size() - 1;
	}

	return FCYERR_OUTOFRANGE;
}

fResult f2dSpriteAnimationImpl::JumpToKeyFrame(f2dSpriteAnimationInstance& Instance, fuInt FrameIndex)const
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	Instance.KeyFrameIndex = FrameIndex;
	Instance.KeyFrameElapsedTime = 0;

	return FCYERR_OK;
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyRect& Dest)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Dest);
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyRect& Dest, const fcyRect& SubTex)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Dest, SubTex);
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Center);
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Center, Scale);
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Center, Scale, SubTex);
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Center, Scale, Rotation);
}

fResult f2dSpriteAnimationImpl::DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex)
{
	if(Instance.KeyFrameIndex >= m_AnimationList.size())
		return FCYERR_OUTOFRANGE;

	const Frame& tFrame = m_AnimationList[Instance.KeyFrameIndex];
	tFrame.FrameSprite->SetFlipType(Instance.FlipType);
	tFrame.FrameSprite->SetColor(Instance.BlendColor);
	return tFrame.FrameSprite->Draw(pGraph, Center, Scale, Rotation, SubTex);
}
