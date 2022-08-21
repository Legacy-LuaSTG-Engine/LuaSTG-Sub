////////////////////////////////////////////////////////////////////////////////
/// @file  fcyStringHelper.h
/// @brief fancy字符串辅助库
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyType.h"

#include <vector>
#include <string>

/// @addtogroup fancy杂项
/// @brief 未分类对象
/// @{

/// @brief fcy字符串辅助
namespace fcyStringHelper
{
	/// @brief      字符串分割
	/// @param[in]  Source   源字符串
	/// @param[in]  SplitStr 用于分割的字符串
	/// @param[in]  AutoTrim 自动剔除空白的分割项
	/// @param[out] Out      输出的列表
	/// @return     被分割的数量
	uint32_t StringSplit(const std::string& Source, const std::string& SplitStr, bool AutoTrim, std::vector<std::string>& Out);
	
	/// @brief      字符串分割，宽字符版本
	/// @param[in]  Source   源字符串
	/// @param[in]  SplitStr 用于分割的字符串
	/// @param[in]  AutoTrim 自动剔除空白的分割项
	/// @param[out] Out      输出的列表
	/// @return     被分割的数量
	uint32_t StringSplit(const std::wstring& Source, const std::wstring& SplitStr, bool AutoTrim, std::vector<std::wstring>& Out);
	
	/// @brief     字符串到小写
	/// @param[in] Source   源字符串
	/// @return    被转换的字符串
	std::string ToLower(const std::string& Source);

	/// @brief     字符串到小写，宽字符版本
	/// @param[in] Source   源字符串
	/// @return    被转换的字符串
	std::wstring ToLower(const std::wstring& Source);

	/// @brief     数字到字符串
	/// @param[in] Number 数字
	/// @param[in] Scale  进制
	/// @return    输出的字符串
	std::string ToStr(int32_t Number, int32_t Scale=10);

	/// @brief     数字到字符串
	/// @param[in] Number 数字
	/// @return    输出的字符串
	std::string ToStr(double Number);

	/// @brief     数字到字符串，宽字符版本
	/// @param[in] Number 数字
	/// @param[in] Scale  进制
	/// @return    输出的字符串
	std::wstring ToWideStr(int32_t Number, int32_t Scale=10);
	
	/// @brief     数字到字符串，宽字符版本
	/// @param[in] Number 数字
	/// @return    输出的字符串
	std::wstring ToWideStr(double Number);

	/// @brief     时间到字符串
	/// @note      将毫秒数转换到h:m:s.ms格式
	/// @param[in] Seconds 毫秒数
	/// @return    输出的字符串
	std::string TimeToStr(double Seconds);

	/// @brief     时间到字符串，宽字符版本
	/// @note      将毫秒数转换到h:m:s.ms格式
	/// @param[in] Seconds 毫秒数
	/// @return    输出的字符串
	std::wstring TimeToWideStr(double Seconds);

	/// @brief     剔除左侧空白字符
	/// @param[in] Org 原始字符串
	std::wstring TrimLeft(const std::wstring &Org);
	
	/// @brief     剔除右侧空白字符
	/// @param[in] Org 原始字符串
	std::wstring TrimRight(const std::wstring &Org);
	
	/// @brief     剔除空白字符
	/// @param[in] Org 原始字符串
	std::wstring Trim(const std::wstring &Org);

	/// @brief     剔除左侧空白字符
	/// @param[in] Org 原始字符串
	std::string TrimLeft(const std::string &Org);
	
	/// @brief     剔除右侧空白字符
	/// @param[in] Org 原始字符串
	std::string TrimRight(const std::string &Org);
	
	/// @brief     剔除空白字符
	/// @param[in] Org 原始字符串
	std::string Trim(const std::string &Org);

	/// @brief     string到wstring
	/// @param[in] Org 原始字符串
	/// @param[in] CodePage 代码页，具体查阅MSDN
	/// @return    返回被转换的字符串
	std::wstring MultiByteToWideChar(const std::string& Org, uint32_t CodePage=65001);

	/// @brief     wstring到string
	/// @param[in] Org 原始字符串
	/// @param[in] CodePage 代码页，具体查阅MSDN
	/// @return    返回被转换的字符串
	std::string WideCharToMultiByte(const std::wstring& Org, uint32_t CodePage=65001);
}
/// @}
