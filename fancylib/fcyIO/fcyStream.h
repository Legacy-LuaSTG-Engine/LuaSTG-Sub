////////////////////////////////////////////////////////////////////////////////
/// @file  fcyStream.h
/// @brief 描述并实现了fancy内部的流式接口
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "fcyRefObj.h"
#include "fcyException.h"
#include "fcyOS/fcyMultiThread.h"

#include <string>
#include <vector>

/// @addtogroup fancy库IO模块
/// @brief      提供对文件、内存的读写以及额外操作支持
/// @note       基于fcyStream的IO系统，通过实现fcyStream接口来实现其他来源的流

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy流寻址方式
////////////////////////////////////////////////////////////////////////////////
enum FCYSEEKORIGIN {
    FCYSEEKORIGIN_BEG = 0,
    FCYSEEKORIGIN_CUR = 1,
    FCYSEEKORIGIN_END = 2,
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy流接口
////////////////////////////////////////////////////////////////////////////////
struct fcyStream : fcyRefObj {
    //返回流是否可写
    virtual fBool CanWrite() = 0;
    
    //返回流是否可变长
    virtual fBool CanResize() = 0;
    
    //返回流长度
    virtual fLen GetLength() = 0;
    
    //设置新长度
    //param[in] Length 流的新长度
    virtual fResult SetLength(fLen Length) = 0;
    
    //获得读写指针的位置
    virtual fLen GetPosition() = 0;
    
    //设置读写位置
    //param[in] Origin 寻址参考位置
    //param[in] Offset 位移
    virtual fResult SetPosition(FCYSEEKORIGIN Origin, fLong Offset) = 0;
    
    //从流中读取字节数据
    //@param[in]  pData      目的缓冲区
    //@param[in]  Length     数据长度
    //@param[out] pBytesRead 真实读写长度，可置为NULL
    virtual fResult ReadBytes(fData pData, fLen Length, fLen* pBytesRead = NULL) = 0;
    
    //向流中写入字节数据
    //param[in]  pSrc        原始缓冲区
    //param[in]  Length      数据长度
    //param[out] pBytesWrite 真实读写长度，可置为NULL
    virtual fResult WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite = NULL) = 0;
    
    //锁定流
    //note    该函数可能会造成阻塞
    //warning 如果一个流在多线程环境下被使用时必须在读写区块中手动加锁
    virtual void Lock() = 0;
    
    //试图锁定流
    //warning 如果一个流在多线程环境下被使用时必须在读写区块中手动加锁
    //return  使用FCYOK和FCYFAILED判断是否成功加锁
    virtual fResult TryLock() = 0;
    
    //解锁流
    //note    该函数必须在Lock和TryLock成功的条件下进行调用
    //warning 如果一个流在多线程环境下被使用时必须在读写区块中手动加锁
    virtual void Unlock() = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy文件流实现
////////////////////////////////////////////////////////////////////////////////
class fcyFileStream : public fcyRefObjImpl<fcyStream> {
private:
    fHandle m_hFile;       ///< @brief 内部文件句柄
    fBool m_bWritable;    ///< @brief 是否可写
    std::wstring m_sPath; ///< @brief 文件路径
    fcyCriticalSection m_CriticalSec; ///< @brief 临界区
public: // 接口实现
    fBool CanWrite();
    
    fBool CanResize();
    
    fLen GetLength();
    
    fResult SetLength(fLen Length);
    
    fLen GetPosition();
    
    fResult SetPosition(FCYSEEKORIGIN Origin, fLong Offset);
    
    fResult ReadBytes(fData pData, fLen Length, fLen* pBytesRead);
    
    fResult WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite);
    
    void Lock();
    
    fResult TryLock();
    
    void Unlock();

public:
    /// @brief     构造函数
    /// @param[in] Path     文件路径
    /// @param[in] Writable 可写
    fcyFileStream(fcStrW Path, fBool Writable);

protected:
    ~fcyFileStream();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy内存流
////////////////////////////////////////////////////////////////////////////////
class fcyMemStream : public fcyRefObjImpl<fcyStream> {
private:
    std::vector<fByte> m_Data;        ///< @brief 内部数组
    fBool m_bResizable;               ///< @brief 可变长
    fBool m_bWritable;                ///< @brief 可写
    fLen m_cPointer;                  ///< @brief 读写位置
    fcyCriticalSection m_CriticalSec; ///< @brief 临界区
public: // 接口实现
    fBool CanWrite();
    
    fBool CanResize();
    
    fLen GetLength();
    
    fResult SetLength(fLen Length);
    
    fLen GetPosition();
    
    fResult SetPosition(FCYSEEKORIGIN Origin, fLong Offset);
    
    fResult ReadBytes(fData pData, fLen Length, fLen* pBytesRead);
    
    fResult WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite);
    
    void Lock();
    
    fResult TryLock();
    
    void Unlock();

public: // 扩展接口
    fData GetInternalBuffer();

public:
    /// @brief     构造函数
    /// @param[in] Src       数据源，置为NULL则不从数据源拷贝
    /// @param[in] Length    数据长度
    /// @param[in] Writable  可写
    /// @param[in] Resizable 可变长
    fcyMemStream(fcData Src, fLen Length, fBool Writable, fBool Resizable);

    fcyMemStream(std::vector<uint8_t>&& rvData);

protected:
    ~fcyMemStream();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy部分流
/// @note  用来对一个流的一部分进行访问
////////////////////////////////////////////////////////////////////////////////
class fcyPartialStream : public fcyRefObjImpl<fcyStream> {
private:
    fcyStream* m_pOrgStream; ///< @brief 原始流
    fLen m_Offset;           ///< @brief 在原始流中的偏移量
    fLen m_pPos;             ///< @brief 当前读写位置
    fLen m_Len;              ///< @brief 原始流长度
public: // 接口实现
    fBool CanWrite();
    
    fBool CanResize();
    
    fLen GetLength();
    
    fResult SetLength(fLen Length);
    
    fLen GetPosition();
    
    fResult SetPosition(FCYSEEKORIGIN Origin, fLong Offset);
    
    fResult ReadBytes(fData pData, fLen Length, fLen* pBytesRead);
    
    fResult WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite);
    
    void Lock();
    
    fResult TryLock();
    
    void Unlock();

public:
    /// @brief     构造函数
    /// @param[in] OrgStream 原始流指针
    /// @param[in] Offset    部分流在原始流中的位移
    /// @param[in] Size      部分流大小
    fcyPartialStream(fcyStream* OrgStream, fLen Offset, fLen Size);

protected:
    ~fcyPartialStream();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy流辅助
////////////////////////////////////////////////////////////////////////////////
namespace fcyStreamHelper {
    /// @brief     填充流
    /// @note      在Src的当前位置读取DataLength个长度到Dest流中
    /// @param[in] Src 原始流
    /// @param[in] Dest 目的流
    /// @param[in] DataLength 数据长度
    /// @return    FCYERR_OK：操作成功完成，FCYERR_INTERNALERR：读取时错误
    fResult FillStream(fcyStream* Src, fcyStream* Dest, fLen DataLength);
};
