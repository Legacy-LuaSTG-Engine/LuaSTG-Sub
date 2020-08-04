////////////////////////////////////////////////////////////////////////////////
/// @file  fcyIni.h
/// @brief fcyIni解析器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyParser/fcyLexicalParser.h"

#include <unordered_map>
#include <vector>

/// @addtogroup fancy库解析辅助
/// @{
////////////////////////////////////////////////////////////////////////////////
/// @brief Ini节
////////////////////////////////////////////////////////////////////////////////
class fcyIniSection
{
	friend class fcyIni;
private:
	std::wstring m_Name;
	std::unordered_map<std::wstring, std::wstring> m_Dict;
	std::vector<std::wstring> m_KeyList;
private:
	void writeToStr(std::wstring& Out);
public:
	/// @brief 获得节名称
	fcStrW GetName();
	
	/// @brief 获得Key/Value对数
	fuInt GetCount();

	/// @brief 通过下标获得键名
	/// @note  不存在，返回NULL
	fcStrW GetKey(fuInt Index);

	/// @brief 通过下标获得键值
	/// @note  不存在，返回NULL
	fcStrW GetValue(fuInt Index);
	
	/// @brief     获得值
	/// @note      通过项名返回值，若不存在返回默认值
	/// @param[in] Key     项
	/// @param[in] Default 默认值
	fcStrW GetValue(fcStrW Key, fcStrW Default);

	/// @brief     设置值
	/// @note      若不存在项，追加新值
	/// @param[in] Key   项名
	/// @param[in] Value 值
	fBool SetValue(fcStrW Key, fcStrW Value);

	/// @brief     移除键
	/// @param[in] Key 键名
	fBool Remove(fcStrW Key);
public:
	/// @brief 构造函数
	fcyIniSection(const std::wstring& Name);
	~fcyIniSection(void);
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Ini文件
////////////////////////////////////////////////////////////////////////////////
class fcyIni
{
private:
	std::unordered_map<std::wstring, fcyIniSection*> m_Dict;
	std::vector<std::wstring> m_SecList;
private:
	fBool ignoreComment(fcyLexicalReader& tReader);
	fcyIniSection* parseIniSection(fcyLexicalReader& tReader);
public:
	/// @brief     追加一个节
	/// @note      fcyIni将会负责删除节
	/// @param[in] pSec 节
	fBool AppendSection(fcyIniSection* pSec);
	/// @brief     是否存在节
	/// @param[in] SecName 节名称
	fBool IsContain(fcStrW SecName);
	/// @brief     移除节
	/// @param[in] SecName 节名称
	fBool Remove(fcStrW SecName);
	/// @brief     获得节
	/// @note      不存在返回NULL
	/// @param[in] SecName 节名称
	fcyIniSection* GetSection(fcStrW SecName);
	/// @brief     获得值
	/// @param[in] Sec     节名称
	/// @param[in] Key     键名
	/// @param[in] Default 默认值
	fcStrW GetValue(fcStrW Sec, fcStrW Key, fcStrW Default);
	/// @brief     设置值
	/// @param[in] Sec     节名称
	/// @param[in] Key     键名
	/// @param[in] Value   键值
	void SetValue(fcStrW Sec, fcStrW Key, fcStrW Value);
	/// @brief     写到字符串
	/// @param[in] Out 目的字符串
	void WriteToStr(std::wstring& Out);
	/// @brief 写到流
	/// @param[in] pStream 目的流
	fBool WriteToStream(fcyStream* pStream);
public:
	/// @brief 构造一个新的INI文件
	fcyIni(void);
	/// @brief 从文本构造INI文件
	fcyIni(const std::wstring& Src);
	/// @brief 从流构造INI文件
	fcyIni(fcyStream* pStream);
	~fcyIni(void);
};

/// @}
