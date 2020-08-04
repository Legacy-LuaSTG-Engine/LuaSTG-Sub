////////////////////////////////////////////////////////////////////////////////
/// @file  fcyRefObj.h
/// @brief 描述并实现了引用计数接口
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyType.h"

#include <Windows.h>

#define FCYREFOBJ

////////////////////////////////////////////////////////////////////////////////
/// @brief 引用计数接口定义
/// @note  AddRef和Release函数应当加锁
////////////////////////////////////////////////////////////////////////////////
struct fcyRefObj
{
	virtual void AddRef()=0;  ///< @brief 增加接口的引用计数
	virtual void Release()=0; ///< @brief 减少接口的引用计数
	                          ///< @note  当计数器置0时销毁对象

	virtual ~fcyRefObj() {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 引用计数接口实现
/// @note  使用模版进行代码复用防止出现菱形继承
////////////////////////////////////////////////////////////////////////////////
template<class T>
class fcyRefObjImpl : 
	public T
{
private:
	fuInt m_cRef;
public:
	/// @brief AddRef实现
	virtual void AddRef()
	{
		InterlockedIncrement(&m_cRef);
	}

	/// @brief Release实现
	virtual void Release()
	{
		fuInt tRet = InterlockedDecrement(&m_cRef);
		if(tRet == 0)
			delete this;
	}
private:
	fcyRefObjImpl(const fcyRefObjImpl& Org);
public:
	fcyRefObjImpl()
		: m_cRef(1) {}
	virtual ~fcyRefObjImpl() {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 智能指针
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class fcyRefPointer
{
protected:
	T* m_pPointer;
public:
	fcyRefPointer& DirectSet(T* Ptr)
	{
		FCYSAFEKILL(m_pPointer);
		m_pPointer = Ptr;

		return *this;
	}
public:
	bool operator==(const fcyRefPointer& Right)const
	{
		return (m_pPointer == Right);
	}
	fcyRefPointer& operator=(const fcyRefPointer& Right)
	{
		if(m_pPointer != Right.m_pPointer)
		{
			FCYSAFEKILL(m_pPointer);
			m_pPointer = Right.m_pPointer;
			if(m_pPointer)
				m_pPointer->AddRef();
		}
		return *this;
	}
	T* operator->()const
	{
		return m_pPointer;
	}
	T* operator*()const
	{
		return m_pPointer;
	}
	T** operator&()
	{
		return &m_pPointer;
	}
	operator T*()const
	{
		return m_pPointer;
	}
	template<typename P>
	operator fcyRefPointer<P>()
	{
		fcyRefPointer<P> tRet = fcyRefPointer<P>((P*)m_pPointer);
		return tRet;
	}
	template<typename P>
	operator fcyRefPointer<P>()const
	{
		fcyRefPointer<P> tRet = fcyRefPointer<P>((P*)m_pPointer);
		return tRet;
	}
public:
	fcyRefPointer()
		: m_pPointer(NULL) {}
	fcyRefPointer(T* pObj)
		: m_pPointer(pObj) 
	{
		if(pObj)
			pObj->AddRef();
	}
	fcyRefPointer(const fcyRefPointer& Right)
		: m_pPointer(Right.m_pPointer)
	{
		if(m_pPointer)
			m_pPointer->AddRef();
	}
	~fcyRefPointer()
	{
		FCYSAFEKILL(m_pPointer);
	}
};
