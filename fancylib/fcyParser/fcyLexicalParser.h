////////////////////////////////////////////////////////////////////////////////
/// @file  fcyLexicalParser.h
/// @brief fcy词法解析辅助类
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyException.h"
#include "fcyIO/fcyStream.h"

#include <string>

/// @addtogroup fancy库异常
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief 词法解析异常
/// @note  派生自fcyException，用来描述词法错误
////////////////////////////////////////////////////////////////////////////////
class fcyLexicalException :
	public fcyException
{
private:
	fuInt m_Line; ///< @brief 储存异常产生的行
	fuInt m_Row;  ///< @brief 储存异常产生的列
public:
	fuInt GetLine();  ///< @brief 获得异常产生的行
	fuInt GetRow();   ///< @brief 获得异常产生的列
public:
	/// @brief          构造函数
	/// @note           构造函数会自动把行号和列号添加到Desc字段
	/// @param[in] Src  异常源
	/// @param[in] Desc 异常描述
	/// @param[in] Line 异常产生的行号
	/// @param[in] Row  异常产生的列号
	fcyLexicalException(fcStr Src, fcStr Desc, fuInt Line, fuInt Row);
	~fcyLexicalException();
};
/// @}

/// @addtogroup fancy库解析辅助
/// @brief 实现对路径、XML、JSON等格式的解析
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief 解析辅助
////////////////////////////////////////////////////////////////////////////////
class fcyLexicalReader
{
private:
	std::wstring m_Str;  ///< @brief 获得异常产生的行
	fuInt m_Pos;         ///< @brief 获得异常产生的行
	fuInt m_Len;         ///< @brief 获得异常产生的行
private: // BOM头处理
	fBool checkUTF8(fcyStream* pStream);
	fBool checkUTF16LE(fcyStream* pStream);
	std::wstring preprocess(fcyStream* pStream);
public:
	fuInt GetLine();     ///< @brief 返回行号
	fuInt GetRow();      ///< @brief 返回列号
	fBool IsEOF();       ///< @brief 是否结束

	fCharW ReadChar();   ///< @brief 读取一个字符
	fCharW PeekChar();   ///< @brief 预读取一个字符

	void IgnoreSpace();  ///< @brief 跳过空白字符

	/// @brief     匹配一个字符
	/// @note      匹配一个字符，失败抛出异常
	/// @param[in] Char         要匹配的字符
	/// @param[in] bIgnoreSpace 忽略空格
	void Match(fCharW Char, fBool bIgnoreSpace);

	/// @brief     匹配一个字符串
	/// @note      匹配一个字符串，失败抛出异常
	/// @param[in] Str          要匹配的字符串
	/// @param[in] bIgnoreSpace 忽略空格
	void Match(fcStrW Str, fBool bIgnoreSpace);
	
	/// @brief     尝试匹配一个字符
	/// @note      尝试匹配一个字符，失败返回false
	/// @param[in] Char         要匹配的字符
	/// @param[in] bIgnoreSpace 忽略空格
	/// @param[in] bMatch       如果能匹配成功，是否匹配这个字符
	/// @return    true=可以匹配，false=匹配失败
	fBool TryMatch(fCharW Char, fBool bIgnoreSpace, fBool bMatch);

	/// @brief     尝试匹配一个字符串
	/// @note      尝试匹配一个字符串，失败返回false
	/// @param[in] Str          要匹配的字符串
	/// @param[in] bIgnoreSpace 忽略空格
	/// @param[in] bMatch       如果能匹配成功，是否匹配这个字符
	/// @return    true=可以匹配，false=匹配失败
	fBool TryMatch(fcStrW Str, fBool bIgnoreSpace, fBool bMatch);
public:
	/// @brief     从文本构造
	/// @param[in] SrcText 要进行匹配的原始字符串
	fcyLexicalReader(const std::wstring& SrcText);
	/// @brief     从流构造
	/// @param[in] SrcText 要进行匹配的原始数据流
	fcyLexicalReader(fcyStream* pStream);
	~fcyLexicalReader(void);
};
/// @}