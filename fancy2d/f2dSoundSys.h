////////////////////////////////////////////////////////////////////////////////
/// @file  f2dSoundSys.h
/// @brief fancy2D声音系统接口定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

/// @addtogroup f2d声音系统
/// @brief fancy2d引擎声音系统
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief   音频解码器接口
/// @warning 一个解码器只能用于一个音频，但是一个流可以被用来创建多个解码器
////////////////////////////////////////////////////////////////////////////////
struct f2dSoundDecoder : public f2dInterface
{
	// --- 返回PCM数据格式 ---
	virtual fuInt GetBufferSize()=0;        ///< @brief  返回解码后PCM总数据大小
	virtual fuInt GetAvgBytesPerSec()=0;    ///< @brief  获得每秒占用的字节数
	virtual fuShort GetBlockAlign()=0;      ///< @brief  获得单个采样大小
	virtual fuShort GetChannelCount()=0;    ///< @brief  获得声道数
	                                        ///< @return 返回1（单声道）或2（立体声）
	virtual fuInt GetSamplesPerSec()=0;     ///< @brief  获得采样率
	                                        ///< @return 返回如44100
	virtual fuShort GetFormatTag()=0;       ///< @brief  获得格式类型
	                                        ///< @return 通常返回1
	virtual fuShort GetBitsPerSample()=0;   ///< @brief  获得比特率
	                                        ///< @return 返回如16（bits）

	// --- 解码操作 ---
	/// @brief 返回相对PCM数据的当前解码位置
	virtual fLen GetPosition()=0;

	/// @brief     相对PCM数据进行寻址
	/// @param[in] Origin 寻址原点
	/// @param[in] Offset 偏移量
	virtual fResult SetPosition(F2DSEEKORIGIN Origin, fInt Offset)=0;

	/// @brief      解码PCM数据
	/// @param[out] pBuffer    目的缓冲区
	/// @param[in]  SizeToRead 要读取的PCM数据大小
	/// @param[out] pSizeRead  实际读取的PCM数据大小，可为NULL
	virtual fResult Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 声音缓冲区
////////////////////////////////////////////////////////////////////////////////
struct f2dSoundBuffer : public f2dInterface
{
	virtual fBool IsDynamic()=0;          ///< @brief 是否为动态缓冲区

	// --- 播放操作 ---
	virtual void Play()=0;                ///< @brief 播放
	virtual void Stop()=0;                ///< @brief 停止
	virtual void Pause()=0;               ///< @brief 暂停

	// --- 属性控制 ---
	virtual fBool IsLoop()=0;                    ///< @brief 返回是否循环
	virtual void SetLoop(fBool bValue)=0;        ///< @brief 设为循环
	virtual fBool IsPlaying()=0;                 ///< @brief 返回是否正在播放
	virtual fDouble GetTotalTime()=0;            ///< @brief 以秒为单位返回总播放时间
	virtual fDouble GetTime()=0;                 ///< @brief 以秒为单位返回播放位置
	virtual fResult SetTime(fDouble Time)=0;     ///< @brief     以秒为单位设置播放位置
		                                         ///< @param[in] Time 播放位置
	virtual fFloat GetVolume()=0;                ///< @brief  获得音量
	                                             ///< @return 返回[0~1]，0表示无声，1表示正常音量
	virtual fResult SetVolume(fFloat Value)=0;   ///< @brief     设置音量[0~1]
	                                             ///< @param[in] Value 音量[0~1]，0表示无声，1表示正常音量
	virtual fFloat GetPan()=0;                   ///< @brief  获得声道平衡
	                                             ///< @return 返回[-1~1]，-1左声道，0正常，1右声道
	virtual fResult SetPan(fFloat Value)=0;      ///< @brief     设置声道平衡
	                                             ///< @param[in] Value 平衡参数[-1~1]，-1左声道，0正常，1右声道
	virtual fuInt GetFrequency()=0;              ///< @brief  获得播放频率
	                                             ///< @return 返回每秒钟采样次数
	virtual fResult SetFrequency(fuInt Value)=0; ///< @brief     设置播放频率
	                                             ///< @param[in] Value 取值范围[100~100,000]
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 精灵音频
/// @note  用于播放多个相同音效
////////////////////////////////////////////////////////////////////////////////
struct f2dSoundSprite : public f2dInterface
{
	virtual void Play()=0;                ///< @brief 播放所有声音
	virtual void Stop()=0;                ///< @brief 停止所有声音，相当于清空
	virtual void Pause()=0;               ///< @brief 暂停所有声音
	virtual fDouble GetTotalTime()=0;     ///< @brief 获得原始声音长度

