////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFileSys.h
/// @brief fancy2D文件系统接口定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

/// @addtogroup f2d文件系统
/// @brief fancy2d引擎中的文件系统
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D虚拟文件系统
////////////////////////////////////////////////////////////////////////////////
struct f2dFileFolderNode;
struct f2dFileStreamNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D文件系统节点
////////////////////////////////////////////////////////////////////////////////
struct f2dFileNode : public f2dInterface
{
	virtual f2dFileFolderNode* ToFolder()=0; ///< @brief 转换到文件夹节点（不增加引用计数）
	virtual f2dFileStreamNode* ToStream()=0; ///< @brief 转换到流节点（不增加引用计数）

	virtual fcStrW GetName()=0;              ///< @brief 获得节点名称
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D文件夹节点
////////////////////////////////////////////////////////////////////////////////
struct f2dFileFolderNode : public f2dFileNode
{
	/// @brief 清空所有子节点
	/// @note  清空所有子节点并减少引用计数
	virtual fResult Clear()=0;

	/// @brief     增加子节点
	/// @note      将一个节点增加到文件夹中，并增加其引用计数
	/// @param[in] pNode    节点指针
	virtual fResult Add(f2dFileNode* pNode)=0;
	
	/// @brief     是否包含子节点
	/// @param[in] NodeName 节点名称
	/// @return    FCYERR_OBJEXSITED：对象已存在；FCYERR_OBJNOTEXSIT：对象不存在
	virtual fResult Contain(fcStrW NodeName)=0;
	
	/// @brief     移除子节点
	/// @note      减少节点的引用计数
	/// @param[in] NodeName 节点名称
	virtual fResult Remove(fcStrW NodeName)=0;
	
	/// @brief     获得子节点
	/// @note      返回节点指针，不增加引用计数。
	/// @param[in] NodeName 节点名称
	/// @return    如果不存在返回NULL
	virtual f2dFileNode* GetSubNode(fcStrW NodeName)=0;
	
	/// @brief     获得子节点
	/// @note      返回节点指针，不增加引用计数。
	/// @param[in] Index 节点索引
	/// @return    如果不存在返回NULL
	virtual f2dFileNode* GetSubNode(fuInt Index)=0;
	
	/// @brief 获得节点数
	virtual fuInt GetNodeCount()=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D流节点
////////////////////////////////////////////////////////////////////////////////
struct f2dFileStreamNode : public f2dFileNode
{
	/// @brief 返回流接口
	/// @note  不增加引用计数
	virtual f2dStream* GetStream()=0;
	
	/// @brief 关闭流
	/// @note  仅对某些节点有效。对于关闭的流在调用GetStream时应该可以被重新加载。
	virtual fResult CloseStream()=0;
    
	/// @brief 返回额外数据
	/// @note  附加字段可以被用来储存额外的属性
	virtual fcStrW GetAdditionData()=0;
	
	/// @brief 设置额外数据
	virtual fResult SetAdditionData(fcStrW Str)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D文件系统
////////////////////////////////////////////////////////////////////////////////
struct f2dFileSys
{
	// --- 节点管理 ---
	/// @brief 获得根节点
	virtual f2dFileFolderNode* GetRootNode()=0;

	/// @brief     获得节点
	/// @param[in] Path 节点的路径
	virtual f2dFileNode* GetNode(fcStrW Path)=0;

	/// @brief     删除节点
	/// @param[in] Path 节点的路径
	virtual fResult RemoveNode(fcStrW Path)=0;

	/// @brief     创建文件夹节点
	/// @param[in] Path 节点的路径
	virtual fResult AddFolderNode(fcStrW Path)=0;

	/// @brief     创建内存流节点
	/// @param[in] Path      节点的路径
	/// @param[in] Src       数据源，可为NULL
	/// @param[in] Length    初始大小
	/// @param[in] Writable  可写
	/// @param[in] Resizable 可变长
	virtual fResult AddMemStreamNode(fcStrW Path, fcData Src, fLen Length, fBool Writable, fBool Resizable)=0;
	
	/// @brief     从文件创建内存流节点
	/// @note      将文件加载到内存并创建流节点
	/// @param[in] Path      节点的路径
	/// @param[in] FilePath  文件路径
	/// @param[in] Writable  可写
	/// @param[in] Resizable 可变长
	virtual fResult AddMemStreamNodeFromFile(fcStrW Path, fcStrW FilePath, fBool Writable, fBool Resizable)=0;

	/// @brief     创建文件流节点
	/// @note      挂接一个来自硬盘上的文件
	/// @param[in] Path      节点的路径
	/// @param[in] FilePath  文件路径
	/// @param[in] Writable  可写
	virtual fResult AddFileStreamNode(fcStrW Path, fcStrW FilePath, fBool Writable)=0;

	// --- 辅助操作 ---
	/// @brief  通过路径取得流
	/// @note   该操作不增加引用计数
	/// @return 成功返回流，否则返回NULL
	virtual f2dStream* GetStream(fcStrW Path)=0;

	// --- 包操作 ---
	/// @brief     将一个资源包加载到节点中
	/// @param[in] Path      路径
	/// @param[in] PakStream 数据流
	virtual fResult LoadResPackage(fcStrW Path, f2dStream* PakStream)=0;

	/// @brief     将一个资源包加载到节点中
	/// @param[in] Path        路径
	/// @param[in] PakFilePath 文件路径
	virtual fResult LoadResPackage(fcStrW Path, fcStrW PakFilePath)=0;

	// --- 真实目录映射 ---
	/// @brief     映射一个真实的文件夹到节点中
	/// @note      映射时会记录所有文件夹，对目录的任何操作将不会在映射后反映。
	/// @warning   真实目录节点不区分大小写
	/// @param[in] Path    路径
	/// @param[in] DirPath 文件夹路径
	virtual fResult LoadRealPath(fcStrW Path, fcStrW DirPath)=0;
};
/// @}
