////////////////////////////////////////////////////////////////////////////////
/// @file  f2dMeshDataImpl.h
/// @brief fancy2D 模型数据实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dRenderDeviceAPI.h"

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
/// @brief 模型对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dMeshDataImpl :
	public fcyRefObjImpl<f2dMeshData>
{
	friend class f2dRenderDeviceImpl;
protected:
	f2dRenderDeviceImpl* m_pParent;

	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DIndexBuffer9* m_pIB;
	IDirect3DVertexDeclaration9* m_pVD;

	fuInt m_CurVertSize;
	fuInt m_CurIndexSize;
	fuInt m_VertCount;
	fuInt m_IndexCount;

	std::unordered_map<int, f2dMeshSubsetInfo> m_SubmeshCache;

public: // 接口实现
	fResult LockVertexData(fuInt StartVert, fuInt VertCount, void** pOut)
	{
		if(FAILED(m_pVB->Lock(StartVert * m_CurVertSize, (VertCount == 0 ? (m_VertCount - StartVert) : VertCount) * m_CurVertSize, pOut, 0)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult UnlockVertexData()
	{
		if(FAILED(m_pVB->Unlock()))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}

	fResult LockIndexData(fuInt StartIndex, fuInt IndexCount, void** pOut)
	{
		if(FAILED(m_pIB->Lock(StartIndex * m_CurIndexSize, (IndexCount == 0 ? (m_IndexCount - StartIndex) : IndexCount) * m_CurIndexSize, pOut, 0)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult UnlockIndexData()
	{
		if(FAILED(m_pIB->Unlock()))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}

	fResult GetSubset(fInt ID, f2dMeshSubsetInfo* pOut)
	{
		std::unordered_map<int, f2dMeshSubsetInfo>::iterator i = m_SubmeshCache.find(ID);

		if(i == m_SubmeshCache.end())
			return FCYERR_OBJNOTEXSIT;
		else
		{
			*pOut = i->second;
			return FCYERR_OK;
		}
	}
	fResult SetSubset(fInt ID, F2DPRIMITIVETYPE Type, fuInt StartIndex, fuInt PrimitiveCount)
	{
		f2dMeshSubsetInfo& tInfo = m_SubmeshCache[ID];

		tInfo.Type = Type;
		tInfo.StartIndex = StartIndex;
		tInfo.PrimitiveCount = PrimitiveCount;

		return FCYERR_OK;
	}
	fuInt GetSubsetCount()
	{
		return m_SubmeshCache.size();
	}

	fResult Render(fInt ID);
protected:
	f2dMeshDataImpl(f2dRenderDeviceImpl* pParent, f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index = false);
	~f2dMeshDataImpl();
};
