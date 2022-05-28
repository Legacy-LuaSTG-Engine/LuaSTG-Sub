#include "Engine/f2dEngineImpl.h"
#include "f2d.h"

f2dEngineImpl::f2dEngineImpl(f2dEngineRenderWindowParam* RenderWindowParam)
	: m_pWindow(NULL), m_pSoundSys(NULL), m_pRenderer(NULL)
{
	m_appmodel = dynamic_cast<LuaSTG::Core::ApplicationModel_Win32*>((LuaSTG::Core::IApplicationModel*)RenderWindowParam->appmodel);

	// 初始化部件
	try
	{
		auto rc = fcyRect(0.0f, 0.0f, (fFloat)RenderWindowParam->mode.width, (fFloat)RenderWindowParam->mode.height);
		auto style = RenderWindowParam->windowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE;
		m_pWindow = new f2dWindowImpl(this, rc, RenderWindowParam->title, false, style);
		m_pRenderer = new f2dRendererImpl(this, RenderWindowParam);
		m_pSoundSys = new f2dSoundSysImpl(this);
	}
	catch(...)
	{
		FCYSAFEKILL(m_pRenderer);
		FCYSAFEKILL(m_pSoundSys);
		FCYSAFEKILL(m_pWindow);

		throw;
	}
}
f2dEngineImpl::~f2dEngineImpl()
{
	// 销毁组件
	FCYSAFEKILL(m_pRenderer);
	FCYSAFEKILL(m_pSoundSys);
	FCYSAFEKILL(m_pWindow);
}

fResult f2dEngineImpl::InitWindow(const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border)
{
	if(m_pWindow)
		return FCYERR_ILLEGAL;

	try
	{
		m_pWindow = new f2dWindowImpl(this, Pos, Title, Visiable, Border);
	}
	catch(const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dEngineImpl::InitSoundSys()
{
	if(m_pSoundSys || !m_pWindow)
		return FCYERR_ILLEGAL;

	try
	{
		m_pSoundSys = new f2dSoundSysImpl(this);
	}
	catch(const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dEngineImpl::InitRenderer(f2dEngineRenderWindowParam* RenderWindowParam)
{
	if(m_pRenderer || !m_pWindow)
		return FCYERR_ILLEGAL;
	
	try
	{
		m_pRenderer = new f2dRendererImpl(this, RenderWindowParam);
	}
	catch(const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
}

F2DEXTERNC fResult F2DDLLFUNC CreateF2DEngineAndInit(fuInt Version, f2dEngineRenderWindowParam* RenderWindowParam, f2dEngine** pOut, f2dInitialErrListener* pErrListener)
{
	if(!pOut)
		return FCYERR_ILLEGAL;
	*pOut = NULL;

	if(Version != F2DVERSION)
		return FCYERR_INVAILDVERSION;
	
	try
	{
		*pOut = new f2dEngineImpl(RenderWindowParam);
	}
	catch(const fcyException& e)
	{
		if(pErrListener)
			pErrListener->OnErr(e.GetTime(), e.GetSrc(), e.GetDesc());

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
