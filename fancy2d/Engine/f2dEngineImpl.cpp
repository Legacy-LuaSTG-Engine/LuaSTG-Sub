#include "Engine/f2dEngineImpl.h"
#include "f2d.h"

f2dEngineImpl::f2dEngineImpl(f2dEngineRenderWindowParam* RenderWindowParam)
	: m_pSoundSys(NULL)
	, m_pRenderer(NULL)
{
	m_appmodel = dynamic_cast<LuaSTG::Core::ApplicationModel_Win32*>((LuaSTG::Core::IApplicationModel*)RenderWindowParam->appmodel);

	// 初始化部件
	try
	{
		m_pRenderer = new f2dRendererImpl(this, RenderWindowParam);
		m_pSoundSys = new f2dSoundSysImpl(this);
	}
	catch(...)
	{
		FCYSAFEKILL(m_pRenderer);
		FCYSAFEKILL(m_pSoundSys);

		throw;
	}
}
f2dEngineImpl::~f2dEngineImpl()
{
	// 销毁组件
	FCYSAFEKILL(m_pRenderer);
	FCYSAFEKILL(m_pSoundSys);
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
