////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundBufferDynamic.h
/// @brief fancy2D音频系统 动态缓冲区
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"
#include "f2dSoundSys.h"
#include "Common/f2dStandardCommon.hpp"
#include "Common/f2dWindowsCommon.h"

// 通过不断解码播放音频
class f2dAudioBufferDynamic
	: public fcyRefObjImpl<f2dSoundBuffer>
	, public IXAudio2VoiceCallback
{
	friend class f2dSoundSysImpl;
private:
	enum class ActionType
	{
		Exit,
		Stop,
		Start,
		Reset,
		SetTime,
		BufferAvailable, // 注意，严禁通过 sendAction 发出，请使用 notifyBufferAvailable
	};
	struct Action
	{
		ActionType type;
		union
		{
			struct ActionExit
			{
				uint8_t _dummy;
			} action_exit;
			struct ActionStop
			{
				uint8_t _dummy;
			} action_stop;
			struct ActionStart
			{
				uint8_t _dummy;
			} action_start;
			struct ActionReset
			{
				bool play;
			} action_reset;
			struct ActionSetTime
			{
				double time;
			} action_set_time;
			struct ActionBufferAvailable
			{
				size_t index;
			} action_buffer_available;
		};
	};
	struct ActionQueue
	{
		size_t const size = 64;
		Action data[64] = {};
		size_t writer_index = 0;
		size_t reader_index = 0;
		HANDLE semaphore_space = NULL;
		HANDLE semaphore_data = NULL;
		HANDLE event_exit = NULL;
		HANDLE event_buffer[2] = { NULL, NULL };
		
		bool createObjects()
		{
			semaphore_space = CreateSemaphoreExW(NULL, 64, 64, NULL, 0, SEMAPHORE_ALL_ACCESS);
			semaphore_data = CreateSemaphoreExW(NULL, 0, 64, NULL, 0, SEMAPHORE_ALL_ACCESS);
			if (semaphore_space == NULL || semaphore_data == NULL) return false;
			event_exit = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
			event_buffer[0] = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
			event_buffer[1] = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
			if (event_exit == NULL || event_buffer[0] == NULL || event_buffer[1] == NULL) return false;
			return true;
		}

		// 线程：任意线程
		// 通知工作线程应该退出
		void notifyExit()
		{
			SetEvent(event_exit);
		}
		// 线程：任意线程
		// 通知指定的缓冲区已经处于可用状态，可用于解码并储存 PCM 数据
		void notifyBufferAvailable(size_t i)
		{
			SetEvent(event_buffer[i]);
		}
		// 线程：仅限引擎更新线程
		// 向工作线程发送动作
		void sendAction(Action const& v)
		{
			WaitForSingleObject(semaphore_space, INFINITE); // 减少剩余空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行
			data[writer_index] = v;
			writer_index = (writer_index + 1) % size;
			ReleaseSemaphore(semaphore_data, 1, NULL); // 增加已用空间的信号量
		}
		// 线程：仅限解码线程
		// 工作线程接收动作
		void reciveAction(Action& v)
		{
			HANDLE objects[4] = { event_exit, event_buffer[0], event_buffer[1], semaphore_data };
			switch (WaitForMultipleObjects(4, objects, FALSE, INFINITE))
			{
			case WAIT_OBJECT_0 + 0:
				v.type = ActionType::Exit; // 特殊处理，且不重置 event
				break;
			case WAIT_OBJECT_0 + 1:
				ResetEvent(event_buffer[0]);
				v.type = ActionType::BufferAvailable; // 特殊处理
				v.action_buffer_available.index = 0;
				break;
			case WAIT_OBJECT_0 + 2:
				ResetEvent(event_buffer[1]);
				v.type = ActionType::BufferAvailable; // 特殊处理
				v.action_buffer_available.index = 1;
				break;
			case WAIT_OBJECT_0 + 3:
				//WaitForSingleObject(semaphore_data, INFINITE); // 减少已用空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行
				v = data[reader_index];
				reader_index = (reader_index + 1) % size;
				ReleaseSemaphore(semaphore_space, 1, NULL); // 增加剩余空间的信号量
				break;
			default:
				v.type = ActionType::Exit; // 出错了，快给爷退出
				break;
			}
		}

		ActionQueue()
		{
		}
		~ActionQueue()
		{
			writer_index = 0;
			reader_index = 0;
			if (semaphore_space) CloseHandle(semaphore_space); semaphore_space = NULL;
			if (semaphore_data) CloseHandle(semaphore_data); semaphore_data = NULL;
			if (event_exit) CloseHandle(event_exit); event_exit = NULL;
			if (event_buffer[0]) CloseHandle(event_buffer[0]); event_buffer[0] = NULL;
			if (event_buffer[1]) CloseHandle(event_buffer[1]); event_buffer[1] = NULL;
		}
	};
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
	Microsoft::WRL::Wrappers::HandleT<Microsoft::WRL::Wrappers::HandleTraits::HANDLENullTraits> working_thread;
	fDouble start_time = 0.0;
	fDouble total_time = 0.0;
	fDouble current_time = 0.0;
	fFloat output_pan = 0.0f;
	ActionQueue action_queue;
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
