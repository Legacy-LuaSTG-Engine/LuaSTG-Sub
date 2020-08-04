#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief 定长对象池
	template <typename T, size_t AllocCount>
	class FixedObjectPool
	{
	private:
		static_assert(std::is_pod<T>::value, "T must be a pod type."); //要求是平凡类型是为了防止出现意外的复制错误
		
		std::vector<size_t> m_FreeIndex;  // 空闲空间索引表
		std::array<bool, AllocCount> m_DataUsed;  // 已用空间标记
		std::array<T, AllocCount> m_DataBuffer;  // 所有空间
	public:
		//申请一个对象
		//<param[out]  size_t --对象id
		//>return[out]   bool --是否成功，失败返回false表明对象池已满
		bool Alloc(size_t& id)
		{
			if (m_FreeIndex.size() > 0)
			{
				id = m_FreeIndex.back();
				m_FreeIndex.pop_back();
				m_DataUsed[id] = true;
				return true;
			}
			id = static_cast<size_t>(-1);
			return false;
		}
		//回收一个对象
		//<param[in] size_t --对象id
		void Free(size_t id)
		{
			if (id < AllocCount && m_DataUsed[id])
			{
				m_DataUsed[id] = false;
				m_FreeIndex.push_back(id);
			}
		}
		//获取对象的数据
		//<param[in]   size_t --对象id
		//>return[out]      T --若id无效返回nullptr
		T* Data(size_t id)
		{
			if (id < AllocCount && m_DataUsed[id])
				return &m_DataBuffer[id];
			return nullptr;
		}
		//返回已分配对象数
		//>return[out] size_t --已分配对象数
		size_t Size()
		{
			return m_DataBuffer.size() - m_FreeIndex.size();
		}
		//清空对象池并回收所有对象
		void Clear()
		{
			m_FreeIndex.resize(m_DataBuffer.size());
			for (size_t i = 0; i < m_DataBuffer.size(); ++i)
			{
				m_FreeIndex[i] = (m_DataBuffer.size() - 1) - i;
				m_DataUsed[i] = false;
			}
		}
	private:
		FixedObjectPool& operator=(const FixedObjectPool&);
		FixedObjectPool(const FixedObjectPool&);
	public:
		FixedObjectPool()
		{
			Clear();
		}
	};
}
