#include "Engine/f2dFileSysImpl.h"

#include "fcyOS/fcyDebug.h"
#include "fcyMisc/fcyStringHelper.h"

#include <Windows.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
f2dFileFolderNodeForReal::f2dFileFolderNodeForReal(fcStrW NodeName, fcStrW DirPath)
	: m_Name(NodeName), m_DirPath(DirPath)
{
	// 遍历所有文件文件夹
	WIN32_FIND_DATA tFindData;
	HANDLE hFind = FindFirstFile((m_DirPath + L"\\*.*").c_str(), &tFindData);
	if(hFind == INVALID_HANDLE_VALUE)
		throw fcyWin32Exception("f2dFileFolderNodeForReal::f2dFileFolderNodeForReal", "FindFirstFile Faild.");
	
	do
	{
		wstring tFileName = 
			fcyStringHelper::ToLower(tFindData.cFileName);

		if(tFileName == L".")
			continue;
		if(tFileName == L"..")
			continue;

		if(tFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 文件夹节点
			f2dFileFolderNodeForReal* tRealNode = new f2dFileFolderNodeForReal(
				tFindData.cFileName, 
				(m_DirPath + L"\\" + tFindData.cFileName).c_str());
			m_NodeList[tFileName] = tRealNode;
			m_SubNodes.push_back(tRealNode);
		}
		else
		{
			// 文件节点
			f2dFileStreamNodeForReal* tRealNode = new f2dFileStreamNodeForReal(
				tFindData.cFileName,
				(m_DirPath + L"\\" + tFindData.cFileName).c_str());
			m_NodeList[tFileName] = tRealNode;
			m_SubNodes.push_back(tRealNode);
		}
	}
	while(FindNextFile(hFind, &tFindData));

	FindClose(hFind);
}

f2dFileFolderNodeForReal::~f2dFileFolderNodeForReal()
{
	vector<f2dFileNode*>::iterator i = m_SubNodes.begin();
	while( i != m_SubNodes.end() )
	{
		FCYSAFEKILL(*i);
		++i;
	}
	m_SubNodes.clear();
	m_NodeList.clear();
}

f2dFileFolderNode* f2dFileFolderNodeForReal::ToFolder()
{
	return this;
}

f2dFileStreamNode* f2dFileFolderNodeForReal::ToStream()
{
	return NULL;
}

fcStrW f2dFileFolderNodeForReal::GetName()
{
	return m_Name.c_str();
}

fResult f2dFileFolderNodeForReal::Clear()
{
	return FCYERR_ILLEGAL;
}

fResult f2dFileFolderNodeForReal::Add(f2dFileNode* pNode)
{
	return FCYERR_ILLEGAL;
}

fResult f2dFileFolderNodeForReal::Contain(fcStrW NodeName)
{
	return (m_NodeList.find(fcyStringHelper::ToLower(NodeName)) == m_NodeList.end())?FCYERR_OBJNOTEXSIT : FCYERR_OBJEXSITED;
}

fResult f2dFileFolderNodeForReal::Remove(fcStrW NodeName)
{
	return FCYERR_ILLEGAL;
}

f2dFileNode* f2dFileFolderNodeForReal::GetSubNode(fcStrW NodeName)
{
	unordered_map<std::wstring, f2dFileNode*>::iterator i = m_NodeList.find(fcyStringHelper::ToLower(NodeName));

	if(i == m_NodeList.end())
		return NULL;
	else
		return i->second;
}

f2dFileNode* f2dFileFolderNodeForReal::GetSubNode(fuInt Index)
{
	if(Index >= GetNodeCount())
		return NULL;
	else
		return m_SubNodes[Index];
}

fuInt f2dFileFolderNodeForReal::GetNodeCount()
{
	return m_SubNodes.size();
}

////////////////////////////////////////////////////////////////////////////////

f2dFileStreamNodeForReal::f2dFileStreamNodeForReal(fcStrW NodeName, fcStrW FilePath)
	: m_Name(NodeName), m_FileName(FilePath), m_pStream(NULL)
{
}

f2dFileStreamNodeForReal::~f2dFileStreamNodeForReal()
{
	FCYSAFEKILL(m_pStream);
}

f2dFileFolderNode* f2dFileStreamNodeForReal::ToFolder()
{
	return NULL;
}

f2dFileStreamNode* f2dFileStreamNodeForReal::ToStream()
{
	return this;
}

fcStrW f2dFileStreamNodeForReal::GetName()
{
	return m_Name.c_str();
}

