#include "RenderDev/f2dMeshDataImpl.h"

#include "RenderDev/f2dRenderDeviceImpl.h"
#include <ios>
#include <fstream>
#include <sstream>
#include <string.h>
#include <map>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dMeshDataImpl::f2dMeshDataImpl(f2dRenderDeviceImpl* pParent, f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index)
	: m_pParent(pParent), m_VertCount(VertCount), m_IndexCount(IndexCount), m_CurIndexSize(Int32Index ? 32:16), m_CurVertSize(0),
	m_pIB(NULL), m_pVB(NULL)
{
	m_pVD = m_pParent->RegisterVertexDeclare(pVertElement, ElementCount, m_CurVertSize);
	if(m_pVD == NULL)
		throw fcyException("f2dMeshDataImpl::f2dMeshDataImpl", "f2dRenderDeviceImpl::RegisterVertexDeclare failed.");

	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();
	HRESULT tHR;
	if(FAILED(tHR = pDev->CreateVertexBuffer(m_VertCount * m_CurVertSize, 0, 0, D3DPOOL_MANAGED, &m_pVB, NULL)))
		throw fcyWin32COMException("f2dMeshDataImpl::f2dMeshDataImpl", "IDirect3DDevice9::CreateVertexBuffer failed.", tHR);

	if(FAILED(tHR = pDev->CreateIndexBuffer(m_IndexCount * m_CurIndexSize, 0, Int32Index ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
	{
		FCYSAFEKILL(m_pVB);
		throw fcyWin32COMException("f2dMeshDataImpl::f2dMeshDataImpl", "IDirect3DDevice9::CreateIndexBuffer failed.", tHR);
	}
}

f2dMeshDataImpl::~f2dMeshDataImpl()
{
	FCYSAFEKILL(m_pVB);
	FCYSAFEKILL(m_pIB);
}

fResult f2dMeshDataImpl::Render(fInt ID)
{
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pParent->GetHandle();

	if(m_SubmeshCache.find(ID) == m_SubmeshCache.end())
		return FCYERR_OBJNOTEXSIT;

	f2dMeshSubsetInfo& tData = m_SubmeshCache[ID];

	if(FCYFAILED(pDev->SetStreamSource(0, m_pVB, 0, m_CurVertSize)))
		return FCYERR_INTERNALERR;
	if(FCYFAILED(pDev->SetIndices(m_pIB)))
		return FCYERR_INTERNALERR;
	if(FCYFAILED(pDev->SetVertexDeclaration(m_pVD)))
		return FCYERR_INTERNALERR;

	if(FCYFAILED(pDev->DrawIndexedPrimitive((D3DPRIMITIVETYPE)tData.Type, 0, 0, m_VertCount, tData.StartIndex, tData.PrimitiveCount)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}
