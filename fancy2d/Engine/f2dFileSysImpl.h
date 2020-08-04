////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFileSysImpl.h
/// @brief fancy2D文件系统接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#include "f2dFileSys.h"

#include <fcyFile/fcyResPackage.h>

#include <string>
#include <vector>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
/// @brief 真实文件夹节点
////////////////////////////////////////////////////////////////////////////////
class f2dFileFolderNodeForReal : 
	public fcyRefObjImpl<f2dFileFolderNode>
{
	friend class f2dFileSysImpl;
private:
	std::wstring m_Name;                                        ///< @brief 节点名
	std::wstring m_DirPath;                                     ///< @brief 文件夹路径
	std::vector<f2dFileNode*> m_SubNodes;                       ///< @brief 缓存所有子节点
	std::unordered_map<std::wstring, f2dFileNode*> m_NodeList;  ///< @brief 加速查找过程
public: // 接口实现
	f2dFileFolderNode* ToFolder();
	f2dFileStreamNode* ToStream();

	fcStrW GetName();
	
	fResult Clear();
	fResult Add(f2dFileNode* pNode);
	fResult Contain(fcStrW NodeName);
	fResult Remove(fcStrW NodeName);
	f2dFileNode* GetSubNode(fcStrW NodeName);
	f2dFileNode* GetSubNode(fuInt Index);
	fuInt GetNodeCount();
protected:
	f2dFileFolderNodeForReal(fcStrW NodeName, fcStrW DirPath);
	~f2dFileFolderNodeForReal();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 真实文件节点
////////////////////////////////////////////////////////////////////////////////
class f2dFileStreamNodeForReal :
	public fcyRefObjImpl<f2dFileStreamNode>
{
	friend class f2dFileFolderNodeForReal;
private:
	std::wstring m_Name;          ///< @brief 节点名
	std::wstring m_FileName;      ///< @brief 文件名
	std::wstring m_AdditionData;  ///< @brief 额外数据
	fcyStream* m_pStream;         ///< @brief 数据流
public: // 接口实现
	f2dFileFolderNode* ToFolder();
	f2dFileStreamNode* ToStream();

	fcStrW GetName();

	f2dStream* GetStream();
	fResult CloseStream();
	fcStrW GetAdditionData();
	fResult SetAdditionData(fcStrW Str);
protected:
	f2dFileStreamNodeForReal(fcStrW NodeName, fcStrW FilePath);
	~f2dFileStreamNodeForReal();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy资源包文件夹节点
////////////////////////////////////////////////////////////////////////////////
class f2dFileFolderNodeForPackage : 
	public fcyRefObjImpl<f2dFileFolderNode>
{
	friend class f2dFileSysImpl;
private:
	std::wstring m_FloderName;         ///< @brief 保存文件夹节点名字

	fcyResPackage* m_pPackage;         ///< @brief 保持Package
	fcyResPackageFloderNode* m_pNode;  ///< @brief 本文件夹的节点

	std::unordered_map<std::wstring, f2dFileNode*> m_NodesCache;  ///< @brief 缓存
public: // 接口实现
	f2dFileFolderNode* ToFolder();
	f2dFileStreamNode* ToStream();
	
	fcStrW GetName();

	fResult Clear();
	fResult Add(f2dFileNode* pNode);
	fResult Contain(fcStrW NodeName);
	fResult Remove(fcStrW NodeName);
	f2dFileNode* GetSubNode(fcStrW NodeName);
	f2dFileNode* GetSubNode(fuInt Index);
	fuInt GetNodeCount();
protected:
	f2dFileFolderNodeForPackage(fcyResPackage* pPackage, fcyResPackageFloderNode* pNode);
	f2dFileFolderNodeForPackage(fcyResPackage* pPackage, fcyResPackageFloderNode* pNode, const std::wstring& NameOverride);
	~f2dFileFolderNodeForPackage();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy资源包文件节点
////////////////////////////////////////////////////////////////////////////////
class f2dFileStreamNodeForPackage :
	public fcyRefObjImpl<f2dFileStreamNode>
{
	friend class f2dFileFolderNodeForPackage;
private:
	fcyResPackage* m_pPackage;       ///< @brief 资源包指针
	fcyResPackageDataNode* m_pNode;  ///< @brief 本节点指针

	fcyStream* m_pStream;            ///< @brief 数据流
public: // 接口实现
	f2dFileFolderNode* ToFolder();
	f2dFileStreamNode* ToStream();

	fcStrW GetName();

	f2dStream* GetStream();
	fResult CloseStream();
	fcStrW GetAdditionData();
	fResult SetAdditionData(fcStrW Str);
protected:
	f2dFileStreamNodeForPackage(fcyResPackage* pPackage, fcyResPackageDataNode* pNode);
	~f2dFileStreamNodeForPackage();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy虚拟文件夹节点
////////////////////////////////////////////////////////////////////////////////
class f2dFileFolderNodeImpl : 
	public fcyRefObjImpl<f2dFileFolderNode>
{
	friend class f2dFileSysImpl;
private:
	std::wstring m_Name;                                       ///< @brief 节点名
	std::unordered_map<std::wstring, f2dFileNode*> m_NodeList; ///< @brief 存放所有节点
	std::vector<f2dFileNode*> m_pList;                         ///< @brief 索引缓存
public: // 接口实现
	f2dFileFolderNode* ToFolder();
	f2dFileStreamNode* ToStream();
	
	fcStrW GetName();

	fResult Clear();
	fResult Add(f2dFileNode* pNode);
	fResult Contain(fcStrW NodeName);
	fResult Remove(fcStrW NodeName);
	f2dFileNode* GetSubNode(fcStrW NodeName);
	f2dFileNode* GetSubNode(fuInt Index);
	fuInt GetNodeCount();
protected:
	f2dFileFolderNodeImpl(fcStrW NodeName);
	~f2dFileFolderNodeImpl();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy虚拟文件节点
////////////////////////////////////////////////////////////////////////////////
class f2dFileStreamNodeImpl :
	public fcyRefObjImpl<f2dFileStreamNode>
{
	friend class f2dFileSysImpl;
private:
	std::wstring m_Name;         ///< @brief 节点名
	fcyStream* m_pStream;        ///< @brief 流指针
	std::wstring m_AdditionStr;  ///< @brief 额外数据域
public: // 接口实现
	f2dFileFolderNode* ToFolder();
	f2dFileStreamNode* ToStream();

	fcStrW GetName();

	f2dStream* GetStream();
	fResult CloseStream();
	fcStrW GetAdditionData();
	fResult SetAdditionData(fcStrW Str);
protected:
	f2dFileStreamNodeImpl(fcStrW NodeName, fcyStream* pStream);
	~f2dFileStreamNodeImpl();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D文件系统
////////////////////////////////////////////////////////////////////////////////
class f2dFileSysImpl :
	public f2dFileSys
{
private:
	f2dFileFolderNode* m_pRootNode;
private:
	// 遍历并返回父节点
	f2dFileNode* traverseNode(fcStrW NodePath, f2dFileFolderNode** pParent);
	f2dFileNode* traverseNode(fcStrW NodePath, std::wstring& NodeName, f2dFileFolderNode** pParent);
	// 遍历到倒数第二层，并沿途创建Folder
	fResult makeNode(fcStrW NodePath, fBool MakeDir, std::wstring& NodeName, f2dFileFolderNode** pParent);
public: // 接口实现
	f2dFileFolderNode* GetRootNode();
	f2dFileNode* GetNode(fcStrW Path);
	fResult RemoveNode(fcStrW Path);
	fResult AddFolderNode(fcStrW Path);
	fResult AddMemStreamNode(fcStrW Path, fcData Src, fLen Length, fBool Writable, fBool Resizable);
	fResult AddMemStreamNodeFromFile(fcStrW Path, fcStrW FilePath, fBool Writable, fBool Resizable);
	fResult AddFileStreamNode(fcStrW Path, fcStrW FilePath, fBool Writable);

	f2dStream* GetStream(fcStrW Path);

	fResult LoadResPackage(fcStrW Path, fcyStream* PakStream);
	fResult LoadResPackage(fcStrW Path, fcStrW PakFilePath);

	fResult LoadRealPath(fcStrW Path, fcStrW DirPath);
public:
	f2dFileSysImpl();
	~f2dFileSysImpl();
};
