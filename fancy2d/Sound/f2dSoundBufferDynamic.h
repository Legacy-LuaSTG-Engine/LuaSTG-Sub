////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundBufferDynamic.h
/// @brief fancy2D音频系统 动态缓冲区
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"
#include "fcyOS/fcyMultiThread.h"

#include "f2dSoundSys.h"

#include <dsound.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief 动态声音缓冲
/// @note  流式缓冲，使用预先准备的64kb的空间来播放声音
////////////////////////////////////////////////////////////////////////////////
class f2dSoundBufferDynamic :
	public fcyRefObjImpl<f2dSoundBuffer>
{
	friend class f2dSoundSysImpl;
private:
	static const fuInt BufferSize;  ///< @brief 缓冲区大小为64kb
	/// @brief 流同步线程
	class SoundBufferFiller :
		public fcyBaseThread
	{
	private:
		f2dSoundBufferDynamic* m_pParent;
	private:
		fuInt ThreadJob();
	public:
		SoundBufferFiller(f2dSoundBufferDynamic* pParent);
		~SoundBufferFiller();
	};
protected:
	// 接口
	IDirectSoundBuffer* m_pBuffer;
	IDirectSoundNotify* m_pNotify;
	f2dSoundDecoder* m_pDecoder;
	// 多线程
	SoundBufferFiller* m_pFiller;  ///< @brief 填充用线程
	fcyCriticalSection m_Sec;      ///< @brief 临界区
	fcyEvent m_EvtBegin;           ///< @brief 开头填充标志点
	fcyEvent m_EvtHalf;            ///< @brief 中途填充标志点
	fcyEvent m_EvtStop;            ///< @brief 终止位置
	fBool m_bHalt;                 ///< @brief 线程终止消息
	// 播放数据
	fuInt m_StartPos;        ///< @brief 播放开始位置
	fInt  m_PlayTime;        ///< @brief 播放时间
	fBool m_bLoop;           ///< @brief 循环标志
	fuInt m_psSize;          ///< @brief 单采样大小
	fuInt m_BufferSize;      ///< @brief 缓冲大小
private:
	void preInit(fuInt StartPos);  ///< @brief 【线程安全】初始化并填充缓冲区
	void updateTime();             ///< @brief 【非线程安全】更新时间
	void fillBuffer(fuInt Index);  ///< @brief 【非线程安全】从内部缓冲区填充音频缓冲区
	void regNotify();              ///< @brief 【非线程安全】注册监听
	void regStopNotify(fuInt Pos); ///< @brief 【非线程安全】注册停止
	fBool isThreadHalt();          ///< @brief 【线程安全】判断线程退出事件
	void haltThread();             ///< @brief 【线程安全】安全终止线程
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
	f2dSoundBufferDynamic(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fBool bGlobalFocus);
	~f2dSoundBufferDynamic();
};
