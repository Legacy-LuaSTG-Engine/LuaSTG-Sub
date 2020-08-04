////////////////////////////////////////////////////////////////////////////////
/// @file  f2dVideoSys.h
/// @brief fancy2D视频播放支持
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

#include "f2dRenderer.h"

/// @addtogroup f2d视频支持
/// @brief fancy2d视频播放支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频对象状态
////////////////////////////////////////////////////////////////////////////////
enum F2DVIDEOSTATE
{
	F2DVIDEOSTATE_STOPPED,  ///< @brief 停止
	F2DVIDEOSTATE_PAUSED,   ///< @brief 暂停
	F2DVIDEOSTATE_PLAYING   ///< @brief 正在播放
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频对象初始化标识
/// @note  可以为一下标识组合
////////////////////////////////////////////////////////////////////////////////
enum F2DVIDEOFLAG
{
	F2DVIDEOFLAG_DEBUG        = 1, ///< @brief 是否允许挂接GraphEdit进行调试（WinVista以上需要注册额外的DLL）
	F2DVIDEOFLAG_NOMANAGEDTEX = 2  ///< @brief 不创建内部纹理
};

////////////////////////////////////////////////////////////////////////////////
/// @brief   fancy2D视频像素处理监听器
/// @note    该接口允许在CPU上对视频像素进行处理
/// @warning 该接口作用在多线程上
////////////////////////////////////////////////////////////////////////////////
struct f2dVideoRendererListener
{
	/// @brief 渲染一帧
	virtual void OnVideoStartRender() {}
	/// @brief 绘制一个像素
	virtual void OnVideoDrawPixel(fuInt X, fuInt Y, fcyColor* pColor) {}
	/// @brief 结束一帧渲染
	virtual void OnVideoEndRender() {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频对象
////////////////////////////////////////////////////////////////////////////////
struct f2dVideo : public f2dInterface
{
	/// @brief 获得视频渲染器监听器
	virtual f2dVideoRendererListener* GetRendererListener()=0;

	/// @brief     设置视频渲染器监听器
	/// @param[in] pListener 监听器
	virtual void SetRendererListener(f2dVideoRendererListener* pListener)=0;

	/// @brief 更新
	/// @note  调用函数用来更新内部状态，渲染纹理
	virtual fResult Update()=0;

	/// @brief 获得内部托管纹理
	/// @note  不增加引用计数
	virtual f2dTexture* GetVideoTex()=0;

	/// @brief 获得视频宽度
	virtual fuInt GetVideoWidth()=0;
	
	/// @brief 获得视频高度
	virtual fuInt GetVideoHeight()=0;

	/// @brief 获得视频渲染器数据吞吐量
	/// @note  以字节为单位
	virtual fFloat GetVideoDataRate()=0;

	/// @brief 获得视频渲染FPS
	virtual fFloat GetVideoRenderFPS()=0;

	/// @brief     复制视频数据到动态纹理
	/// @param[in] pTex 动态纹理指针
	virtual fResult CopyDataToTexture(f2dTexture2D* pTex)=0;

	/// @brief 获得状态
	virtual F2DVIDEOSTATE GetState()=0;

	/// @brief 是否循环
	virtual fBool IsLoop()=0;

	/// @brief 设置循环
	virtual void SetLoop(fBool bLoop)=0;

	/// @brief 播放
	virtual fResult Play()=0;

	/// @brief 暂停
	virtual fResult Pause()=0;

	/// @brief 停止
	virtual fResult Stop()=0;

	/// @brief 获得播放时间
	virtual fDouble GetTotalTime()=0;

	/// @brief 获得当前时间
	virtual fDouble GetCurTime()=0;

	/// @brief 获得当前时间
	virtual fResult SetCurTime(fDouble Time)=0;

	/// @breif 按照关键帧搜寻时间
	virtual fResult Seek(fuInt KeyFrame)=0;

	/// @brief 获得播放速率
	/// @note  1.0为正常速度
	virtual fDouble GetRate()=0;

	/// @brief     设置播放速率
	/// @note      1.0为正常速度
	/// @warning   不是所有的解码器都支持该操作
	/// @param[in] Rate 播放速率
	virtual fResult SetRate(fDouble Rate)=0;

	/// @brief 返回音量
	virtual fFloat  GetVolume()=0;

	/// @brief     设置音量
	/// @param[in] Volume 音量大小[0~1]
	virtual fResult SetVolume(fFloat Volume)=0;

	/// @brief 返回平衡
	virtual fFloat GetPan()=0;

	/// @brief     设置平衡
	/// @param[in] Pan 平衡值[-1~1]
	virtual fResult SetPan(fFloat Pan)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频系统
////////////////////////////////////////////////////////////////////////////////
struct f2dVideoSys
{
	/// @brief      加载过滤器到缓冲区
	/// @note       加载一个DirectShow的过滤器到缓冲区中用以解析或者解码视频
	/// @param[in]  pGUIDStr 指定DirectShow过滤器对象的GUID字符串，如"1B544C20-FD0B-11CE-8C63-00AA0044B51E"
	virtual fResult LoadFilter(fcStrW pGUIDStr)=0;

	/// @brief      从AX文件加载过滤器
	/// @param[in]  AXFilePath ax过滤器文件路径
	/// @param[in]  pGUIDStr   指定DirectShow过滤器对象的GUID字符串，如"1B544C20-FD0B-11CE-8C63-00AA0044B51E"
	virtual fResult LoadFilterFromAX(fcStrW AXFilePath, fcStrW pGUIDStr)=0;

	/// @brief      创建视频对象
	/// @param[in]  pStream 视频流
	/// @param[in]  Flag    F2DVIDEOFLAG枚举的一个或者多个组合
	/// @param[out] pOut    输出创建的视频对象指针
	virtual fResult CreateVideo(f2dStream* pStream, fuInt Flag, f2dVideo** pOut)=0;
};

/// @}
