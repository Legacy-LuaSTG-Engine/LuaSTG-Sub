#ifndef _M_ARM

#include "Video/f2dVideoRenderer.h"

#include <Dvdmedia.h>

////////////////////////////////////////////////////////////////////////////////
const GUID f2dVideoRenderer::GUID_f2dVideoRenderer = 
{ 0x598256bc, 0x185b, 0x4f8c, { 0x98, 0x81, 0xda, 0x12, 0xab, 0x50, 0xcc, 0x74 } };

f2dVideoRenderer::f2dVideoRenderer(HRESULT *phr)
	: m_TexWidth(0), m_TexHeight(0), m_RawDataType(SUPPORTMEDIATYPE_RGB24), m_lVideoWidth(0), m_lVideoHeight(0), m_lVideoPitch(0),
	m_BufferFlag(0),
	m_pListener(NULL), m_TimeTotal(0), m_DataTotal(0), m_FPSTotal(0), m_DataPerSecond(0), m_FPS(0),
	CBaseVideoRenderer(GUID_f2dVideoRenderer, L"f2dVideoRenderer", NULL, phr)
{
	m_TexBuffer[0] = m_TexBuffer[1] = NULL;
}

f2dVideoRenderer::~f2dVideoRenderer()
{
	FCYSAFEDELARR(m_TexBuffer[1]);
	FCYSAFEDELARR(m_TexBuffer[0]);
}

void f2dVideoRenderer::renderRGBSample(fcData pVideo, fcyColor* pTex)
{
	// 由于视频数据从底部向上保存，先偏移到最后一行
	pTex += (m_lVideoWidth * (m_lVideoHeight-1));

	// 拷贝视频数据
	for(fuInt y=0; y<m_lVideoHeight; y++)
	{
		fuInt SrcPos = 0;
		
		// 复制行
		for(fuInt x=0; x<m_lVideoWidth; x++) 
		{
			switch(m_RawDataType)
			{
			case SUPPORTMEDIATYPE_RGB24:
				pTex->b = pVideo[SrcPos++];  // B
				pTex->g = pVideo[SrcPos++];  // G
				pTex->r = pVideo[SrcPos++];  // R
				pTex->a = 0xFF;              // A
				break;
			case SUPPORTMEDIATYPE_RGB32:
				pTex->b = pVideo[SrcPos++];  // B
				pTex->g = pVideo[SrcPos++];  // G
				pTex->r = pVideo[SrcPos++];  // R
				pTex->a = 0xFF;              // A
				SrcPos++;
				break;
			case SUPPORTMEDIATYPE_ARGB32:
				pTex->b = pVideo[SrcPos++];  // B
				pTex->g = pVideo[SrcPos++];  // G
				pTex->r = pVideo[SrcPos++];  // R
				pTex->a = pVideo[SrcPos++];  // A
				break;
			}

			if(m_pListener)
				m_pListener->OnVideoDrawPixel(x, y, pTex);

			pTex++;
		}

		// 移动到下一行
		pVideo += m_lVideoPitch;
		pTex -= m_lVideoWidth << 1;
	}
}

