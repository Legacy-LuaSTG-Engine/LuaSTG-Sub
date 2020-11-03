////////////////////////////////////////////////////////////////////////////////
/// @file  fcyBinaryHelper.h
/// @brief fancy二进制读写辅助
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyIO/fcyStream.h"

/// @addtogroup fancy库IO模块
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy二进制读取辅助
////////////////////////////////////////////////////////////////////////////////
class fcyBinaryReader
{
private:
	fcyStream* m_pStream;         ///< @原始流
public:
	/// @brief   返回基础流
	/// @warning 本函数不会增加原始流的引用计数，如果需要管理这个指针请手动调用AddRef
	fcyStream* GetBaseStream();

	/// @brief 读取一个字符
	fChar ReadChar();
	
	/// @brief      读取多个字符
	/// @param[out] OutBuffer 输出缓冲区
	/// @param[in]  Length    要读取的字符个数
	void ReadChars(fStr OutBuffer, fLen Length);
	
	/// @brief 读取一个宽字符
	fCharW ReadCharW();
	
	/// @brief      读取多个宽字符
	/// @param[out] OutBuffer 输出缓冲区
	/// @param[in]  Length    要读取的字符个数
	void ReadCharsW(fStrW OutBuffer, fLen Length);
	
	/// @brief 读取一个字节
	fByte ReadByte();

	/// @brief 读取多个字节数据
	/// @param[out] OutBuffer 输出缓冲区
	/// @param[in]  Length    要读取的字节数目
	void ReadBytes(fData OutBuffer, fLen Length);
	
	/// @brief 读取一个短整数
	fShort ReadInt16();
	
	/// @brief 读取一个无符号短整数
	fuShort ReadUInt16();

	/// @brief 读取一个整数
	fInt ReadInt32();

	/// @brief 读取一个无符号整数
	fuInt ReadUInt32();

	/// @brief 读取一个长整数
	fLong ReadInt64();

	/// @brief 读取一个无符号长整数
	fuLong ReadUInt64();

	/// @brief 读取一个浮点数
	fFloat ReadFloat();

	/// @brief 读取一个双精度浮点数
	fDouble ReadDouble();
public:
	/// @brief     构造函数
	/// @param[in] pStream 原始流
	fcyBinaryReader(fcyStream* pStream);
	~fcyBinaryReader();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy二进制写入辅助
////////////////////////////////////////////////////////////////////////////////
class fcyBinaryWriter
{
private:
	fcyStream* m_pStream;         ///< @原始流
public:
	/// @brief   返回基础流
	/// @warning 本函数不会增加原始流的引用计数，如果需要管理这个指针请手动调用AddRef
	fcyStream* GetBaseStream();
	
	/// @brief 写出一个字符
	void Write(fChar Value);
	
	/// @brief 写出一个字符串
	void Write(fcStr Buffer, fLen Length);
	
	/// @brief 写出一个宽字符
	void Write(fCharW Value);
	
	/// @brief 写出一个宽字符串
	void Write(fcStrW Buffer, fLen Length);
	
	/// @brief 写出一个字节
	void Write(fByte Value);
	
	/// @brief 写出一个内存缓冲
	void Write(fcData Buffer, fLen Length);
	
	/// @brief 写出一个短整数
	void Write(fShort Value);
	
	/// @brief 写出一个无符号短整数
	void Write(fuShort Value);
	
	/// @brief 写出一个整数
	void Write(fInt Value);
	
	/// @brief 写出一个无符号整数
	void Write(fuInt Value);
	
	/// @brief 写出一个长整数
	void Write(fLong Value);
	
	/// @brief 写出一个无符号长整数
	void Write(fuLong Value);
	
	/// @brief 写出一个浮点数
	void Write(fFloat Value);
	
	/// @brief 写出一个双精度浮点数
	void Write(fDouble Value);
public:
	/// @brief     构造函数
	/// @param[in] pStream 原始流
	fcyBinaryWriter(fcyStream* pStream);
	~fcyBinaryWriter();
};

/// @}