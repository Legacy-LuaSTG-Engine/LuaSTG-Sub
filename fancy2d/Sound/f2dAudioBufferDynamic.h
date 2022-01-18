////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundBufferDynamic.h
/// @brief fancy2D音频系统 动态缓冲区
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"
#include "fcyOS/fcyMultiThread.h"

#include "f2dSoundSys.h"
#include "Common/f2dStandardCommon.hpp"
#include "Common/f2dWindowsCommon.h"

#include <dsound.h>

// 通过不断解码播放音频
class f2dAudioBufferDynamic
	: public fcyRefObjImpl<f2dSoundBuffer>
	, public IXAudio2VoiceCallback
{
	friend class f2dSoundSysImpl;
protected:
	enum class State
	{
		Stop,
		Pause,
		Play,
	};
	State source_state = State::Stop;
	f2dSoundSys* m_pSoundSys = nullptr;
	f2dSoundDecoder* m_pDecoder;
	IXAudio2SourceVoice* xa2_source = NULL;
	Microsoft::WRL::Wrappers::Event event_start;
	Microsoft::WRL::Wrappers::Event event_stop;
	Microsoft::WRL::Wrappers::Event event_reset;
	Microsoft::WRL::Wrappers::Event event_buf1;
	Microsoft::WRL::Wrappers::Event event_buf2;
	Microsoft::WRL::Wrappers::Event event_exit;
	Microsoft::WRL::Wrappers::HandleT<Microsoft::WRL::Wrappers::HandleTraits::HANDLENullTraits> working_thread;
	fDouble start_time = 0.0;
	fDouble total_time = 0.0;
	fDouble current_time = 0.0;
	CRITICAL_SECTION start_time_lock;
public:
	void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired);
	void WINAPI OnVoiceProcessingPassEnd();
	void WINAPI OnStreamEnd();
	void WINAPI OnBufferStart(void* pBufferContext);
	void WINAPI OnBufferEnd(void* pBufferContext);
	void WINAPI OnLoopEnd(void* pBufferContext);
	void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error);
private:
	static DWORD WINAPI WorkingThread(LPVOID lpThreadParameter);
public:
	// 接口实现

	fBool IsDynamic();

	void Play();
	void Stop();
	void Pause();
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

	// 应该废弃的方法

	fBool IsLoop();
	void SetLoop(fBool bValue);
protected: // 禁止直接new/delete
	f2dAudioBufferDynamic(f2dSoundSys* pSoundSys, f2dSoundDecoder* pDecoder, fBool bGlobalFocus);
	~f2dAudioBufferDynamic();
};
