////////////////////////////////////////////////////////////////////////////////
/// @file  fcyJson.h
/// @brief fcyJson解析器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyParser/fcyLexicalParser.h"

#include <string>
#include <vector>
#include <unordered_map>

/// @addtogroup fancy库解析辅助
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief JSON元素类型
////////////////////////////////////////////////////////////////////////////////
enum FCYJSONVALUETYPE
{
	FCYJSONVALUETYPE_NULL,   ///< @brief 空类型
	FCYJSONVALUETYPE_BOOL,   ///< @brief 逻辑类型
	FCYJSONVALUETYPE_NUMBER, ///< @brief 数字类型
	FCYJSONVALUETYPE_STRING, ///< @brief 字符串类型
	FCYJSONVALUETYPE_LIST,   ///< @brief 表类型
	FCYJSONVALUETYPE_DICT,   ///< @brief 字典类型
};

class fcyJsonString;
class fcyJsonList;
class fcyJsonDict;

////////////////////////////////////////////////////////////////////////////////
/// @brief JSON值
////////////////////////////////////////////////////////////////////////////////
class fcyJsonValue
{
	friend class fcyJsonList;
	friend class fcyJsonDict;
	friend class fcyJson;
protected:
	FCYJSONVALUETYPE m_Type;  ///< @brief JSON对象类型

	/// @brief 数据域
	union
	{
		fBool m_ValueBool;     ///< @brief 逻辑型
		fDouble m_ValueNumber; ///< @brief 数字型
	};
protected:
	/// @brief      写到字符串
	/// @param[out] OutStr 输出的目的字符串
	virtual void writeToStr(std::wstring& OutStr);
public:  // 类型转换
	FCYJSONVALUETYPE GetType(); ///< @brief 返回类型
	
