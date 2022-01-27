////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSysAPI.h
/// @brief fancy2D音频系统 静态缓冲区
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"
#include "f2dSoundSys.h"
#include "Common/f2dStandardCommon.hpp"
#include "Common/f2dWindowsCommon.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief 静态声音缓冲
////////////////////////////////////////////////////////////////////////////////
class f2dAudioBufferStatic
	: public fcyRefObjImpl<f2dSoundBuffer>
	, public IXAudio2VoiceCallback
{
	friend class f2dSoundSysImpl;
protected:
	f2dSoundSys* m_pSoundSys = nullptr;
	IXAudio2SourceVoice* xa2_source = NULL;
	XAUDIO2_BUFFER xa2_buffer = {};
	Microsoft::WRL::Wrappers::Event event_end;
	std::vector<BYTE> pcm_data;
	fFloat output_pan = 0.0f;
	fuInt sample_rate = 0;
	fuShort channel_cnt = 0;
	fBool is_playing = false;
public:
	void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired);
	void WINAPI OnVoiceProcessingPassEnd();
	void WINAPI OnStreamEnd();
	void WINAPI OnBufferStart(void* pBufferContext);
	void WINAPI OnBufferEnd(void* pBufferContext);
	void WINAPI OnLoopEnd(void* pBufferContext);
	void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error);
public:
	void UpdateFFT() {};
	fuInt GetFFTSize() { return 0; }
	fFloat* GetFFTData() { return nullptr; }
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
	f2dAudioBufferStatic(f2dSoundSys* pSoundSys, f2dSoundDecoder* pDecoder, fBool bGlobalFocus);
	~f2dAudioBufferStatic();
};
