////////////////////////////////////////////////////////////////////////////////
/// @file  f2dVideoRenderer.h
/// @brief fancy2D视频渲染过滤器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Video/f2dVideoStream.h"

#include "f2dVideoSys.h"
#include "f2dRenderer.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频渲染到纹理
////////////////////////////////////////////////////////////////////////////////
class f2dVideoRenderer : 
	public CBaseVideoRenderer
{
private:
	// {598256BC-185B-4F8C-9881-DA12AB50CC74}
	static const GUID GUID_f2dVideoRenderer;
	enum SUPPORTMEDIATYPE
	{
		SUPPORTMEDIATYPE_RGB24,
		SUPPORTMEDIATYPE_RGB32,
		SUPPORTMEDIATYPE_ARGB32,
		SUPPORTMEDIATYPE_YUY2
	};
private:
	fInt m_TexWidth;                 // 纹理宽度
	fInt m_TexHeight;                // 纹理高度

	SUPPORTMEDIATYPE m_RawDataType;  // 原始格式
	fuInt m_lVideoWidth;             // 视频宽度
	fuInt m_lVideoHeight;            // 视频高度
	fuInt m_lVideoPitch;             // 视频数据补齐值

	CCritSec m_Lock;                 // 锁

	// 缓冲区
	fInt m_BufferFlag;           // 缓冲区标识，标记正在使用的缓冲区
	fcyColor* m_TexBuffer[2];    // 纹理数据数组
	CCritSec m_BufferLock[2];    // 缓冲区锁

	f2dVideoRendererListener* m_pListener; // 监听器

	// 吞吐量计算
	fcyStopWatch m_Watch;
	fFloat m_TimeTotal;
	fLen m_DataTotal;
	fLen m_FPSTotal;
	fFloat m_DataPerSecond;
	fFloat m_FPS;
private:
	FCYINLINE fByte clip255(fInt value)
	{
		if(value < 0)
			return 0;
		else if(value > 255)
			return 255;
		return (fByte)value;
	}

	void renderRGBSample(fcData pVideo, fcyColor* pTex);
	void renderYUY2Sample(fcData pVideo, fcyColor* pTex);
public:
	f2dVideoRendererListener* GetRendererListener();
	void SetRendererListener(f2dVideoRendererListener* pListener);
	fcyColor* GetInternalPtr(fuInt Index);
	fuInt GetVideoWidth();
	fuInt GetVideoHeight();
	fResult CopyDataToTexture(f2dTexture2D* pTex);
	fFloat GetDataPerSecond();
	fFloat GetVideoRenderFPS();
public: // 接口实现
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT SetMediaType(const CMediaType *pMediaType);
	HRESULT DoRenderSample(IMediaSample *pMediaSample);
public:
	f2dVideoRenderer(HRESULT *phr = NULL);
	~f2dVideoRenderer();
};
