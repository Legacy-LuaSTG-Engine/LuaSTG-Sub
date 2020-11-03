////////////////////////////////////////////////////////////////////////////////
/// @file  fcyFunctor.h
/// @brief fancy调用封装
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyException.h"

/// @addtogroup fancy杂项
/// @{

template<typename T>
class fcyFunctor;

template<typename R, typename... P>
class fcyFunctor<R(P...)>
{
private:
	class fcyFunctorCallBack :
		public fcyRefObj
	{
	public:
		virtual R Invoke(P...) = 0;  // (P1,P2,P3...)
	protected:
		virtual ~fcyFunctorCallBack() {}
	};

	template<typename StaticFunc>
	class fcyFunctorCallBack_StaticFunc :
		public fcyRefObjImpl<fcyFunctorCallBack>
	{
	private:
		StaticFunc m_pFunc;
	public:
		R Invoke(P... args)  // (P1 p1, P2 p2, P3 p3...)
		{
			return m_pFunc(args...); // (p1, p2, p3...)
		}
	public:
		fcyFunctorCallBack_StaticFunc(StaticFunc Func)
			: m_pFunc(Func) {}
	};

	template<typename Class, typename MemFunc>
	class fcyFunctorCallBack_MemFunc :
		public fcyRefObjImpl<fcyFunctorCallBack>
	{
	private:
		Class* m_pClsPtr;
		MemFunc m_pClsMemFunc;
	public:
		R Invoke(P... args) // (P1 p1, P2 p2, P3 p3...)
		{
			return (m_pClsPtr->*m_pClsMemFunc)(args...); // (p1, p2, p3...)
		}
	public:
		fcyFunctorCallBack_MemFunc(Class* Ptr, MemFunc Func)
			: m_pClsPtr(Ptr), m_pClsMemFunc(Func) {}
	};
private:
	fcyFunctorCallBack* m_pCallBack;
public:
	fcyFunctor& operator=(const fcyFunctor& Right)
	{
		if (this != &Right)
		{
			FCYSAFEKILL(m_pCallBack);

			m_pCallBack = Right.m_pCallBack;

			if (m_pCallBack)
				m_pCallBack->AddRef();
		}

		return *this;
	}

	bool operator==(const fcyFunctor& Right)
	{
		return (m_pCallBack == Right.m_pCallBack);
	}

	R operator() (P... args) // (P1 p1, P2 p2, P3 p3...)
	{
		if (!m_pCallBack)
			throw fcyException("fcyFunctor::operator()", "Invalid pointer.");
		else
			return m_pCallBack->Invoke(args...); // (p1, p2, p3...)
	}
	R operator() (P... args)const // (P1 p1, P2 p2, P3 p3...)
	{
		if (!m_pCallBack)
			throw fcyException("fcyFunctor::operator()const", "Invalid pointer.");
		else
			return m_pCallBack->Invoke(args...); // (p1, p2, p3...)
	}
public:
	bool Empty()const
	{
		return (NULL == m_pCallBack);
	}
public:
	fcyFunctor()
		: m_pCallBack(NULL) { }

	template<typename StaticFunc>
	fcyFunctor(StaticFunc pFunc)
		: m_pCallBack(NULL)
	{
		m_pCallBack = new fcyFunctorCallBack_StaticFunc<StaticFunc>(pFunc);
	}

	template<typename Class, typename MemFunc>
	fcyFunctor(Class* Ptr, MemFunc pFunc)
		: m_pCallBack(NULL)
	{
		m_pCallBack = new fcyFunctorCallBack_MemFunc<Class, MemFunc>(Ptr, pFunc);
	}

	fcyFunctor(const fcyFunctor& Org)
		: m_pCallBack(Org.m_pCallBack)
	{
		if (m_pCallBack)
			m_pCallBack->AddRef();
	}

	~fcyFunctor()
	{
		FCYSAFEKILL(m_pCallBack);
	}
};

/// @}