	fBool ToNull();             ///< @brief 到NULL
	                            ///< @note  true表示NULL, false表示非NULL
	fBool ToBool();             ///< @brief 到Bool
	                            ///< @note  true表示true, false表示false或非Bool型
	fDouble ToNumber();         ///< @brief 到数字
	                            ///< @note  非Number型用0表示
public:  // 用于接口转换
	virtual fcyJsonString* ToString(); ///< @brief 转换到字符串
	virtual fcyJsonList* ToList();     ///< @brief 转换到数组
	virtual fcyJsonDict* ToDict();     ///< @brief 转换到字典
public: // 值类型操作, 非值类型返回false
	fBool SetValue();                  ///< @brief  设置为null
	                                   ///< @return 如果对象不是值类型，则返回假
	fBool SetValue(fBool Value);       ///< @brief  设置为bool
	                                   ///< @return 如果对象不是值类型，则返回假
	fBool SetValue(fDouble Value);     ///< @brief  设置为数字
	                                   ///< @return 如果对象不是值类型，则返回假
public: // 用于手动创建
	fcyJsonValue();                      ///< @brief 创建为null值类型
	fcyJsonValue(fBool Value);           ///< @brief 创建为bool值类型
	fcyJsonValue(fDouble Value);         ///< @brief 创建为数值类型
	fcyJsonValue(FCYJSONVALUETYPE Type); ///< @brief 创建为String/List/Dict
	virtual ~fcyJsonValue();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief JSON字符串
////////////////////////////////////////////////////////////////////////////////
class fcyJsonString :
	public fcyJsonValue
{
protected:
	std::wstring m_Str;   ///< @brief 字符串字面值
protected:
	void writeToStr(std::wstring& OutStr);
public:
	fcyJsonString* ToString();
public:
	fcStrW GetStr();           ///< @brief 获得字符串指针
	void SetStr(fcStrW Value); ///< @brief 设置字符串
public:
	/// @brief     构造函数
	/// @param[in] Value 字符串值
	fcyJsonString(fcStrW Value);
	~fcyJsonString();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief JSON数组
/// @note  JsonList将会负责销毁子对象
////////////////////////////////////////////////////////////////////////////////
class fcyJsonList : 
	public fcyJsonValue
{
protected:
	std::vector<fcyJsonValue*> m_ObjList;   ///< @brief 内部数组
protected:
	void writeToStr(std::wstring& OutStr);
public:
	fcyJsonList* ToList();
public:
	/// @brief     获得对象
	/// @param[in] Index 索引
	/// @return    如果索引不存在返回NULL，否则返回对象指针
	fcyJsonValue* GetValue(fuInt Index);

	/// @brief     设置对象
	/// @param[in] Index 索引
	/// @param[in] pNew  对象指针
	/// @return    返回操作是否成功
	fResult SetValue(fuInt Index, fcyJsonValue* pNew);

	/// @brief     往数组末端追加对象
	/// @param[in] pNew 对象指针
	void Append(fcyJsonValue* pNew);

	/// @brief     删除Index处对象
	/// @param[in] Index 对象索引
	fResult Remove(fuInt Index);

	/// @brief 清空
	void Clear();

	/// @brief 返回对象数量
	fuInt GetCount();
public:
	/// @brief 构造函数
	fcyJsonList();
	~fcyJsonList();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief JSON字典
////////////////////////////////////////////////////////////////////////////////
class fcyJsonDict :
	public fcyJsonValue
{
protected:
	std::vector<std::wstring> m_ObjList;                      ///< @brief 对象数组
	std::unordered_map<std::wstring, fcyJsonValue*> m_Cache;  ///< @brief 对象字典缓存
protected:
	void writeToStr(std::wstring& OutStr);
public:
	fcyJsonDict* ToDict();
public:
	/// @brief      返回对象
	/// @param[in]  Index 对象索引
	/// @param[out] pKeyOut 键
	/// @return     返回对象指针，若索引越界返回NULL
	fcyJsonValue* GetValue(fuInt Index, fcStrW* pKeyOut = NULL);

	/// @brief     返回对象
	/// @param[in] Name 对象键名
	/// @return    返回对象指针，若不存在对象返回NULL
	fcyJsonValue* GetValue(fcStrW Name);

	/// @brief     设置对象
	/// @note      如果对象已经存在则会释放原有对象
	/// @param[in] Name 对象键名
	/// @param[in] pNew 对象指针
	void SetValue(fcStrW Name, fcyJsonValue* pNew);

	/// @brief     是否包含对象
	/// @param[in] Name 对象的键名
	/// @return    true=包含对象，false=没包含对象
	fBool Contain(fcStrW Name);

	/// @brief     移除对象
	/// @param[in] Index 对象键名
	/// @return    true=成功，false=失败
	fBool Remove(fcStrW Index);

	/// @brief 清空
	void Clear();

	/// @brief 返回元素个数
	fuInt GetCount();
public:
	/// @brief 构造函数
	fcyJsonDict();
	~fcyJsonDict();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief JSON解析器
/// @note  注意该解析器只会解析第一个元素。
////////////////////////////////////////////////////////////////////////////////
class fcyJson
{
private:
	fcyJsonValue* m_Root;  ///< @brief 根节点
private: // 解析函数
	fcyJsonValue* parseValue(fcyLexicalReader& Context);   ///< @brief 解析一个值
	fcyJsonValue* parseNumber(fcyLexicalReader& Context);  ///< @brief 解析一个数值
	fcyJsonString* parseString(fcyLexicalReader& Context); ///< @brief 解析一个字符串
	fcyJsonList* parseList(fcyLexicalReader& Context);     ///< @brief 解析一个表
	fcyJsonDict* parseDict(fcyLexicalReader& Context);     ///< @brief 解析一个字典
public:
	fcyJsonValue* GetRoot();             ///< @brief 解析一个根节点
	void SetRoot(fcyJsonValue* pValue);  ///< @brief 设置根节点

	void WriteToStr(std::wstring& pOut); ///< @brief 写到字符串
	void WriteToStream(fcyStream* pOut); ///< @brief 写到流
public:
	fcyJson();                           ///< @brief 构造函数
	fcyJson(const std::wstring& Str);    ///< @brief 从文本构造JSON
	fcyJson(fcyStream* pStream);         ///< @brief 从流构造JSON
	~fcyJson();
};
/// @}
