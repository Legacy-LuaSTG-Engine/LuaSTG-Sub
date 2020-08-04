////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSpriteImpl.h
/// @brief fancy2D音频系统 音效精灵
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#include "f2dSoundSys.h"

#include <dsound.h>

#include <deque>

////////////////////////////////////////////////////////////////////////////////
/// @brief 音效精灵
////////////////////////////////////////////////////////////////////////////////
class f2dSoundSpriteImpl : 
	public fcyRefObjImpl<f2dSoundSprite>
{
	friend class f2dSoundSysImpl;
private:
	IDirectSound8* m_pParent;
	IDirectSoundBuffer* m_pBuffer;                 ///< @brief 原始缓冲
	std::deque<IDirectSoundBuffer*> m_BufferList;  ///< @brief 使用中缓冲
	fuInt m_BufferSize;
	fuInt m_psSize;

	fuInt m_MaxCount;
public: // 接口实现
	void Play();
	void Stop();
	void Pause();
	fDouble GetTotalTime();

	fResult PlayNewSound(fFloat Volume, fFloat Pan);
protected: // 禁止直接new/delete
	f2dSoundSpriteImpl(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt MaxCount = 10);
	~f2dSoundSpriteImpl();
};
