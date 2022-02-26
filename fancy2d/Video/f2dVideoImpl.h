////////////////////////////////////////////////////////////////////////////////
/// @file  f2dVideoImpl.h
/// @brief fancy2DVideo接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Video/f2dVideoStream.h"
#include "Video/f2dVideoRenderer.h"

class f2dVideoSysImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频采样器信息
////////////////////////////////////////////////////////////////////////////////
struct f2dVideoFilterInfo
{
	HINSTANCE hDLLInstance; ///< @brief DLL实例
	GUID FilterGUID;        ///< @brief 过滤器GUID
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dVideoImpl :
	public fcyRefObjImpl<f2dVideo>
{
private:
	// --- 视频流对象 ---
	f2dVideoStream* m_pStream;
	f2dVideoStreamReader* m_pReader;

	// --- DEBUG用 ---
	DWORD m_ObjectTableEntry;

	// --- DShow组件 ---
	IGraphBuilder* m_pFilterGraph;  ///< @brief 过滤器管理器
	IMediaControl* m_pMediaControl; ///< @brief 媒体控制器
	IMediaEventEx* m_pMediaEvent;   ///< @brief 媒体事件
	IMediaSeeking* m_pMediaSeek;    ///< @brief 媒体寻址
	IVideoFrameStep* m_pVideoStep;  ///< @brief 视频单步渲染控制
	IBasicAudio* m_pAudioControl;   ///< @brief 声音控制

	// --- 纹理组件 ---
	f2dVideoRenderer* m_pRenderer;
	f2dTexture2D* m_pTex;

	// --- 状态 ---
	fBool m_bLoop;
private:
	void clearUp();
	void registerDebug();
	void unregisterDebug();
private:
	fBool addFilter(const f2dVideoFilterInfo& Filter);
	void clearUselessFilter();
public: // 接口实现
	f2dVideoRendererListener* GetRendererListener();
	void SetRendererListener(f2dVideoRendererListener* pListener);

	fResult Update();
	f2dTexture* GetVideoTex();

	fuInt GetVideoWidth();
	fuInt GetVideoHeight();
	fFloat GetVideoDataRate();
	fFloat GetVideoRenderFPS();
	fResult CopyDataToTexture(f2dTexture2D* pTex);

	fBool IsLoop();
	void SetLoop(fBool bLoop);
	F2DVIDEOSTATE GetState();
	fResult Play();
	fResult Pause();
	fResult Stop();

	fDouble GetTotalTime();
	fDouble GetCurTime();
	fResult SetCurTime(fDouble Time);
	fResult Seek(fuInt KeyFrame);
	fDouble GetRate();
	fResult SetRate(fDouble Rate);
	fFloat  GetVolume();
	fResult SetVolume(fFloat Volume);
	fFloat  GetPan();
	fResult SetPan(fFloat Pan);
public:
	f2dVideoImpl(f2dVideoSysImpl* pParent, f2dStream* pStream, fuInt Flag);
	~f2dVideoImpl(void);
};