	virtual fResult PlayNewSound(fFloat Volume = 1.f, fFloat Pan = 0.f)=0; ///< @brief 播放一个声音
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 音频系统
////////////////////////////////////////////////////////////////////////////////
struct f2dSoundSys
{
	/// @brief      创建静态音频
	/// @note       静态音频将被完整加载到内存中，适合用于音效
	/// @param[in]  pDecoder     音频解码器，静态缓冲区不会保存解码器指针，仅在初始化时解码所有PCM数据
	/// @param[in]  bGlobalFocus 全局响应，设为true则当应用程序不被激活时仍然可以发出声音
	/// @param[out] pOut         输出的音频缓冲区指针
	virtual fResult CreateStaticBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut)=0;

	/// @brief      创建共享内存的静态缓冲区音频
	/// @note       共享内存缓冲区，适合重复播放的音效
	/// @param[in]  pOrg 原始的静态缓冲区
	/// @param[out] pOut 产生的共享内存区段的静态缓冲区
	virtual fResult CreateSharedStaticBuffer(f2dSoundBuffer* pOrg, f2dSoundBuffer** pOut)=0;

	/// @brief      创建动态音频
	/// @note       动态音频只会缓存极少量的PCM数据，按需解码，适合背景音乐。
	/// @param[in]  pDecoder     音频解码器，动态缓冲区会保存解码器指针，请不要将一个解码器用于多个缓冲区
	/// @param[in]  bGlobalFocus 全局响应，设为true则当应用程序不被激活时仍然可以发出声音
	/// @param[out] pOut         输出的音频缓冲区指针
	virtual fResult CreateDynamicBuffer(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, f2dSoundBuffer** pOut) = 0;

	/// @brief      创建推流器
	/// @note       推流器用于混音输出，只有Play和Pause有效，其余函数均不起作用
	/// @param[in]  pDecoder            绑定的解码器
	/// @param[in]  iBufferSampleCount  缓冲区存储的采样数
	/// @param[in]  bGlobalFocus        全局响应，设为true则当应用程序不被激活时仍然可以发出声音
	/// @param[out] pOut                输出的音频缓冲区指针
	virtual fResult CreatePullBuffer(f2dSoundDecoder* pDecoder, fuInt iBufferSampleCount, fBool bGlobalFocus, f2dSoundBuffer** pOut) = 0;

	/// @brief      创建精灵音效
	/// @note       提供音效播放功能，会把PCM数据读取到内存中
	/// @param[in]  pDecoder     音频解码器，静态缓冲区不会保存解码器指针，仅在初始化时解码所有PCM数据
	/// @param[in]  bGlobalFocus 全局响应，设为true则当应用程序不被激活时仍然可以发出声音
	/// @param[in]  iMaxCount    指定同时播放的最大音效数量, 默认可取10
	/// @param[out] pOut         输出的音频缓冲区指针
	virtual fResult CreateSoundSprite(f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt iMaxCount, f2dSoundSprite** pOut)=0;

	/// @brief      创建Wave解码器
	/// @note       解码器会对流的存取行为加锁，以保证流在多线程环境下被复用时不产生错误。
	/// @param[in]  pStream 数据流
	/// @param[out] pOut    输出的解码器指针
	virtual fResult CreateWaveDecoder(f2dStream* pStream, f2dSoundDecoder** pOut)=0;

	/// @brief      创建Vorbis解码器，用于解析Ogg音频
	/// @note       解码器会对流的存取行为加锁，以保证流在多线程环境下被复用时不产生错误。
	/// @param[in]  pStream 数据流
	/// @param[out] pOut    输出的解码器指针
	virtual fResult CreateOGGVorbisDecoder(f2dStream* pStream, f2dSoundDecoder** pOut)=0;
};

/// @}