void f2dVideoRenderer::renderYUY2Sample(fcData pVideo, fcyColor* pTex)
{
	fLen tDataCount = m_lVideoWidth * m_lVideoHeight / 2;

	// 拷贝并转换YUY2数据
	// B = 1.164(Y - 16) + 2.018(U - 128)
	// G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
	// R = 1.164(Y - 16) + 1.596(V - 128)
	if(m_pListener)
	{
		fuInt tX = 0, tY = 0;

		for(fLen i = 0; i<tDataCount; ++i)
		{
			// Y0 U0 Y1 V0 数据
			fByte Y0 = *(pVideo++);
			fByte U = *(pVideo++);
			fByte Y1 = *(pVideo++);
			fByte V = *(pVideo++);

			fInt tY;
			fInt tU = U - 128;
			fInt tV = V - 128;

			tY = Y0 - 16;
			pTex->b = clip255(( 298 * tY + 516 * tU            + 128) >> 8);
			pTex->g = clip255(( 298 * tY - 100 * tU - 208 * tV + 128) >> 8);
			pTex->r = clip255(( 298 * tY            + 409 * tV + 128) >> 8);
			pTex->a = 0xFF;
			
			m_pListener->OnVideoDrawPixel(tX, tY, pTex);
			tX++;
			if(tX >= m_lVideoWidth)
			{
				tX = 0;
				tY++;
			}
			pTex++;

			tY = Y1 - 16;
			pTex->b = clip255(( 298 * tY + 516 * tU            + 128) >> 8);
			pTex->g = clip255(( 298 * tY - 100 * tU - 208 * tV + 128) >> 8);
			pTex->r = clip255(( 298 * tY            + 409 * tV + 128) >> 8);
			pTex->a = 0xFF;

			m_pListener->OnVideoDrawPixel(tX, tY, pTex);
			tX++;
			if(tX >= m_lVideoWidth)
			{
				tX = 0;
				tY++;
			}
			pTex++;
		}
	}
	else
	{	
		for(fLen i = 0; i<tDataCount; ++i)
		{
			// Y0 U0 Y1 V0 数据
			fByte Y0 = *(pVideo++);
			fByte U = *(pVideo++);
			fByte Y1 = *(pVideo++);
			fByte V = *(pVideo++);

			fInt tY;
			fInt tU = U - 128;
			fInt tV = V - 128;

			tY = Y0 - 16;
			pTex->b = clip255(( 298 * tY + 516 * tU            + 128) >> 8);
			pTex->g = clip255(( 298 * tY - 100 * tU - 208 * tV + 128) >> 8);
			pTex->r = clip255(( 298 * tY            + 409 * tV + 128) >> 8);
			pTex->a = 0xFF;
			pTex++;

			tY = Y1 - 16;
			pTex->b = clip255(( 298 * tY + 516 * tU            + 128) >> 8);
			pTex->g = clip255(( 298 * tY - 100 * tU - 208 * tV + 128) >> 8);
			pTex->r = clip255(( 298 * tY            + 409 * tV + 128) >> 8);
			pTex->a = 0xFF;
			pTex++;
		}
	}
}

f2dVideoRendererListener* f2dVideoRenderer::GetRendererListener()
{
	CAutoLock tLock(&m_Lock);

	f2dVideoRendererListener* pRet = m_pListener;

	return pRet;
}

void f2dVideoRenderer::SetRendererListener(f2dVideoRendererListener* pListener)
{
	CAutoLock tLock(&m_Lock);

	m_pListener = pListener;
}

fcyColor* f2dVideoRenderer::GetInternalPtr(fuInt Index)
{
	CAutoLock tLock(&m_Lock);

	if(Index > 1)
		return NULL;
	
	fcyColor* pColor = m_TexBuffer[Index];

	return pColor;
}

fuInt f2dVideoRenderer::GetVideoWidth()
{
	CAutoLock tLock(&m_Lock);

	fuInt tRet = (fuInt)m_lVideoWidth;

	return tRet;
}

fuInt f2dVideoRenderer::GetVideoHeight()
{
	CAutoLock tLock(&m_Lock);

	fuInt tRet = (fuInt)m_lVideoHeight;

	return tRet;
}

