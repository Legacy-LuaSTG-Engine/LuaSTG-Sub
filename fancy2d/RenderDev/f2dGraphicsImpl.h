////////////////////////////////////////////////////////////////////////////////
/// @file  f2dGraphicsImpl.h
/// @brief fancy2D渲染器实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "RenderDev/f2dRenderDeviceAPI.h"

#include "RenderDev/f2dRenderDeviceImpl.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 渲染器部分实现模板
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class f2dGraphicsImpl :
	public fcyRefObjImpl<T>
{
protected:
	f2dRenderDeviceImpl* m_pParent;
	
	bool m_bInRender;
	bool m_bStateDirty;

	fcyMatrix4 m_WorldMat;
	fcyMatrix4 m_LookatMat;
	fcyMatrix4 m_ProjMat;

	f2dBlendState m_BlendState;
public: // 接口实现
	fBool IsInRender()
	{
		return m_bInRender;
	}
	virtual fResult Begin()
	{
		if(m_bInRender)
			return FCYERR_ILLEGAL;

		m_pParent->SubmitCurGraphics(this, m_bStateDirty);
		m_bStateDirty = false;
		m_bInRender = true;

		return FCYERR_OK;
	}
	virtual fResult End()
	{
		if(!m_bInRender)
			return FCYERR_ILLEGAL;

		m_bInRender = false;

		return FCYERR_OK;
	}
	virtual fResult Flush() { return FCYERR_OK; }
	const fcyMatrix4& GetWorldTransform()
	{
		return m_WorldMat;
	}
	const fcyMatrix4& GetViewTransform()
	{
		return m_LookatMat;
	}
	const fcyMatrix4& GetProjTransform()
	{
		return m_ProjMat;
	}
	void SetWorldTransform(const fcyMatrix4& Mat)
	{
		m_WorldMat = Mat;

		if(IsInRender())
		{
			Flush();
			m_pParent->SubmitWorldMat(Mat);
		}
		else
			m_bStateDirty = true;
	}
	void SetViewTransform(const fcyMatrix4& Mat)
	{
		m_LookatMat = Mat;
		
		if(IsInRender())
		{
			Flush();
			m_pParent->SubmitLookatMat(Mat);
		}
		else
			m_bStateDirty = true;
	}
	void SetProjTransform(const fcyMatrix4& Mat)
	{
		m_ProjMat = Mat;
		
		if(IsInRender())
		{
			Flush();
			m_pParent->SubmitProjMat(Mat);
		}
		else
			m_bStateDirty = true;
	}
	const f2dBlendState& GetBlendState()
	{
		return m_BlendState;
	}
	void SetBlendState(const f2dBlendState& State)
	{
		m_BlendState = State;

		if(IsInRender())
		{
			Flush();
			m_pParent->SubmitBlendState(State);
		}
		else
			m_bStateDirty = true;
	}
public:
	f2dGraphicsImpl(f2dRenderDeviceImpl* pParent)
		: m_pParent(pParent), m_bStateDirty(true), m_bInRender(false) 
	{
		m_BlendState.BlendOp = F2DBLENDOPERATOR_ADD;
		m_BlendState.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
		m_BlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
		m_BlendState.AlphaBlendOp = F2DBLENDOPERATOR_ADD;
		m_BlendState.AlphaSrcBlend = F2DBLENDFACTOR_SRCALPHA;
		m_BlendState.AlphaDestBlend = F2DBLENDFACTOR_INVSRCALPHA;
	}
	virtual ~f2dGraphicsImpl()
	{
		if(m_pParent->QueryCurGraphics() == this)
			m_pParent->SubmitCurGraphics(NULL, true);
	}
};
