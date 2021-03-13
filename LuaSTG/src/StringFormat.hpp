#pragma once
#include <string>

namespace LuaSTGPlus
{
	// 转换
	std::wstring MultiByteToWideChar(const std::string& src, unsigned int cp = 65001);
	
	// 转换
	std::string WideCharToMultiByte(const std::wstring& src, unsigned int cp = 65001);
	
	//字符串格式化
	//param[in] Format 字符串格式，不支持精度
	std::string StringFormat(const char* Format, ...)noexcept;
	
	//字符串格式化 va_list版本
	std::string StringFormatV(const char* Format, va_list vaptr)noexcept;
	
	//字符串格式化 宽字符
	//param[in] Format 字符串格式，不支持精度
	std::wstring StringFormat(const wchar_t* Format, ...)noexcept;
	
	//字符串格式化 宽字符、va_list版本
	std::wstring StringFormatV(const wchar_t* Format, va_list vaptr)noexcept;
};
