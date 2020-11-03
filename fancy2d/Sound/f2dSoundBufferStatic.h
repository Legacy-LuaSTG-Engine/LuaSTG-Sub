////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSysAPI.h
/// @brief fancy2D音频系统 静态缓冲区
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#include "f2dSoundSys.h"

#include <dsound.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief 静态声音缓冲
////////////////////////////////////////////////////////////////////////////////
class f2dSoundBufferStatic :
	public fcyRefObjImpl<f2dSoundBuffer>
{
	friend class f2dSoundSysImpl;
protected:
	IDirectSoundBuffer* m_pBuffer; ///< @brief 声音缓冲
	
	fBool m_bLoop;      ///< @brief 是否循环
	fuInt m_BufferSize; ///< @brief 缓冲区大小
	fuInt m_psSize;     ///< @brief 一秒钟所占字节数
public: // 接口实现
	fBool IsDynamic();

	void Play();
	void Stop();
	void Pause();

	fBool IsLoop();
	void SetLoop(fBool bValue);
	fBool IsPlaying();
	fDouble GetTotalTime();
	fDouble GetTime();
	fResult SetTime(fDouble Time);
	fFloat GetVolume();
	fResult SetVolume(fFloat Value);
	fFloat GetPan();
	fResult SetPan(fFloat Value);
	fuInt GetFrequency();
	fResult SetFrequency(fuInt Value);
protected: // 禁止直接new/delete
	f2dSoundBufferStatic(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fBool bGlobalFocus);
	f2dSoundBufferStatic(IDirectSoundBuffer* pBuffer);
	~f2dSoundBufferStatic();
};
