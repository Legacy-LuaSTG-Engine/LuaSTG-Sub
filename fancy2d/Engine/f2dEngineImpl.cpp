#include "Engine/f2dEngineImpl.h"

f2dEngineImpl::f2dEngineImpl()
	: m_pSoundSys(nullptr)
{
	m_pSoundSys = new f2dSoundSysImpl(this);
}
f2dEngineImpl::~f2dEngineImpl()
{
	FCYSAFEKILL(m_pSoundSys);
}

fResult CreateF2DEngineAndInit(f2dEngine** pOut)
{
	if(!pOut)
		return FCYERR_ILLEGAL;
	*pOut = nullptr;

	try
	{
		*pOut = new f2dEngineImpl();
	}
	catch(const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
