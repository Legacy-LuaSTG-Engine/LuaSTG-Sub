////////////////////////////////////////////////////////////////////////////////
/// @file  f2dMsgPumpImpl.h
/// @brief fancy2D 消息泵实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#include "f2dEngine.h"

#include <queue>
#include <vector>

#include <fcyOS/fcyMultiThread.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief 消息内存辅助
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class f2dMsgMemHelper : 
	public fcyRefObjImpl<f2dInterface>
{
protected:
	T m_Obj;
public:
	T& GetObj() { return m_Obj; }
public:
	f2dMsgMemHelper(const T& Org)
		: m_Obj(Org) {}
	~f2dMsgMemHelper() {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 消息实现
////////////////////////////////////////////////////////////////////////////////
class f2dMsgImpl :
	public f2dMsg
{
public:
	f2dInterface* m_pMemObj;
private:
	f2dMsgImpl& operator=(const f2dMsgImpl& Right);
public:
	f2dMsgImpl()
		: m_pMemObj(NULL) { }
	f2dMsgImpl(const f2dMsg& Msg, f2dInterface* pMemObj)
		: f2dMsg(Msg), m_pMemObj(pMemObj) 
	{
		if(m_pMemObj)
			m_pMemObj->AddRef();
	}
	f2dMsgImpl(const f2dMsgImpl& Org)
		: f2dMsg(Org), m_pMemObj(Org.m_pMemObj)
	{
		if(m_pMemObj)
			m_pMemObj->AddRef();
	}
	~f2dMsgImpl() 
	{
		FCYSAFEKILL(m_pMemObj);
	}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 消息泵实现
////////////////////////////////////////////////////////////////////////////////
class f2dMsgPumpImpl :
	public f2dMsgPump
{
protected:
	std::queue<f2dMsgImpl> m_MsgQueue;
	std::vector<f2dInterface*> m_MemObj;
public:
	void Push(const f2dMsg& Msg, f2dInterface* pMemObj)
	{
		m_MsgQueue.push(f2dMsgImpl(Msg, pMemObj));
	}
	void Clear()
	{
		while(!m_MsgQueue.empty())
		{
			FCYSAFEKILL(m_MsgQueue.front().m_pMemObj);
			m_MsgQueue.pop();
		}

		while(!m_MemObj.empty())
		{
			FCYSAFEKILL(m_MemObj.back());
			m_MemObj.pop_back();
		}
	}
public: // 接口实现
	fBool IsEmpty()
	{
		return m_MsgQueue.empty();
	}
	fResult GetMsg(f2dMsg* MsgOut)
	{
		if(IsEmpty())
			return FCYERR_OBJNOTEXSIT;
		
		f2dMsgImpl& pMsg = m_MsgQueue.front();
		if(MsgOut)
		{
			f2dMsg* MsgBase = dynamic_cast<f2dMsg*>(&pMsg);
			*MsgOut = *MsgBase;
			if(pMsg.m_pMemObj)
			{
				m_MemObj.push_back(pMsg.m_pMemObj);
				pMsg.m_pMemObj->AddRef();
			}
			m_MsgQueue.pop();
		}

		return FCYERR_OK;
	}
	fResult PeekMsg(f2dMsg* MsgOut)
	{
		if(IsEmpty())
			return FCYERR_OBJNOTEXSIT;
		
		f2dMsgImpl& pMsg = m_MsgQueue.front();
		if(MsgOut)
		{
			f2dMsg* MsgBase = dynamic_cast<f2dMsg*>(&pMsg);
			*MsgOut = *MsgBase;
		}

		return FCYERR_OK;
	}
public:
	f2dMsgPumpImpl()
	{ }
	~f2dMsgPumpImpl()
	{
		Clear();
	}
};
