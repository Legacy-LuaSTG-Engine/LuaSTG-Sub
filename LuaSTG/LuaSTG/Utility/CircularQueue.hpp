#pragma once
#include <cassert>
#include <array>

namespace LuaSTGPlus
{
	template <typename T, size_t MaxSize>
	class CircularQueue
	{
	private:
		std::array<T, MaxSize> m_Data;
		size_t m_Front = 0; // 头部索引
		size_t m_Rear  = 0; // 下一个可置入的对象的索引
		size_t m_Count = 0; // 已用空间
	public:
		T& operator[](size_t idx)
		{
			assert(idx < m_Count);
			return m_Data[(idx + m_Front) % MaxSize];
		}
	public:
		// 队列是否为空
		bool IsEmpty() const noexcept { return m_Count == 0; }
		// 队列是否已满
		bool IsFull() const noexcept { return m_Count >= MaxSize; }
		// 返回已经使用的空间
		size_t Size() const noexcept { return m_Count; }
		// 返回最大容量
		constexpr size_t Capacity() const noexcept { return MaxSize; }
		// 重置
		void Clear() noexcept { m_Front = 0; m_Rear = 0; m_Count = 0; }
		// 预分配空间
		void PlacementResize(size_t size) { assert(size <= MaxSize); m_Rear = size; m_Count = size; }
		// 在尾部插入对象，仅分配空间，不产生复制
		T& PlacementPushTail()
		{
			assert(!IsFull());
			T& Data = m_Data[m_Rear]; // 当前索引的位置
			m_Rear = (m_Rear + 1) % MaxSize; // 索引后移
			m_Count += 1;
			return Data;
		}
		// 在头部插入对象，仅分配空间，不产生复制
		T& PlacementPushHead()
		{
			assert(!IsFull());
			m_Front = (m_Front + MaxSize - 1) % MaxSize; // 头部索引前移
			T& Data = m_Data[m_Front]; // 头部索引的位置
			m_Count += 1;
			return Data;
		}
		// 在尾部插入对象
		void PushTail(T const& value)
		{
			assert(!IsFull());
			m_Data[m_Rear] = value; // 当前索引的位置
			m_Rear = (m_Rear + 1) % MaxSize; // 索引后移
			m_Count += 1;
		}
		// 在头部插入对象
		void PushHead(T const& value)
		{
			assert(!IsFull());
			m_Front = (m_Front + MaxSize - 1) % MaxSize; // 头部索引前移
			m_Data[m_Front] = value; // 头部索引的位置
			m_Count += 1;
		}
		// 从尾部移除一个对象
		T& PopTail()
		{
			assert(!IsEmpty());
			m_Rear = (m_Rear + MaxSize - 1) % MaxSize; // 当前索引前移
			T& Data = m_Data[m_Rear]; // 当前索引的位置
			m_Count -= 1;
			return Data;
		}
		// 从头部移除一个对象
		T& PopHead()
		{
			assert(!IsEmpty());
			T& Data = m_Data[m_Front]; // 头部索引的位置
			m_Front = (m_Front + 1) % MaxSize; // 头部索引后移
			m_Count -= 1;
			return Data;
		}
		// 访问尾部
		T& Tail() { assert(!IsEmpty()); return m_Data[m_Rear + MaxSize - 1]; }
		// 访问头部
		T& Head() { assert(!IsEmpty()); return m_Data[m_Front]; }
	public:
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
			assert(!IsEmpty());
			return m_Data[m_Front];
		}
		//获得尾部对象的引用
		T& Back()
		{
			assert(!IsEmpty());
			if (m_Rear == 0)
				return m_Data[MaxSize - 1];//这时尾部其实在最后
			else
				return m_Data[m_Rear - 1];//正常索引对象
		}
	};
};
