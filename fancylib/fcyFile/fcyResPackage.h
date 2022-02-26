////////////////////////////////////////////////////////////////////////////////
/// @file  fcyResPackage.h
/// @brief fcy资源包
/// @note  本模块目前仅支持资源包的读取
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyException.h"
#include "fcyParser/fcyXml.h"
#include "fcyIO/fcyDeflate.h"

#include <string>
#include <unordered_map>
#include <vector>

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

class fcyResPackageDataNode;
class fcyResPackageFloderNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief 资源包节点
////////////////////////////////////////////////////////////////////////////////
class fcyResPackageNode
{
	friend class fcyResPackageFloderNode;
private:
	fcyResPackageFloderNode* m_pParent;  ///< @brief 父节点
	std::wstring m_Name;                 ///< @brief 文件名
protected:
	/// @brief     从流读取和Xml数据中读取流
	/// @param[in] pStream  流
	/// @param[in] pXmlNode Xml节点
	virtual fResult Read(fcyStream* pStream, fcyXmlElement* pXmlNode)=0;
public: // 接口转换
	virtual fcyResPackageDataNode* ToDataNode()=0;    ///< @brief 转到数据节点
	virtual fcyResPackageFloderNode* ToFloderNode()=0;///< @brief 转到文件夹节点
public:
	fcyResPackageFloderNode* GetParent();  ///< @brief 获得父节点
	fcStrW GetName();                      ///< @brief 返回键名
protected: // 限制改操作
	fResult SetName(fcStrW Name);          ///< @brief 设置键名
protected:
	/// @brief     构造函数
	/// @param[in] pParent 父节点
	/// @param[in] Name    键名
	fcyResPackageNode(fcyResPackageFloderNode* pParent, const std::wstring& Name);
	virtual ~fcyResPackageNode();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 资源包数据节点
////////////////////////////////////////////////////////////////////////////////
class fcyResPackageDataNode : 
	public fcyResPackageNode
{
	friend class fcyResPackageFloderNode;
private:
	fcyStream* m_pStream;       ///< @brief 保存原始流

	// 属性
	fuInt m_DataOffset;         ///< @brief 相对主缓冲区的偏移
	fuInt m_DataRealSize;       ///< @brief 数据真实大小
	fuInt m_DataCompressedSize; ///< @brief 数据压缩后大小
	fBool m_bDataCompressed;    ///< @brief 是否被压缩

	// 附加数据
	std::wstring m_AdditionStr; ///< @brief 额外数据
public:
	fuInt GetDataOffset();           ///< @brief 返回相对主缓冲区的偏移
	fuInt GetDataRealSize();         ///< @brief 返回数据真实大小
	fuInt GetDataCompressedSize();   ///< @brief 返回数据压缩后大小
	fBool IsDataCompressed();        ///< @brief 返回数据是否被压缩

	/// @brief      导出流
	/// @param[out] pOut 存放导出流的指针
	/// @return     返回操作是否成功
	fResult DumpStream(fcyStream** pOut);

	fcStrW GetAdditonStr();          ///< @brief 返回额外字段
	void SetAdditionStr(fcStrW Src); ///< @brief 设置额外字段
protected:
	fResult Read(fcyStream* pStream, fcyXmlElement* pXmlNode);
public:
	fcyResPackageDataNode* ToDataNode();
	fcyResPackageFloderNode* ToFloderNode();
protected:
	/// @brief     构造函数
	/// @param[in] pParent 父节点
	/// @param[in] Name 键名
	fcyResPackageDataNode(fcyResPackageFloderNode* pParent, const std::wstring& Name);
	~fcyResPackageDataNode();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 资源包文件夹节点
////////////////////////////////////////////////////////////////////////////////
class fcyResPackageFloderNode : public fcyResPackageNode
{
	friend class fcyResPackage;
private:
	std::vector<fcyResPackageNode*> m_NodeList; ///< @brief 节点列表
public:
	fuInt GetNodeCount();                       ///< @brief 获得节点数目
	fcyResPackageNode* GetNode(fuInt Index);    ///< @brief 通过索引获得节点
	fcyResPackageNode* GetNode(fcStrW Name);    ///< @brief 获得节点
	fBool Contains(fcStrW Name);                ///< @brief 节点是否存在
protected: // 限制改操作
	fBool Append(fcyResPackageNode* pNode);     ///< @brief 追加节点
	fBool Remove(fcStrW Name);                  ///< @brief 移除节点
	void Clear();                               ///< @brief 清空并释放内存
protected:
	fResult Read(fcyStream* pStream, fcyXmlElement* pXmlNode);
public:
	fcyResPackageDataNode* ToDataNode();
	fcyResPackageFloderNode* ToFloderNode();
protected:
	/// @brief     构造函数
	/// @param[in] pParent 父节点
	/// @param[in] Name    键名
	fcyResPackageFloderNode(fcyResPackageFloderNode* pParent, const std::wstring& Name);
	~fcyResPackageFloderNode();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 资源包文件
////////////////////////////////////////////////////////////////////////////////
class fcyResPackage :
	public fcyRefObjImpl<fcyRefObj>
{
private:
	fcyResPackageFloderNode m_Root;  ///< @brief 根节点

	// 数据头
	fuInt m_Flag;        ///< @brief 文件标识符
    fuInt m_Version;     ///< @brief 文件版本号
    fuInt m_IndexOffset; ///< @brief 文件索引区位移
private:
	/// @brief     从流中加载文件
	/// @param[in] 流
	/// @return    返回操作结果
	fResult loadFromStream(fcyStream* pStream);
public:
	fcyResPackageFloderNode* GetRoot(); ///< @brief 获得根节点
public:
	/// @brief     构造函数
	/// @param[in] pStream 流
	fcyResPackage(fcyStream* pStream);
protected:
	~fcyResPackage(void);
};
/// @}