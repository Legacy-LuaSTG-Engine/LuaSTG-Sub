#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	template <typename T, size_t MaxSize>
	class CirularQueue
	{
	private:
		std::array<T, MaxSize> m_Data;
		size_t m_Front = 0;//头部索引
		size_t m_Rear = 0; //下一个可置入的对象的索引
		size_t m_Count = 0;//已用空间
	public:
		T& operator[](size_t idx)
		{
			LASSERT(idx < m_Count);
			return m_Data[(idx + m_Front) % MaxSize];
		}
	public:
		//队列是否为空
		bool IsEmpty() { return m_Front == m_Rear; }
		//队列是否已满
		bool IsFull() { return (m_Front == (m_Rear + 1) % MaxSize); }
		//返回已经使用的空间
		size_t Size() { return m_Count; }
		//返回最大索引
		size_t Max() { return MaxSize - 1; }
		//在尾部置入一个对象，如果循环队列已满则返回false
		bool Push(T val)
		{
			if (IsFull())
				return false;
			else
			{
				m_Data[m_Rear] = val;//置入对象
				m_Rear = (m_Rear + 1) % MaxSize;//尾部索引后移
				++m_Count;
				return true;
			}
		}
		//在头部（反向）置入一个对象，如果循环队列已满则返回false
		bool PushBack(T val)
		{
			if (IsFull())
				return false;
			else
			{
				m_Data[(m_Front + MaxSize - 1) % MaxSize] = val;//找到头部（反向）置入对象
				m_Front = (m_Front + MaxSize - 1) % MaxSize;//头部索引循环前移
				++m_Count;
				return true;
			}
		}
		//从头部剔除一个对象，并获得该对象的引用
		bool Pop(T& out)
		{
			if (IsEmpty())
				return false;
			else
			{
				out = m_Data[m_Front];
				m_Front = (m_Front + 1) % MaxSize;
				--m_Count;
				return true;
			}
		}
		//获得头部对象的引用
		T& Front()
		{
			LASSERT(!IsEmpty());
			return m_Data[m_Front];
		}
		//获得尾部对象的引用
		T& Back()
		{
			LASSERT(!IsEmpty());
			if (m_Rear == 0)
				return m_Data[MaxSize - 1];//这时尾部其实在最后
			else
				return m_Data[m_Rear - 1];//正常索引对象
		}
	};
}