fResult f2dVideoRenderer::CopyDataToTexture(f2dTexture2D* pTex)
{
	if(!pTex)
		return FCYERR_INVAILDPARAM;
	if(!pTex->IsDynamic() || !m_TexBuffer[0])
		return FCYERR_ILLEGAL;
	if(pTex->GetWidth() < m_lVideoWidth || pTex->GetHeight() < m_lVideoHeight)
		return FCYERR_INVAILDPARAM;

	fInt tIndex = 0;

	{
		// 锁定以获得双缓冲区属性
		m_Lock.Lock();

		tIndex = !m_BufferFlag;

		// 锁定缓冲区
		m_BufferLock[tIndex].Lock();

		m_Lock.Unlock();
	}

	fuInt tPitch = 0;
	fByte* pData = NULL;
	if(FCYFAILED(pTex->Lock(NULL, true, &tPitch, &pData)))
	{
		m_BufferLock[tIndex].Unlock();
		return FCYERR_INTERNALERR;
	}

	fcyColor* pTexData = m_TexBuffer[tIndex];
	for(fuInt Y = 0; Y<m_lVideoHeight; Y++)
	{
		for(fuInt X = 0; X<m_lVideoWidth; X++)
		{
			((fcyColor*)pData)[X] = *pTexData;

			pTexData++;
		}

		pData += tPitch;
	}

	pTex->Unlock();

	// 解锁缓冲区
	m_BufferLock[tIndex].Unlock();

	return FCYERR_OK;
}

fFloat f2dVideoRenderer::GetDataPerSecond()
{
	CAutoLock tLock(&m_Lock);

	fFloat tRet = m_DataPerSecond;

	return tRet;
}

fFloat f2dVideoRenderer::GetVideoRenderFPS()
{
	CAutoLock tLock(&m_Lock);

	fFloat tRet = m_FPS;

	return tRet;
}

HRESULT f2dVideoRenderer::CheckMediaType(const CMediaType *pMediaType)
{
	CAutoLock tLock(&m_Lock);

	if(IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video))
		if(IsEqualGUID(*pMediaType->FormatType(), FORMAT_VideoInfo))
		{
			// RGB颜色
			if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24))
				return S_OK;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB32))
				return S_OK;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_ARGB32))
				return S_OK;
			// YUV颜色
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_YUY2))
				return S_OK;
		}
		else if(IsEqualGUID(*pMediaType->FormatType(), FORMAT_VideoInfo2))
		{
			// RGB颜色
			if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24))
				return S_OK;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB32))
				return S_OK;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_ARGB32))
				return S_OK;
			// YUV颜色
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_YUY2))
				return S_OK;
		}

	return E_FAIL;
}

