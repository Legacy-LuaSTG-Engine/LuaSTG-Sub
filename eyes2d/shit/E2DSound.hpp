#pragma once
#include "fcyIO/fcyStream.h"

#ifdef ERROR
#undef ERROR
#endif // ERROR

namespace Eyes2D {
	namespace Sound {
		//音频文件解码
		struct AudioDecoder
		{
			//声道数量[典型值:2(声道)]
			virtual unsigned short GetChannels() = 0;

			//每秒采样数[典型值:44100(Hz)]
			virtual unsigned int GetSamplesPerSec() = 0;

			//每秒传输字节数[典型值:176400(byte)]
			virtual unsigned int GetAvgBytesPerSec() = 0;

			//块对齐，每次采样所采样的字节数[典型值:4(byte)]
			virtual unsigned short GetBlockAlign() = 0;

			//每个采样的位数[典型值:16(bit)=2(byte)]
			virtual unsigned short GetBitsPerSample() = 0;

			//返回指向音频数据头的指针
			virtual unsigned char* GetDataBuffer() = 0;

			//返回音频数据大小，单位为字节(byte)
			virtual unsigned int GetDataSize() = 0;
		};

		//音频状态
		enum class AudioStatus {
			PLAY,  //播放中（可信，状态来自XAudio2引擎）
			PAUSE, //暂停状态（可信，状态来自用户）
			STOP,  //停止状态（可信，状态来自用户）
			ERROR, //出错（可信，状态来自XAudio2引擎）
		};

		//音频播放器
		struct AudioPlayer {
			//播放音频
			virtual void Play() = 0;
			//停止播放，不跳转到开头
			virtual void Pause() = 0;
			//停止播放，并跳转到开头
			virtual void Stop() = 0;
			//获取状态（可信，详见SoundStatus）
			virtual AudioStatus GetStatus() = 0;
			//获取内部解码器
			virtual AudioDecoder* GetDecoder() = 0;

			//设置循环节，单位为采样，如果start和end均设置为0则不循环
			virtual void SetLoop(unsigned int start, unsigned int end, unsigned int loopcount) = 0;
			//获得循环节，单位为采样，如果start和end均为0则不循环（可信，来自用户设置）
			virtual void GetLoop(unsigned int* outstart, unsigned int* outend, unsigned int* outloopcount) = 0;

			//设置播放位置，单位为采样，并刷新缓冲区队列
			virtual bool SetTime(unsigned int sample) = 0;
			//获取播放位置，单位为采样（不可信，是计算的值）
			virtual unsigned int GetTime() = 0;
			//获取播放时间，单位为采样（可信，来自XAudio2引擎）
			virtual unsigned long long GetPlayerdTime() = 0;
			//获取音频长度，单位为采样（可信，来自解码器）
			//virtual unsigned int GetTotalTime() = 0;

			//设置播放位置，单位为秒，并刷新缓冲区队列
			virtual bool SetTimeSec(float sec) = 0;
			//获取播放位置，单位为秒（不可信，是计算的值）
			virtual float GetTimeSec() = 0;
			//获取播放时间，单位为秒（不可信，音频中途可能变速播放）
			virtual float GetPlayerdTimeSec() = 0;
			//获取音频长度，单位为秒（可信，来自解码器，但是有浮点数误差）
			//virtual float GetTotalTimeSec() = 0;

			//设置音量，归一化线性音量
			virtual void SetVolume(float volume) = 0;
			//获取音量，归一化线性音量（可信，来自XAudio2引擎）
			virtual float GetVolume() = 0;

			//设置声道平衡，归一化线性音量，取值[-1.0 ~ 1.0]，会自动转换成输出矩阵
			virtual void SetBalance(float volume) = 0;
			//获取声道平衡，归一化线性音量，值域[-1.0 ~ 1.0]（可信，来自用户设置）
			virtual float GetBalance() = 0;

			//设置播放速率，取值[0.0005 ~ 2.0000]，会影响音高
			virtual void SetSpeed(float volume) = 0;
			//获取播放速率，值域[0.0005 ~ 2.0000]（可信，来自XAudio2引擎）
			virtual float GetSpeed() = 0;

			//获取FFT变换后的结果，默认获取第一个声道的变换结果（不可信，仅用于粗略的可视化）
			//virtual bool GetFFT(float** outdata, unsigned int* outsize, unsigned int channel = 1) = 0;
		};

		//混音音轨
		struct AudioMixer {
			//获取音轨标识
			virtual unsigned GetID() = 0;
			//设主音量，归一化线性音量
			virtual bool SetVolume(float v) = 0;
			//获主音量，归一化线性音量（可信，来自XAudio2引擎）
			virtual float GetVolume() = 0;
		};

		//音频引擎
		struct AudioEngine {
			//启动音频引擎
			virtual bool Start() = 0;
			//停止音频引擎
			virtual void Stop() = 0;

			//设置主音量，归一化线性音量
			virtual bool SetVolume(float v) = 0;
			//获取主音量，归一化线性音量（可信，来自XAudio2引擎）
			virtual float GetVolume() = 0;

			//获取混音音轨
			virtual AudioMixer* GetMixer(unsigned int id) = 0;
			//获取音源音轨
			virtual AudioPlayer* GetPlayer(unsigned int id) = 0;

			//创建混音音轨
			virtual unsigned int CreateMixer(AudioMixer** out) = 0;
			//创建音源音轨
			virtual unsigned int CreatePlayer(fcyStream* stream, AudioPlayer** out) = 0;
			//移除混音音轨
			virtual bool RemoveMixer(unsigned int id) = 0;
			//移除音源音轨
			virtual bool RemovePlayer(unsigned int id) = 0;
		};
	}
}