f2dStream* f2dFileStreamNodeForReal::GetStream()
{
	if(!m_pStream)
	{
		try
		{
			m_pStream = new fcyFileStream(m_FileName.c_str(), true);
		}
		catch(...)
		{
			try
			{
				m_pStream = new fcyFileStream(m_FileName.c_str(), false);
			}
			catch(const fcyException& e)
			{
				FCYDEBUGEXCPT(e);
				return NULL;
			}
		}
	}
	
	return m_pStream;
}

fResult f2dFileStreamNodeForReal::CloseStream()
{
	FCYSAFEKILL(m_pStream);
	return FCYERR_OK;
}

fcStrW f2dFileStreamNodeForReal::GetAdditionData()
{
	return m_AdditionData.c_str();
}

fResult f2dFileStreamNodeForReal::SetAdditionData(fcStrW Str)
{
	if(Str == NULL)
		Str = L"";
	m_AdditionData = Str;
	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

f2dFileFolderNodeForPackage::f2dFileFolderNodeForPackage(fcyResPackage* pPackage, fcyResPackageFloderNode* pNode)
	: m_pPackage(pPackage), m_pNode(pNode)
{
	pPackage->AddRef();

	m_FloderName = pNode->GetName();
}

f2dFileFolderNodeForPackage::f2dFileFolderNodeForPackage(fcyResPackage* pPackage, fcyResPackageFloderNode* pNode, const std::wstring& NameOverride)
	: m_pPackage(pPackage), m_pNode(pNode), m_FloderName(NameOverride)
{
	pPackage->AddRef();
}

f2dFileFolderNodeForPackage::~f2dFileFolderNodeForPackage()
{
	unordered_map<std::wstring, f2dFileNode*>::iterator i = m_NodesCache.begin();
	while(i != m_NodesCache.end())
	{
		FCYSAFEKILL(i->second);
		++i;
	}
	m_NodesCache.clear();

	FCYSAFEKILL(m_pPackage);
}

f2dFileFolderNode* f2dFileFolderNodeForPackage::ToFolder()
{
	return this;
}

f2dFileStreamNode* f2dFileFolderNodeForPackage::ToStream()
{
	return NULL;
}

fcStrW f2dFileFolderNodeForPackage::GetName()
{
	return m_FloderName.c_str();
}
	
fResult f2dFileFolderNodeForPackage::Clear()
{
	return FCYERR_ILLEGAL;
}

fResult f2dFileFolderNodeForPackage::Add(f2dFileNode* pNode)
{
	return FCYERR_ILLEGAL;
}

fResult f2dFileFolderNodeForPackage::Contain(fcStrW NodeName)
{
	return m_pNode->Contains(NodeName)?FCYERR_OBJEXSITED : FCYERR_OBJNOTEXSIT;
}

fResult f2dFileFolderNodeForPackage::Remove(fcStrW NodeName)
{
	return FCYERR_ILLEGAL;
}

f2dFileNode* f2dFileFolderNodeForPackage::GetSubNode(fcStrW NodeName)
{
	if(m_NodesCache.find(NodeName) != m_NodesCache.end())
		return m_NodesCache[NodeName];

	// 是否存在
	if(m_pNode->Contains(NodeName))
	{
		fcyResPackageNode* tNode = m_pNode->GetNode(NodeName);

		if(tNode->ToDataNode())
		{
			m_NodesCache[NodeName] = new f2dFileStreamNodeForPackage(m_pPackage, tNode->ToDataNode());
		}
		else
		{
			m_NodesCache[NodeName] = new f2dFileFolderNodeForPackage(m_pPackage, tNode->ToFloderNode());
		}
		return m_NodesCache[NodeName];
	}
	else
		return NULL;
}

f2dFileNode* f2dFileFolderNodeForPackage::GetSubNode(fuInt Index)
{
	fcyResPackageNode* pNode = m_pNode->GetNode(Index);
	if(pNode)
		return GetSubNode(pNode->GetName());
	else
		return NULL;
}

fuInt f2dFileFolderNodeForPackage::GetNodeCount()
{
	return m_pNode->GetNodeCount();
}

////////////////////////////////////////////////////////////////////////////////

f2dFileStreamNodeForPackage::f2dFileStreamNodeForPackage(fcyResPackage* pPackage, fcyResPackageDataNode* pNode)
	: m_pPackage(pPackage), m_pNode(pNode), m_pStream(NULL)
{
	pPackage->AddRef();
}

f2dFileStreamNodeForPackage::~f2dFileStreamNodeForPackage()
{
	FCYSAFEKILL(m_pStream);
	FCYSAFEKILL(m_pPackage);
}

f2dFileFolderNode* f2dFileStreamNodeForPackage::ToFolder()
{
	return NULL;
}

f2dFileStreamNode* f2dFileStreamNodeForPackage::ToStream()
{
	return this;
}

fcStrW f2dFileStreamNodeForPackage::GetName()
{
	return m_pNode->GetName();
}

f2dStream* f2dFileStreamNodeForPackage::GetStream()
{
	if(!m_pStream)
	{
		if(FCYFAILED(m_pNode->DumpStream(&m_pStream)))
			return NULL;
	}
	return m_pStream;
}

fResult f2dFileStreamNodeForPackage::CloseStream()
{
	if(m_pStream == NULL)
		return FCYERR_ILLEGAL;
	FCYSAFEKILL(m_pStream);
	return FCYERR_OK;
}

fcStrW f2dFileStreamNodeForPackage::GetAdditionData()
{
	return m_pNode->GetAdditonStr();
}

fResult f2dFileStreamNodeForPackage::SetAdditionData(fcStrW Str)
{
	if(Str == NULL)
		m_pNode->SetAdditionStr(L"");
	else
		m_pNode->SetAdditionStr(Str);
	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

f2dFileFolderNodeImpl::f2dFileFolderNodeImpl(fcStrW NodeName)
	: m_Name(NodeName)
{
}

f2dFileFolderNodeImpl::~f2dFileFolderNodeImpl()
{
	Clear();
}

f2dFileFolderNode* f2dFileFolderNodeImpl::ToFolder()
{
	return this;
}

f2dFileStreamNode* f2dFileFolderNodeImpl::ToStream()
{
	return NULL;
}

fcStrW f2dFileFolderNodeImpl::GetName()
{
	return m_Name.c_str();
}

fResult f2dFileFolderNodeImpl::Clear()
{
	// 清除所有节点
	unordered_map<wstring, f2dFileNode*>::iterator i = 
		m_NodeList.begin();

	while(i != m_NodeList.end())
	{
		i->second->Release();
		++i;
	}

	m_NodeList.clear();
	m_pList.clear();

	return FCYERR_OK;
}

fResult f2dFileFolderNodeImpl::Add(f2dFileNode* pNode)
{
	if(!pNode->GetName() || wcslen(pNode->GetName()) == 0 || !pNode)
		return FCYERR_INVAILDPARAM;
	if(Contain(pNode->GetName()) == FCYERR_OBJEXSITED)
		return FCYERR_OBJEXSITED;

	m_NodeList[pNode->GetName()] = pNode;
	m_pList.push_back(pNode);
	pNode->AddRef();

	return FCYERR_OK;
}

fResult f2dFileFolderNodeImpl::Contain(fcStrW NodeName)
{
	if(!NodeName || wcslen(NodeName) == 0)
		return FCYERR_INVAILDPARAM;

	unordered_map<wstring, f2dFileNode*>::iterator i = 
		m_NodeList.find(NodeName);

	return i!=m_NodeList.end()?FCYERR_OBJEXSITED : FCYERR_OBJNOTEXSIT;
}

fResult f2dFileFolderNodeImpl::Remove(fcStrW NodeName)
{
	if(!NodeName || wcslen(NodeName) == 0)
		return FCYERR_INVAILDPARAM;

	unordered_map<wstring, f2dFileNode*>::iterator i = 
		m_NodeList.find(NodeName);

	if(i == m_NodeList.end())
		return FCYERR_OBJNOTEXSIT;

	vector<f2dFileNode*>::iterator j = m_pList.begin();
	while(j != m_pList.end())
	{
		if((*j) == i->second)
		{
			m_pList.erase(j);
			break;
		}
	}

	i->second->Release();
	m_NodeList.erase(i);

	return FCYERR_OK;
}

f2dFileNode* f2dFileFolderNodeImpl::GetSubNode(fcStrW NodeName)
{
	if(Contain(NodeName) != FCYERR_OBJEXSITED)
		return NULL;

	return m_NodeList[NodeName];
}

f2dFileNode* f2dFileFolderNodeImpl::GetSubNode(fuInt Index)
{
	if(Index >= GetNodeCount())
		return NULL;
	else
		return m_pList[Index];
}

fuInt f2dFileFolderNodeImpl::GetNodeCount()
{
	return m_pList.size();
}

////////////////////////////////////////////////////////////////////////////////

f2dFileStreamNodeImpl::f2dFileStreamNodeImpl(fcStrW NodeName, fcyStream* pStream)
	: m_Name(NodeName), m_pStream(pStream)
{
	if(!pStream)
		throw fcyException("f2dFileStreamNodeImpl::f2dFileStreamNodeImpl", "Invalid Pointer.");

	m_pStream->AddRef();
}

f2dFileStreamNodeImpl::~f2dFileStreamNodeImpl()
{
	FCYSAFEKILL(m_pStream);
}

f2dFileFolderNode* f2dFileStreamNodeImpl::ToFolder()
{
	return NULL;
}

f2dFileStreamNode* f2dFileStreamNodeImpl::ToStream()
{
	return this;
}

fcStrW f2dFileStreamNodeImpl::GetName()
{
	return m_Name.c_str();
}

f2dStream* f2dFileStreamNodeImpl::GetStream()
{
	return m_pStream;
}

fResult f2dFileStreamNodeImpl::CloseStream()
{
	return FCYERR_ILLEGAL;
}

fcStrW f2dFileStreamNodeImpl::GetAdditionData()
{
	return m_AdditionStr.c_str();
}

fResult f2dFileStreamNodeImpl::SetAdditionData(fcStrW Str)
{
	if(Str == NULL)
		m_AdditionStr = L"";
	else
		m_AdditionStr = Str;
	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

f2dFileSysImpl::f2dFileSysImpl()
{
	m_pRootNode = new f2dFileFolderNodeImpl(L"");
}

f2dFileSysImpl::~f2dFileSysImpl()
{
	FCYSAFEKILL(m_pRootNode);
}

f2dFileNode* f2dFileSysImpl::traverseNode(fcStrW NodePath, f2dFileFolderNode** pParent)
{
	// 分割路径
	vector<wstring> tPathList;
	fcyStringHelper::StringSplit(NodePath, L"\\", true, tPathList);

	f2dFileNode* pNode = GetRootNode();
	f2dFileFolderNode* pParentNode = GetRootNode();
	for(fuInt i = 0; i<tPathList.size(); ++i)
	{
		pParentNode = pNode->ToFolder();
		if(!pParentNode)
			break;

		pNode = pParentNode->GetSubNode(tPathList[i].c_str());
		if(!pNode) // 不存在该节点
		{
			pParentNode = NULL;
			break;
		}
	}

	if(pParent)
		*pParent = pParentNode;

	// 是否找到父节点
	if(!pParentNode)
		return NULL;
	else
		return pNode;
}

f2dFileNode* f2dFileSysImpl::traverseNode(fcStrW NodePath, wstring& NodeName, f2dFileFolderNode** pParent)
{
	// 分割路径
	vector<wstring> tPathList;
	fcyStringHelper::StringSplit(NodePath, L"\\", true, tPathList);

	if(tPathList.size())
	{
		NodeName = tPathList[tPathList.size()-1];
	}
	else
		NodeName.clear();

	f2dFileNode* pNode = GetRootNode();
	f2dFileFolderNode* pParentNode = GetRootNode();
	for(fuInt i = 0; i<tPathList.size(); ++i)
	{
		pParentNode = pNode->ToFolder();
		if(!pParentNode)
			break;

		pNode = pParentNode->GetSubNode(tPathList[i].c_str());
		if(!pNode) // 不存在该节点
		{
			pParentNode = NULL;
			break;
		}
	}

	if(pParent)
		*pParent = pParentNode;

	// 是否找到父节点
	if(!pParentNode)
		return NULL;
	else
		return pNode;
}

fResult f2dFileSysImpl::makeNode(fcStrW NodePath, fBool MakeDir, std::wstring& NodeName, f2dFileFolderNode** pParent)
{
	// 分割路径
	vector<wstring> tPathList;
	fcyStringHelper::StringSplit(NodePath, L"\\", true, tPathList);

	if(tPathList.size())
	{
		NodeName = tPathList[tPathList.size()-1];
	}
	else
	{
		NodeName.clear();
		return FCYERR_INVAILDPARAM;
	}

	f2dFileNode* pNode = GetRootNode();
	f2dFileFolderNode* pParentNode = GetRootNode();
	for(fuInt i = 0; i<tPathList.size()-1; ++i) // 遍历到倒数第二层
	{
		pParentNode = pNode->ToFolder();
		if(!pParentNode)
			return FCYERR_INVAILDPARAM;  // 中间无效点

		pNode = pParentNode->GetSubNode(tPathList[i].c_str());
		if(!pNode) // 不存在该节点
		{
			if(MakeDir) // 创建节点
			{
				f2dFileFolderNode* pTempNode = new f2dFileFolderNodeImpl(tPathList[i].c_str());
				if(FCYFAILED(pParentNode->Add(pTempNode)))
				{
					FCYSAFEKILL(pTempNode);
					return FCYERR_INTERNALERR;
				}
				pNode = pTempNode;
				FCYSAFEKILL(pTempNode);
			}
			else
			{
				return FCYERR_OBJNOTEXSIT;
			}
		}
	}

	// 检查pNode
	if(!pNode->ToFolder())
		return FCYERR_INVAILDPARAM;  // 中间无效点

	if(pParent)
		*pParent = pNode->ToFolder();

	return FCYERR_OK;
}

f2dFileFolderNode* f2dFileSysImpl::GetRootNode()
{
	return m_pRootNode;
}

f2dFileNode* f2dFileSysImpl::GetNode(fcStrW Path)
{
	return traverseNode(Path, NULL);
}

fResult f2dFileSysImpl::RemoveNode(fcStrW Path)
{
	f2dFileFolderNode* pParent = NULL;
	wstring tStr;

	if(traverseNode(Path, tStr, &pParent))
	{
		return pParent->Remove(tStr.c_str());
	}
	else
		return FCYERR_OBJNOTEXSIT;
}

fResult f2dFileSysImpl::AddFolderNode(fcStrW Path)
{
	wstring tNodeName;
	f2dFileFolderNode* pParent = NULL;
	
	fResult tVR = makeNode(Path, true, tNodeName, &pParent);
	if(FCYFAILED(tVR))
		return tVR;
	else
	{
		f2dFileFolderNode* pNewNode = new f2dFileFolderNodeImpl(tNodeName.c_str());
		if(FCYFAILED(pParent->Add(pNewNode)))
		{
			FCYSAFEKILL(pNewNode);
			return FCYERR_INTERNALERR;
		}
		FCYSAFEKILL(pNewNode);
		return FCYERR_OK;
	}
}

fResult f2dFileSysImpl::AddMemStreamNode(fcStrW Path, fcData Src, fLen Length, fBool Writable, fBool Resizable)
{
	wstring tNodeName;
	f2dFileFolderNode* pParent = NULL;
	
	fResult tVR = makeNode(Path, true, tNodeName, &pParent);
	if(FCYFAILED(tVR))
		return tVR;
	else
	{
		fcyStream* pNewNode = NULL;
		try
		{
			pNewNode = new fcyMemStream(Src, Length, Writable, Resizable);
		}
		catch(const fcyException& e)
		{
			FCYDEBUGEXCPT(e);
			return FCYERR_INTERNALERR;
		}
		f2dFileStreamNode* pStreamNode = new f2dFileStreamNodeImpl(tNodeName.c_str(), pNewNode);
		if(FCYFAILED(pParent->Add(pStreamNode)))
		{
			FCYSAFEKILL(pNewNode);
			FCYSAFEKILL(pStreamNode);
			return FCYERR_INTERNALERR;
		}
		FCYSAFEKILL(pNewNode);
		FCYSAFEKILL(pStreamNode);
		return FCYERR_OK;
	}
}

fResult f2dFileSysImpl::AddMemStreamNodeFromFile(fcStrW Path, fcStrW FilePath, fBool Writable, fBool Resizable)
{
	fcyFileStream* pStream = NULL;
	
	// 打开文件
	try
	{
		pStream = new fcyFileStream(FilePath, false);
	}
	catch(const fcyException& e)
	{
		FCYDEBUGEXCPT(e);
		return FCYERR_INTERNALERR;
	}

	// 分配内存
	fData pMem = NULL;
	fLen tLen = pStream->GetLength();
	try
	{
		pMem = new fByte[(size_t)tLen];
	}
	catch(const bad_alloc&)
	{
		fcyDebug::Trace("[ TimeTick = %d ] std::bad_alloc @f2dFileSysImpl::AddMemStreamNodeFromFile : Out Of Mem\n", GetTickCount());
		FCYSAFEKILL(pStream);
		return FCYERR_OUTOFMEM;
	}

	// 读取数据到内存
	if(FAILED(pStream->SetPosition(FCYSEEKORIGIN_BEG, 0)))
	{
		FCYSAFEDELARR(pMem);
		FCYSAFEKILL(pStream);
		return FCYERR_INTERNALERR;
	}
	if(FAILED(pStream->ReadBytes(pMem, tLen, NULL)))
	{
		FCYSAFEDELARR(pMem);
		FCYSAFEKILL(pStream);
		return FCYERR_INTERNALERR;
	}

	// 销毁文件流
	FCYSAFEKILL(pStream);

	// 创建内存流
	fResult tVR = AddMemStreamNode(Path, pMem, tLen, Writable, Resizable);

	// 销毁内存
	FCYSAFEDELARR(pMem);

	return tVR;
}

fResult f2dFileSysImpl::AddFileStreamNode(fcStrW Path, fcStrW FilePath, fBool Writable)
{
	wstring tNodeName;
	f2dFileFolderNode* pParent = NULL;
	
	fResult tVR = makeNode(Path, true, tNodeName, &pParent);
	if(FCYFAILED(tVR))
		return tVR;
	else
	{
		fcyStream* pNewNode = NULL;
		try
		{
			pNewNode = new fcyFileStream(FilePath, Writable);
		}
		catch(const fcyException& e)
		{
			FCYDEBUGEXCPT(e);
			return FCYERR_INTERNALERR;
		}
		f2dFileStreamNode* pStreamNode = new f2dFileStreamNodeImpl(tNodeName.c_str(), pNewNode);
		if(FCYFAILED(pParent->Add(pStreamNode)))
		{
			FCYSAFEKILL(pNewNode);
			FCYSAFEKILL(pStreamNode);
			return FCYERR_INTERNALERR;
		}
		FCYSAFEKILL(pNewNode);
		FCYSAFEKILL(pStreamNode);
		return FCYERR_OK;
	}
}

f2dStream* f2dFileSysImpl::GetStream(fcStrW Path)
{
	f2dFileNode* pNode = GetNode(Path);
	if(!pNode || !pNode->ToStream())
		return NULL;
	else
	{
		return pNode->ToStream()->GetStream();
	}
}

fResult f2dFileSysImpl::LoadResPackage(fcStrW Path, fcyStream* PakStream)
{
	// 创建Package
	fcyResPackage* tPackage = NULL;
	try
	{
		tPackage = new fcyResPackage(PakStream);
	}
	catch(const fcyException& e)
	{
		FCYDEBUGEXCPT(e);
		return FCYERR_INTERNALERR;
	}

	wstring tNodeName;
	f2dFileFolderNode* pParent = NULL;
	
	fResult tVR = makeNode(Path, true, tNodeName, &pParent);
	if(FCYFAILED(tVR))
	{
		FCYSAFEKILL(tPackage);
		return tVR;
	}
	else
	{
		f2dFileFolderNodeForPackage* pNewNode = new f2dFileFolderNodeForPackage(tPackage, tPackage->GetRoot(), tNodeName);
		
		if(FCYFAILED(pParent->Add(pNewNode)))
		{
			FCYSAFEKILL(pNewNode);
			FCYSAFEKILL(tPackage);
			return FCYERR_INTERNALERR;
		}
		FCYSAFEKILL(pNewNode);
		FCYSAFEKILL(tPackage);
		return FCYERR_OK;
	}
}

fResult f2dFileSysImpl::LoadResPackage(fcStrW Path, fcStrW PakFilePath)
{
	fcyStream* pFile = NULL;
	try
	{
		pFile = new fcyFileStream(PakFilePath, false);
	}
	catch(const fcyException& e)
	{
		FCYDEBUGEXCPT(e);
		return FCYERR_INTERNALERR;
	}

	fResult tVR = LoadResPackage(Path, pFile);

	FCYSAFEKILL(pFile);

	return tVR;
}

fResult f2dFileSysImpl::LoadRealPath(fcStrW Path, fcStrW DirPath)
{
	wstring tNodeName;
	f2dFileFolderNode* pParent = NULL;
	
	fResult tVR = makeNode(Path, true, tNodeName, &pParent);
	if(FCYFAILED(tVR))
	{
		return tVR;
	}
	else
	{
		// 创建真实目录
		f2dFileFolderNodeForReal* tDir = NULL;
		try
		{
			tDir = new f2dFileFolderNodeForReal(tNodeName.c_str(), DirPath);
		}
		catch(const fcyException& e)
		{
			FCYDEBUGEXCPT(e);
			return FCYERR_INTERNALERR;
		}

		if(FCYFAILED(pParent->Add(tDir)))
		{
			FCYSAFEKILL(tDir);
			return FCYERR_INTERNALERR;
		}
		FCYSAFEKILL(tDir);
		return FCYERR_OK;
	}
}
