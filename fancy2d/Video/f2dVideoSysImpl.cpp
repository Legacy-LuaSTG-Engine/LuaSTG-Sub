#ifndef _M_ARM

#include "Video/f2dVideoSysImpl.h"

#include "Engine/f2dEngineImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dVideoSysImpl::f2dVideoSysImpl(f2dEngineImpl* pEngine)
	: m_pEngine(pEngine), m_pDev(pEngine->GetRenderer()->GetDevice())
{
	HRESULT tHR = CoInitializeEx( NULL, COINIT_MULTITHREADED );
}

f2dVideoSysImpl::~f2dVideoSysImpl()
{}

GUID f2dVideoSysImpl::strToGUID(fcStrW pGUIDStr)
{
	GUID tRet = { 0 };
	int tData[11] = { 0 };

	swscanf_s(pGUIDStr, L"%08x-%04x-%04x-%02x%02x-%2x%2x%2x%2x%2x%2x", 
		&tData[0], &tData[1], &tData[2],
		&tData[3], &tData[4],
		&tData[5], &tData[6],
		&tData[7], &tData[8],
		&tData[9], &tData[10]);

	tRet.Data1 = tData[0];
	tRet.Data2 = tData[1];
	tRet.Data3 = tData[2];

	for(int i = 3; i < 11; ++i)
		tRet.Data4[i] = tData[i];

	return tRet;
}

BOOL f2dVideoSysImpl::createInstanceFromDll(HINSTANCE hDll, REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
    typedef HRESULT (__stdcall *GETCLASS_PROC)(REFCLSID,REFIID,LPVOID*);
    
    GETCLASS_PROC procGetClassObject = (GETCLASS_PROC)GetProcAddress( hDll, "DllGetClassObject" ); 
    
    if( procGetClassObject )
    {
		IClassFactory* pFactory = NULL;
        
        HRESULT hr = procGetClassObject(rclsid, IID_IClassFactory, (void**)&pFactory);
		if(pFactory)
		{
			hr = pFactory->CreateInstance(NULL, riid, ppv);

			FCYSAFEKILL(pFactory);

			return *ppv!=NULL;
		}
    }
    
    return FALSE;
}

f2dRenderDevice* f2dVideoSysImpl::GetRenderDevice()
{
	return m_pDev;
}

fResult f2dVideoSysImpl::LoadFilter(fcStrW pGUIDStr)
{
	IBaseFilter* pFilter = NULL;
	GUID tGUID = strToGUID(pGUIDStr);

	HRESULT tHR = CoCreateInstance(tGUID, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pFilter);
	if(FAILED(tHR))
	{
		m_pEngine->ThrowException(fcyException("f2dVideoSysImpl::LoadFilter", "CoCreateInstance Failed."));
		return FCYERR_INTERNALERR;
	}

	FCYSAFEKILL(pFilter);

	for(fuInt i = 0; i<m_FliterGUIDList.size(); ++i)
		if(m_FliterGUIDList[i].FilterGUID == tGUID)
			return FCYERR_OBJEXSITED;

	f2dVideoFilterInfo tInfo = { NULL, tGUID };
	m_FliterGUIDList.push_back(tInfo);

	return FCYERR_OK;
}

fResult f2dVideoSysImpl::LoadFilterFromAX(fcStrW AXFilePath, fcStrW pGUIDStr)
{
	IBaseFilter* pFilter = NULL;
	GUID tGUID = strToGUID(pGUIDStr);

	HMODULE tDLL = LoadLibraryEx(AXFilePath, 0, LOAD_WITH_ALTERED_SEARCH_PATH);

	if(FALSE == createInstanceFromDll(tDLL, tGUID, IID_IBaseFilter, (void**)&pFilter))
	{
		m_pEngine->ThrowException(fcyException("f2dVideoSysImpl::LoadFilterFromAX", "createInstanceFromDll Failed."));
		return FCYERR_INTERNALERR;
	}

	FCYSAFEKILL(pFilter);

	for(fuInt i = 0; i<m_FliterGUIDList.size(); ++i)
		if(m_FliterGUIDList[i].FilterGUID == tGUID)
			return FCYERR_OBJEXSITED;

	f2dVideoFilterInfo tInfo = { tDLL, tGUID };
	m_FliterGUIDList.push_back(tInfo);

	return FCYERR_OK;
}

fResult f2dVideoSysImpl::CreateVideo(f2dStream* pStream, fuInt Flag, f2dVideo** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	if(!pStream)
		return FCYERR_INVAILDPARAM;

	try
	{
		*pOut = new f2dVideoImpl(this, pStream, Flag);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

#endif
