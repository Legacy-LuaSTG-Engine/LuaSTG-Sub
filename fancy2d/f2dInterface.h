////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInterface.h
/// @brief fancy2D接口定义
/// @note  定义了所有f2d接口的基类
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <fcyType.h>
#include <fcyMath.h>

// --- f2dInterface 等价于 fcyRefObj ---
#ifdef FCYREFOBJ

typedef fcyRefObj f2dInterface;

#else

////////////////////////////////////////////////////////////////////////////////
/// @brief f2d引用计数接口定义
////////////////////////////////////////////////////////////////////////////////
struct f2dInterface
{
	virtual void AddRef()=0;  ///< @brief 增加接口的引用计数
	virtual void Release()=0; ///< @brief 减少接口的引用计数
	                          ///< @note  当计数器置0时销毁对象
};

#endif

// --- 在f2d中对流接口重新定义 ---
/// @cond NOFCYLIB

#ifdef FCYSTREAM

typedef FCYSEEKORIGIN F2DSEEKORIGIN;
typedef fcyStream f2dStream;

#else

enum F2DSEEKORIGIN
{
	F2DSEEKORIGIN_BEG = 0,  ///< @brief 从头开始寻址
	                        ///< @note  指针寻址位置开始于0处
	F2DSEEKORIGIN_CUR = 1,  ///< @brief 从当前位置开始寻址
	F2DSEEKORIGIN_END = 2   ///< @brief 从结尾处开始寻址
	                        ///< @note  指针寻址位置开始于EOF处
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy流接口
////////////////////////////////////////////////////////////////////////////////
struct f2dStream : f2dInterface
{
	/// @brief 返回流是否可写
	virtual fBool CanWrite()=0;

	/// @brief 返回流是否可变长
	virtual fBool CanResize()=0;

	/// @brief 返回流长度
	virtual fLen GetLength()=0;

	/// @brief     设置新长度
	/// @param[in] Length 流的新长度
	virtual fResult SetLength(fLen Length)=0;
	
	/// @brief 获得读写指针的位置
	virtual fLen GetPosition()=0;

	/// @brief     设置读写位置
	/// @param[in] Origin 寻址参考位置
	/// @param[in] Offset 位移
	virtual fResult SetPosition(F2DSEEKORIGIN Origin, fLong Offset)=0;

	/// @brief      从流中读取字节数据
	/// @param[in]  pData      目的缓冲区
	/// @param[in]  Length     数据长度
	/// @param[out] pBytesRead 真实读写长度，可置为NULL
	virtual fResult ReadBytes(fData pData, fLen Length, fLen* pBytesRead=NULL)=0;

	/// @brief      向流中写入字节数据
	/// @param[in]  pSrc        原始缓冲区
	/// @param[in]  Length      数据长度
	/// @param[out] pBytesWrite 真实读写长度，可置为NULL
	virtual fResult WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite=NULL)=0;

	/// @brief   锁定流
	/// @note    该函数可能会造成阻塞
	/// @warning 如果一个流在多线程环境下被使用时必须在读写区块中手动加锁
	virtual void Lock()=0;
	
	/// @brief   试图锁定流
	/// @warning 如果一个流在多线程环境下被使用时必须在读写区块中手动加锁
	/// @return  使用FCYOK和FCYFAILED判断是否成功加锁
	virtual fResult TryLock()=0;

	/// @brief   解锁流
	/// @note    该函数必须在Lock和TryLock成功的条件下进行调用
	/// @warning 如果一个流在多线程环境下被使用时必须在读写区块中手动加锁
	virtual void Unlock()=0;
};

#endif

/// @endcond
