#include "Engine/f2dEngineImpl.h"

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

fResult CreateF2DEngineAndInit(f2dEngineRenderWindowParam* RenderWindowParam, f2dEngine** pOut)
{
	if(!pOut)
		return FCYERR_ILLEGAL;
	*pOut = NULL;

	try
	{
		*pOut = new f2dEngineImpl(RenderWindowParam);
	}
	catch(const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
