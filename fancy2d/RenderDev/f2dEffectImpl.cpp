#include "RenderDev/f2dEffectImpl.h"

#include "RenderDev/f2dRenderDeviceImpl.h"

#include "fcyOS/fcyDebug.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dTexture* f2dEffectParamValueImpl::GetTexture()
{
	return ((f2dEffectImpl*)m_pParent)->GetCachedTex(m_pHandle);
}

fResult f2dEffectParamValueImpl::SetTexture(f2dTexture* pTex)
{
	F2DEPVTYPE tType = GetValueType();
	if( tType != F2DEPVTYPE_TEXTURE1D &&
		tType != F2DEPVTYPE_TEXTURE2D &&
		tType != F2DEPVTYPE_TEXTURE3D
		)
		return FCYERR_ILLEGAL;

	if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetTexture(m_pHandle, pTex ? (IDirect3DBaseTexture9*)pTex->GetHandle() : NULL)))
		return FCYERR_INTERNALERR;
	else
	{
		((f2dEffectImpl*)m_pParent)->SetCachedTex(m_pHandle, pTex);
		return FCYERR_OK;
	}
}

////////////////////////////////////////////////////////////////////////////////

f2dEffectParamImpl<f2dEffectParam>* f2dEffectFunctionImpl::GetCacheObj(D3DXHANDLE pHandle)
{
	if(pHandle == NULL)
		return NULL;
	if(m_ObjCache.find(pHandle) == m_ObjCache.end())
	{
		m_ObjCache[pHandle] = f2dEffectImpl::CreateEffectParam(m_pParent, pHandle);
	}

	return (f2dEffectParamImpl<f2dEffectParam>*)m_ObjCache[pHandle];
}

////////////////////////////////////////////////////////////////////////////////

f2dEffectParamImpl<f2dEffectParam>* f2dEffectPassImpl::GetCacheObj(D3DXHANDLE pHandle)
{
	if(pHandle == NULL)
		return NULL;
	if(m_ObjCache.find(pHandle) == m_ObjCache.end())
	{
		m_ObjCache[pHandle] = f2dEffectImpl::CreateEffectParam(m_pParent, pHandle);
	}

	return (f2dEffectParamImpl<f2dEffectParam>*)m_ObjCache[pHandle];
}

////////////////////////////////////////////////////////////////////////////////

f2dEffectParamImpl<f2dEffectParam>* f2dEffectTechniqueImpl::GetCacheObj(D3DXHANDLE pHandle)
{
	if(pHandle == NULL)
		return NULL;
	if(m_ObjCache.find(pHandle) == m_ObjCache.end())
	{
		m_ObjCache[pHandle] = f2dEffectImpl::CreateEffectParam(m_pParent, pHandle);
	}

	return (f2dEffectParamImpl<f2dEffectParam>*)m_ObjCache[pHandle];
}

////////////////////////////////////////////////////////////////////////////////

f2dEffectParamImpl<f2dEffectParam>* f2dEffectImpl::CreateEffectParam(f2dEffect* pParent, D3DXHANDLE pThisHandle)
{
	D3DXPARAMETER_DESC tDesc;
	if(FCYFAILED(((ID3DXEffect*)pParent->GetHandle())->GetParameterDesc(pThisHandle, &tDesc)))
		return NULL;

	if(tDesc.Elements > 1)
		return (f2dEffectParamImpl<f2dEffectParam>*)new f2dEffectParamArrImpl(pParent, pThisHandle, tDesc);

	switch(tDesc.Class)
	{
	case D3DXPC_SCALAR:
	case D3DXPC_VECTOR:
	case D3DXPC_MATRIX_ROWS:
	case D3DXPC_MATRIX_COLUMNS:
	case D3DXPC_OBJECT:
		return (f2dEffectParamImpl<f2dEffectParam>*)new f2dEffectParamValueImpl(pParent, pThisHandle, tDesc);
	case D3DXPC_STRUCT:
		return (f2dEffectParamImpl<f2dEffectParam>*)new f2dEffectParamStructImpl(pParent, pThisHandle, tDesc);
	}

	return NULL;
}

