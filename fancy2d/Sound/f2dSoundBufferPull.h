////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundBufferPull.h
/// @brief fancy2D音频系统 动态缓冲区推流器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"
#include "fcyOS/fcyMultiThread.h"

#include "f2dSoundSys.h"

#include <dsound.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief 基于动态声音缓冲的推流器
////////////////////////////////////////////////////////////////////////////////
class f2dSoundBufferPull :
	public fcyRefObjImpl<f2dSoundBuffer>
{
	friend class f2dSoundSysImpl;
private:
	/// @brief 流同步线程
	class SoundBufferFiller :
		public fcyBaseThread
	{
	private:
		f2dSoundBufferPull* m_pParent;
	private:
		fuInt ThreadJob();
	public:
		SoundBufferFiller(f2dSoundBufferPull* pParent);
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
	fuInt m_iBufferSize;     ///< @brief 缓冲区大小
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
	f2dSoundBufferPull(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fuInt iBufferSampleCount, fBool bGlobalFocus);
	~f2dSoundBufferPull();
};
