﻿////////////////////////////////////////////////////////////////////////////////
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
	char ReadChar();
	
	/// @brief      读取多个字符
	/// @param[out] OutBuffer 输出缓冲区
	/// @param[in]  Length    要读取的字符个数
	void ReadChars(char* OutBuffer, uint64_t Length);
	
	/// @brief 读取一个宽字符
	wchar_t ReadCharW();
	
	/// @brief      读取多个宽字符
	/// @param[out] OutBuffer 输出缓冲区
	/// @param[in]  Length    要读取的字符个数
	void ReadCharsW(fStrW OutBuffer, uint64_t Length);
	
	/// @brief 读取一个字节
	uint8_t ReadByte();

	/// @brief 读取多个字节数据
	/// @param[out] OutBuffer 输出缓冲区
	/// @param[in]  Length    要读取的字节数目
	void ReadBytes(fData OutBuffer, uint64_t Length);
	
	/// @brief 读取一个短整数
	int16_t ReadInt16();
	
	/// @brief 读取一个无符号短整数
	uint16_t ReadUInt16();

	/// @brief 读取一个整数
	int32_t ReadInt32();

	/// @brief 读取一个无符号整数
	uint32_t ReadUInt32();

	/// @brief 读取一个长整数
	int64_t ReadInt64();

	/// @brief 读取一个无符号长整数
	uint64_t ReadUInt64();

	/// @brief 读取一个浮点数
	float ReadFloat();

	/// @brief 读取一个双精度浮点数
	double ReadDouble();
public:
	/// @brief     构造函数
	/// @param[in] pStream 原始流
	fcyBinaryReader(fcyStream* pStream);
	~fcyBinaryReader();
};

/// @}