HRESULT f2dVideoRenderer::SetMediaType(const CMediaType *pMediaType)
{
	CAutoLock tLock(&m_Lock);

	HRESULT tHR = S_OK;
	VIDEOINFOHEADER *pVideoInfo = NULL;
	VIDEOINFOHEADER2 *pVideoInfo2 = NULL;

	if(IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video))
	{
		if(IsEqualGUID(*pMediaType->FormatType(), FORMAT_VideoInfo))
		{
			pVideoInfo = (VIDEOINFOHEADER*)pMediaType->Format();

			// RGB颜色
			if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24))
				m_RawDataType = SUPPORTMEDIATYPE_RGB24;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB32))
				m_RawDataType = SUPPORTMEDIATYPE_RGB32;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_ARGB32))
				m_RawDataType = SUPPORTMEDIATYPE_ARGB32;
			// YUV颜色
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_YUY2))
				m_RawDataType = SUPPORTMEDIATYPE_YUY2;
			else
				return E_FAIL;

			// 计算MediaType数据
			m_lVideoWidth = pVideoInfo->bmiHeader.biWidth;
			m_lVideoHeight = abs(pVideoInfo->bmiHeader.biHeight);
			if(m_RawDataType == SUPPORTMEDIATYPE_RGB24)
				m_lVideoPitch = (m_lVideoWidth * 3 + 3) & ~(3);
			else if(m_RawDataType == SUPPORTMEDIATYPE_RGB32 || m_RawDataType == SUPPORTMEDIATYPE_ARGB32)
				m_lVideoPitch = m_lVideoWidth * 4;
			else if(m_RawDataType == SUPPORTMEDIATYPE_YUY2)
				m_lVideoPitch = m_lVideoWidth * 4;
		}
		else if(IsEqualGUID(*pMediaType->FormatType(), FORMAT_VideoInfo2))
		{
			pVideoInfo2 = (VIDEOINFOHEADER2*)pMediaType->Format();

			// RGB颜色
			if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24))
				m_RawDataType = SUPPORTMEDIATYPE_RGB24;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB32))
				m_RawDataType = SUPPORTMEDIATYPE_RGB24;
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_ARGB32))
				m_RawDataType = SUPPORTMEDIATYPE_RGB24;
			// YUV颜色
			else if(IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_YUY2))
				m_RawDataType = SUPPORTMEDIATYPE_YUY2;
			else
				return E_FAIL;

			// 计算MediaType数据
			m_lVideoWidth = pVideoInfo2->bmiHeader.biWidth;
			m_lVideoHeight = abs(pVideoInfo2->bmiHeader.biHeight);
			if(m_RawDataType == SUPPORTMEDIATYPE_RGB24)
				m_lVideoPitch = (m_lVideoWidth * 3 + 3) & ~(3);
			else if(m_RawDataType == SUPPORTMEDIATYPE_RGB32 || m_RawDataType == SUPPORTMEDIATYPE_ARGB32)
				m_lVideoPitch = m_lVideoWidth * 4;
			else if(m_RawDataType == SUPPORTMEDIATYPE_YUY2)
				m_lVideoPitch = m_lVideoWidth * 4;
		}
		else
			return E_FAIL;
	}
	else
		return E_FAIL;

	// 创建纹理
	if(!m_TexBuffer[0] || (m_lVideoWidth != m_TexWidth || m_lVideoHeight != m_TexHeight))
	{
		FCYSAFEDELARR(m_TexBuffer[0]);
		FCYSAFEDELARR(m_TexBuffer[1]);

		m_TexWidth = m_lVideoWidth;
		m_TexHeight = m_lVideoHeight;
		m_TexBuffer[0] = new fcyColor[m_lVideoWidth * m_lVideoHeight];
		m_TexBuffer[1] = new fcyColor[m_lVideoWidth * m_lVideoHeight];
	}

	return S_OK;
}

HRESULT f2dVideoRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
	if(!pMediaSample || !m_TexBuffer[0])
		return E_POINTER;

	// 获得视频采样区指针
	fByte *pSamplePtr;
	pMediaSample->GetPointer(&pSamplePtr);

	if(m_pListener)
		m_pListener->OnVideoStartRender();

	// 拷贝RGB数据
	if(m_RawDataType <= SUPPORTMEDIATYPE_ARGB32)
		renderRGBSample(pSamplePtr, m_TexBuffer[m_BufferFlag]);
	// 拷贝YUY2数据
	else if(m_RawDataType == SUPPORTMEDIATYPE_YUY2)
		renderYUY2Sample(pSamplePtr, m_TexBuffer[m_BufferFlag]);

	if(m_pListener)
		m_pListener->OnVideoEndRender();

	// 更新缓冲区标识
	m_Lock.Lock();

	m_BufferFlag = !m_BufferFlag;

	// 如此时在拷贝数据则等待
	m_BufferLock[m_BufferFlag].Lock();
	m_BufferLock[m_BufferFlag].Unlock();

	m_Lock.Unlock();

	m_TimeTotal += (float)m_Watch.GetElapsed();
	m_DataTotal += pMediaSample->GetActualDataLength();
	m_FPSTotal ++;
	m_Watch.Reset();
	if(m_TimeTotal >= 1.f)
	{
		m_DataPerSecond = m_DataTotal / m_TimeTotal;
		m_FPS = m_FPSTotal / m_TimeTotal;
		m_TimeTotal = 0;
		m_DataTotal = 0;
		m_FPSTotal = 0;
	}

	return S_OK;
}

#endif
