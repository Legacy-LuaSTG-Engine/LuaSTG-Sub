////////////////////////////////////////////////////////////////////////////////
/// @file  fcyPathParser.h
/// @brief fcy路径解析器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyType.h"

#include <string>

/// @addtogroup fancy库解析辅助
/// @{

/// @brief Fcy路径解析函数
namespace fcyPathParser
{
	/// @brief     获得后缀
	/// @param[in] Path 路径
	/// @return    返回后缀
	std::string GetExtension(const std::string& Path);
	
	/// @brief     获得小写后缀
	/// @param[in] Path 路径
	/// @return    返回小写后缀
	std::string GetExtensionLower(const std::string& Path);

	/// @brief     获得文件名
	/// @param[in] Path 路径
	/// @return    返回文件名
	std::string GetFileName(const std::string& Path);

	/// @brief     获得去后缀文件名
	/// @param[in] Path 路径
	/// @return    返回去后缀文件名
	std::string GetFileNameWithoutExt(const std::string& Path);

	/// @brief     获得目录
	/// @param[in] Path 路径
	/// @return    返回目录
	std::string GetPath(const std::string& Path);

	/// @brief     获得后缀，宽字符版
	/// @param[in] Path 路径
	/// @return    返回后缀
	std::wstring GetExtension(const std::wstring& Path);
	
	/// @brief     获得小写后缀，宽字符版
	/// @param[in] Path 路径
	/// @return    返回小写后缀
	std::wstring GetExtensionLower(const std::wstring& Path);

	/// @brief     获得文件名，宽字符版
	/// @param[in] Path 路径
	/// @return    返回文件名
	std::wstring GetFileName(const std::wstring& Path);

	/// @brief     获得去后缀文件名，宽字符版
	/// @param[in] Path 路径
	/// @return    返回去后缀文件名
	std::wstring GetFileNameWithoutExt(const std::wstring& Path);

	/// @brief     获得目录，宽字符版
	/// @param[in] Path 路径
	/// @return    返回目录
	std::wstring GetPath(const std::wstring& Path);
};
/// @}