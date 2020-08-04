////////////////////////////////////////////////////////////////////////////////
/// @file  f2dVideoImpl.h
/// @brief fancy2D视频对象实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Video/f2dVideoImpl.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D视频系统实现
////////////////////////////////////////////////////////////////////////////////
class f2dVideoSysImpl :
	public fcyRefObjImpl<f2dVideoSys>
{
	friend class f2dVideoImpl;
private:
	f2dEngineImpl* m_pEngine;

	f2dRenderDevice* m_pDev;
	std::vector<f2dVideoFilterInfo> m_FliterGUIDList;
private:
	GUID strToGUID(fcStrW pGUIDStr);
	static BOOL createInstanceFromDll(HINSTANCE hDll, REFCLSID rclsid, REFIID riid, LPVOID * ppv);
public:
	f2dRenderDevice* GetRenderDevice();
public: // 接口实现
	fResult LoadFilter(fcStrW pGUIDStr);
	fResult LoadFilterFromAX(fcStrW AXFilePath, fcStrW pGUIDStr);
	fResult CreateVideo(f2dStream* pStream, fuInt Flag, f2dVideo** pOut);
public:
	f2dVideoSysImpl(f2dEngineImpl* pEngine);
	~f2dVideoSysImpl();
};