f2dEffectImpl::f2dEffectImpl(f2dRenderDeviceImpl* pDev, f2dStream* pStream, fBool bAutoState)
	: m_pParent(pDev), m_pEffect(NULL), m_pCurTechnique(NULL), m_bAutoState(bAutoState),
	m_pWorldVar(NULL), m_pProjVar(NULL), m_pViewVar(NULL)
{
	if(!pStream)
		throw fcyException("f2dEffectImpl::f2dEffectImpl", "Param 'pStream' is null.");

	// 读取所有代码
	fByte *tCodes = new fByte[ (size_t)pStream->GetLength() + 1 ];
	pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
	if(FCYFAILED(pStream->ReadBytes(tCodes, pStream->GetLength(), NULL)))
	{
		FCYSAFEDELARR(tCodes);
		throw fcyException("f2dEffectImpl::f2dEffectImpl", "f2dStream::ReadBytes failed.");
	}
	tCodes[ pStream->GetLength() ] = '\0';

	// 创建效果
	ID3DXEffect* pEffect = NULL;
	ID3DXBuffer* pErr = NULL;
	HRESULT tHR = pDev->GetAPI().DLLEntry_D3DXCreateEffectEx((IDirect3DDevice9*)pDev->GetHandle(), tCodes, (UINT)pStream->GetLength(), NULL, NULL, NULL, 0, NULL, &pEffect, &pErr);
	FCYSAFEDELARR(tCodes);

	// 失败处理
	if (FAILED(tHR) || !pEffect)
	{
		if(pErr)
		{
			string tErrBuffer;
		
			tErrBuffer.resize(pErr->GetBufferSize() + 1024);
			sprintf_s(&tErrBuffer[0], pErr->GetBufferSize() + 1024, 
				"D3DXCreateEffectEx failed. ( HRESULT = %d )\nDetail: %s",
				tHR,
				(const char*)pErr->GetBufferPointer());

			FCYSAFEKILL(pErr);

			MessageBoxA(NULL, tErrBuffer.data(), "Fancy2D Error", MB_OK | MB_ICONERROR);

			// 强制截断以满足fcyException大小
			if(tErrBuffer.size() > 453)
			{
				tErrBuffer[450] = '.';
				tErrBuffer[451] = '.';
				tErrBuffer[452] = '.';
				tErrBuffer[453] = '\0';
			}

			throw fcyException("f2dEffectImpl::f2dEffectImpl", tErrBuffer.c_str());
		}
		else
		{
			throw fcyWin32COMException("f2dEffectImpl::f2dEffectImpl", "D3DXCreateEffectEx failed.", tHR);
		}
	}

	FCYSAFEKILL(pErr);

	m_pEffect = pEffect;
	m_pEffect->GetDesc(&m_Desc);

	// === 注册监听 ===
	m_pParent->AttachListener(this, 16);

	// === 扫描所有变量并记录带语义变量 ===
	for(fuInt i = 0; i<GetParamCount(); ++i)
	{
		f2dEffectParam* pParam = GetParam(i);
		
		if(pParam->GetType() == F2DEPTYPE_VALUE)
		{
			f2dEffectParamValue* pValue = (f2dEffectParamValue*)pParam;

			if(pValue->GetSemantic())
			{
				string tSemantic = pValue->GetSemantic();
				if(pValue->GetValueType() == F2DEPVTYPE_MATRIX)
				{
					if(tSemantic == "WORLD")
						m_pWorldVar = pValue;
					else if(tSemantic == "VIEW")
						m_pViewVar = pValue;
					else if(tSemantic == "PROJECTION")
						m_pProjVar = pValue;
				}
			}
		}
	}
}

f2dEffectImpl::~f2dEffectImpl()
{
	FCYSAFEKILL(m_pEffect);

	// === 释放所有缓存的对象 ===
	std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*>::iterator i = m_ParamCache.begin();
	while(i != m_ParamCache.end())
	{
		delete(i->second);
		++i;
	}

	std::unordered_map<D3DXHANDLE, f2dEffectFunctionImpl*>::iterator j = m_FuncCache.begin();
	while(j != m_FuncCache.end())
	{
		delete(j->second);
		j++;
	}

	std::unordered_map<D3DXHANDLE, f2dEffectTechniqueImpl*>::iterator k = m_TechCache.begin();
	while(k != m_TechCache.end())
	{
		delete(k->second);
		k++;
	}

	// === 释放所有缓存的纹理 ===
	for(unordered_map<D3DXHANDLE, f2dTexture*>::iterator i = m_CachedTex.begin();
		i != m_CachedTex.end(); ++i)
	{
		FCYSAFEKILL(i->second);
	}

	// === 释放监听 ===
	m_pParent->RemoveListener(this);
}

void f2dEffectImpl::SyncState(f2dGraphics3D* pGraph)
{
	if(m_bAutoState)
	{
		if(m_pWorldVar)
			m_pWorldVar->SetMatrix(pGraph->GetWorldTransform());
		if(m_pViewVar)
			m_pViewVar->SetMatrix(pGraph->GetViewTransform());
		if(m_pProjVar)
			m_pProjVar->SetMatrix(pGraph->GetProjTransform());
	}
}
