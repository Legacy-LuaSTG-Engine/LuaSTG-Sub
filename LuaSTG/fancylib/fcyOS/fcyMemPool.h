////////////////////////////////////////////////////////////////////////////////
/// @file  fcyMemPool.h
/// @brief fancy内存池
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyType.h"

#ifdef _DEBUG
#include <cassert>
#endif

#include <vector>

/// @addtogroup fancy库底层支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief     定长内存池实现
/// @param[in] BlockSize 内存块大小
////////////////////////////////////////////////////////////////////////////////
template <uint64_t BlockSize>
class fcyMemPool
{
private:
	uint64_t m_PerPoolLen;                ///< @brief 下次分配的块数量
	uint64_t m_CurAlloctedSize;           ///< @brief 当前分配的大小
	std::vector<void*> m_AllocMemPtr; ///< @brief 已分配内存指针
	std::vector<void*> m_FreeMemPool; ///< @brief 空闲内存指针
private:
	/// @brief 追加空闲内存
	/// @param[in] BlockCount 块数量
	void appendMem(uint32_t BlockCount)
	{
		// 计算需要分配的大小
		uint64_t tAllocSize = BlockCount * BlockSize;
		m_CurAlloctedSize += tAllocSize;
		// 计算当前的空闲指针池应该有的空间
		uint64_t tPoolSize = m_FreeMemPool.size() + BlockCount;
		// 如果不够进行预留
		if(m_FreeMemPool.capacity() < tPoolSize)
			m_FreeMemPool.reserve( (size_t)tPoolSize );
		// 分配空间
		uint8_t* tPtr = (uint8_t*)malloc( (size_t)tAllocSize );
		// 记录分配的空间
		m_AllocMemPtr.push_back(tPtr);
		// 记录所有内存块
		for(uint32_t i = 0; i<BlockCount; ++i)
		{
			m_FreeMemPool.push_back(tPtr);
			tPtr += BlockSize;
		}
	}

	/// @brief 分配内存池
	void allocPool()
	{
		appendMem((uint32_t)m_PerPoolLen);

		// 扩大下次分配值
		m_PerPoolLen = m_PerPoolLen * 2;
	}
public:
	/// @brief  分配内存
	/// @return 内存块指针
	void* Alloc()
	{
		if(m_FreeMemPool.empty())
			allocPool();
		void* pRet = m_FreeMemPool.back();
		m_FreeMemPool.pop_back();
		return pRet;
	}

	/// @brief     释放内存
	/// @param[in] Ptr 内存块指针
	void Free(void* Ptr)
	{
		m_FreeMemPool.push_back(Ptr);
	}

	/// @brief  获得总分配大小
	/// @return 总分配的内存
	uint64_t GetTotalSize()
	{
		return m_CurAlloctedSize;
	}

	/// @brief  获得空闲内存总大小
	/// @return 空闲内存块大小
	uint64_t GetFreeSize()
	{
		return m_FreeMemPool.size() * BlockSize;
	}
public:
	/// @brief     构造函数
	/// @param[in] InitMemSize 初始内存大小
	fcyMemPool(uint32_t InitMemSize = 16)
	{
		m_CurAlloctedSize = 0;  // 已分配大小为0
		m_PerPoolLen = 4;       // 默认下次追加块数量为4

		m_AllocMemPtr.reserve(4);
		m_FreeMemPool.reserve(128);

		appendMem(InitMemSize / BlockSize);
	}

	~fcyMemPool()
	{
#ifdef _DEBUG
		if(GetTotalSize()!=GetFreeSize())
			assert(false);
#endif
		for(std::vector<void*>::iterator i = m_AllocMemPtr.begin(); i != m_AllocMemPtr.end(); ++i)
		{
			free((*i));
		}
	}
};
/// @}
