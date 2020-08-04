////////////////////////////////////////////////////////////////////////////////
/// @file  f2dEffectImpl.h
/// @brief fancy2D D3DXEffect包装
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "RenderDev/f2dRenderDeviceAPI.h"

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数对象模板
////////////////////////////////////////////////////////////////////////////////
template<class T>
class f2dEffectParamImpl :
	public T
{
protected:
	f2dEffect* m_pParent;

	D3DXHANDLE m_pHandle;
	D3DXPARAMETER_DESC m_Desc;

	// 子对象缓存
	std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*> m_ObjCache;
protected:
	f2dEffectParamImpl<f2dEffectParam>* GetCacheObj(D3DXHANDLE pHandle)
	{
		if(pHandle == NULL)
			return NULL;
		if(m_ObjCache.find(pHandle) == m_ObjCache.end())
		{
			m_ObjCache[pHandle] = f2dEffectImpl::CreateEffectParam(m_pParent, pHandle);
		}

		return (f2dEffectParamImpl<f2dEffectParam>*)m_ObjCache[pHandle];
	}
public: // 接口实现
	fcStr GetName()
	{
		return m_Desc.Name;
	}
	F2DEPTYPE GetType()
	{
		if(m_Desc.Elements > 1)
			return F2DEPTYPE_ARRAY;

		switch(m_Desc.Class)
		{
		case D3DXPC_SCALAR:
		case D3DXPC_VECTOR:
		case D3DXPC_MATRIX_ROWS:
		case D3DXPC_MATRIX_COLUMNS:
		case D3DXPC_OBJECT:
			return F2DEPTYPE_VALUE;
		case D3DXPC_STRUCT:
			return F2DEPTYPE_STRUCT;
		}

		return F2DEPTYPE_UNKNOWN;
	}
	fcStr GetSemantic()
	{
		return m_Desc.Semantic;
	}
	f2dEffectParam* GetAnnotation(fcStr Name)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotationByName(m_pHandle, Name);

		return GetCacheObj(tHandle);
	}
	f2dEffectParam* GetAnnotation(fuInt Index)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotation(m_pHandle, Index);

		return GetCacheObj(tHandle);
	}
	fuInt GetAnnotationCount()
	{
		return m_Desc.Annotations;
	}
protected:
	f2dEffectParamImpl(f2dEffect* pParent, D3DXHANDLE pThisHandle, const D3DXPARAMETER_DESC& Desc)
		: m_pParent(pParent), m_pHandle(pThisHandle), m_Desc(Desc) { }
public:
	~f2dEffectParamImpl()
	{
		std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*>::iterator i = m_ObjCache.begin();
		while(i != m_ObjCache.end())
		{
			delete(i->second);
			++i;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数值对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectParamValueImpl :
	public f2dEffectParamImpl<f2dEffectParamValue>
{
	friend class f2dEffectImpl;
public: // 接口实现
	F2DEPVTYPE GetValueType()
	{
		switch(m_Desc.Class)
		{
		case D3DXPC_VECTOR:
			return F2DEPVTYPE_VECTOR;
		case D3DXPC_MATRIX_COLUMNS:
		case D3DXPC_MATRIX_ROWS:
			return F2DEPVTYPE_MATRIX;
		}

		switch (m_Desc.Type)
		{
		case D3DXPT_VOID:
			return F2DEPVTYPE_VOID;
		case D3DXPT_BOOL:
			return F2DEPVTYPE_BOOL;
		case D3DXPT_INT:
			return F2DEPVTYPE_INT;
		case D3DXPT_FLOAT:
			return F2DEPVTYPE_FLOAT;
		case D3DXPT_TEXTURE1D:
			return F2DEPVTYPE_TEXTURE1D;
		case D3DXPT_TEXTURE2D:
			return F2DEPVTYPE_TEXTURE2D;
		case D3DXPT_TEXTURE3D:
			return F2DEPVTYPE_TEXTURE3D;
		case D3DXPT_STRING:
			return F2DEPVTYPE_STRING;
		}

		return F2DEPVTYPE_UNKNOWN;
	}

	fuInt GetRow()
	{
		return m_Desc.Rows;
	}
	fuInt GetColumn()
	{
		return m_Desc.Columns;
	}

	fBool IsRowFirstMatrix()
	{
		return (m_Desc.Class == D3DXPC_MATRIX_ROWS);
	}

	fBool GetBool()
	{
		BOOL tRet;
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->GetBool(m_pHandle, &tRet)))
			return false;

		return tRet == TRUE ? true : false;
	}
	fFloat GetFloat()
	{
		float tRet;
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->GetFloat(m_pHandle, &tRet)))
			return 0.f;

		return tRet;
	}
	fInt GetInt()
	{
		int tRet;
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->GetInt(m_pHandle, &tRet)))
			return 0;

		return tRet;
	}
	fcyVec4 GetVector()
	{
		fcyVec4 tRet;
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->GetVector(m_pHandle, (D3DXVECTOR4*)&tRet)))
			return fcyVec4();

		return tRet;
	}
	fcyMatrix4 GetMatrix()
	{
		fcyMatrix4 tRet;
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->GetMatrix(m_pHandle, (D3DXMATRIX*)&tRet)))
			return fcyMatrix4();

		return tRet;
	}
	f2dTexture* GetTexture();
	fcStr GetString()
	{
		fcStr tRet;
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->GetString(m_pHandle, &tRet)))
			return NULL;

		return tRet;
	}

	fResult SetBool(fBool Value)
	{
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetBool(m_pHandle, Value)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult SetFloat(fFloat Value)
	{
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetFloat(m_pHandle, Value)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult SetInt(fInt Value)
	{
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetInt(m_pHandle, Value)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult SetVector(const fcyVec4& Value)
	{
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetVector(m_pHandle, (D3DXVECTOR4*)&Value)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult SetMatrix(const fcyMatrix4& Value)
	{
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetMatrix(m_pHandle, (D3DXMATRIX*)&Value)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
	fResult SetTexture(f2dTexture* pTex);
	fResult SetString(fcStr Str)
	{
		if(FAILED(((ID3DXEffect*)m_pParent->GetHandle())->SetString(m_pHandle, Str)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
protected:
	f2dEffectParamValueImpl(f2dEffect* pParent, D3DXHANDLE pThisHandle, const D3DXPARAMETER_DESC& Desc)
		: f2dEffectParamImpl(pParent, pThisHandle, Desc) { }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数数组对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectParamArrImpl :
	public f2dEffectParamImpl<f2dEffectParamArr>
{
	friend class f2dEffectImpl;
public: // 接口实现
	fuInt GetElementCount()
	{
		return m_Desc.Elements;
	}

	f2dEffectParam* GetElement(fuInt Index)
	{
		return GetCacheObj(((ID3DXEffect*)m_pParent->GetHandle())->GetParameterElement(m_pHandle, Index));
	}
protected:
	f2dEffectParamArrImpl(f2dEffect* pParent, D3DXHANDLE pThisHandle, const D3DXPARAMETER_DESC& Desc)
		: f2dEffectParamImpl(pParent, pThisHandle, Desc) { }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数结构体对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectParamStructImpl :
	public f2dEffectParamImpl<f2dEffectParamStruct>
{
	friend class f2dEffectImpl;
public: // 接口实现
	fuInt GetElementCount()
	{
		return m_Desc.StructMembers;
	}
	f2dEffectParam* GetElement(fcStr Name)
	{
		return GetCacheObj(((ID3DXEffect*)m_pParent->GetHandle())->GetParameterByName(m_pHandle, Name));
	}
	f2dEffectParam* GetElement(fuInt Index)
	{
		return GetCacheObj(((ID3DXEffect*)m_pParent->GetHandle())->GetParameterElement(m_pHandle, Index));
	}
protected:
	f2dEffectParamStructImpl(f2dEffect* pParent, D3DXHANDLE pThisHandle, const D3DXPARAMETER_DESC& Desc)
		: f2dEffectParamImpl(pParent, pThisHandle, Desc) { }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数函数对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectFunctionImpl :
	public f2dEffectFunction
{
	friend class f2dEffectImpl;
protected:
	f2dEffect* m_pParent;

	D3DXHANDLE m_pHandle;
	D3DXFUNCTION_DESC m_FuncDesc;

	// 子对象缓存
	std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*> m_ObjCache;
protected:
	f2dEffectParamImpl<f2dEffectParam>* GetCacheObj(D3DXHANDLE pHandle);
public: // 接口实现
	fcStr GetName()
	{
		return m_FuncDesc.Name;
	}
	f2dEffectParam* GetAnnotation(fcStr Name)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotationByName(m_pHandle, Name);

		return GetCacheObj(tHandle);
	}
	f2dEffectParam* GetAnnotation(fuInt Index)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotation(m_pHandle, Index);

		return GetCacheObj(tHandle);
	}
	fuInt GetAnnotationCount()
	{
		return m_FuncDesc.Annotations;
	}
protected:
	f2dEffectFunctionImpl(f2dEffect* pParent, D3DXHANDLE pThisHandle)
		: m_pParent(pParent), m_pHandle(pThisHandle)
	{
		((ID3DXEffect*)m_pParent->GetHandle())->GetFunctionDesc(pThisHandle, &m_FuncDesc);
	}
	~f2dEffectFunctionImpl()
	{
		std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*>::iterator i = m_ObjCache.begin();
		while(i != m_ObjCache.end())
		{
			delete(i->second);
			++i;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数Pass对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectPassImpl :
	public f2dEffectPass
{
	friend class f2dEffectTechniqueImpl;
protected:
	f2dEffect* m_pParent;

	fuInt m_Index;
	D3DXHANDLE m_pHandle;
	D3DXPASS_DESC m_PassDesc;

	// 子对象缓存
	std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*> m_ObjCache;
protected:
	f2dEffectParamImpl<f2dEffectParam>* GetCacheObj(D3DXHANDLE pHandle);
public: // 接口实现
	fuInt GetIndex()
	{
		return m_Index;
	}

	fcStr GetName()
	{
		return m_PassDesc.Name;
	}
	f2dEffectParam* GetAnnotation(fcStr Name)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotationByName(m_pHandle, Name);

		return GetCacheObj(tHandle);
	}
	f2dEffectParam* GetAnnotation(fuInt Index)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotation(m_pHandle, Index);

		return GetCacheObj(tHandle);
	}
	fuInt GetAnnotationCount()
	{
		return m_PassDesc.Annotations;
	}
protected:
	f2dEffectPassImpl(f2dEffect* pParent, D3DXHANDLE pTechnique, D3DXHANDLE pThisHandle)
		: m_pParent(pParent), m_pHandle(pThisHandle), m_Index(0)
	{
		D3DXTECHNIQUE_DESC tTechDesc;
		((ID3DXEffect*)m_pParent->GetHandle())->GetTechniqueDesc(pTechnique, &tTechDesc);
		for(fuInt i = 0; i<tTechDesc.Passes; ++i)
		{
			if(pThisHandle == ((ID3DXEffect*)m_pParent->GetHandle())->GetPass(pTechnique, i))
			{
				m_Index = i;
				break;
			}
		}

		((ID3DXEffect*)m_pParent->GetHandle())->GetPassDesc(pThisHandle, &m_PassDesc);
	}
	~f2dEffectPassImpl()
	{
		std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*>::iterator i = m_ObjCache.begin();
		while(i != m_ObjCache.end())
		{
			delete(i->second);
			++i;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect参数Technique对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectTechniqueImpl :
	public f2dEffectTechnique
{
	friend class f2dEffectImpl;
protected:
	f2dEffect* m_pParent;

	D3DXHANDLE m_pHandle;
	D3DXTECHNIQUE_DESC m_TechDesc;

	// 子对象缓存
	std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*> m_ObjCache;

	// Pass缓存
	std::unordered_map<D3DXHANDLE, f2dEffectPassImpl*> m_PassCache;
protected:
	f2dEffectParamImpl<f2dEffectParam>* GetCacheObj(D3DXHANDLE pHandle);
	f2dEffectPassImpl* GetCachePass(D3DXHANDLE pHandle)
	{
		if(pHandle == NULL)
			return NULL;
		if(m_PassCache.find(pHandle) == m_PassCache.end())
		{
			m_PassCache[pHandle] = new f2dEffectPassImpl(m_pParent, m_pHandle, pHandle);
		}

		return m_PassCache[pHandle];
	}
public: // 接口实现
	fcStr GetName()
	{
		return m_TechDesc.Name;
	}
	f2dEffectParam* GetAnnotation(fcStr Name)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotationByName(m_pHandle, Name);

		return GetCacheObj(tHandle);
	}
	f2dEffectParam* GetAnnotation(fuInt Index)
	{
		D3DXHANDLE tHandle = ((ID3DXEffect*)m_pParent->GetHandle())->GetAnnotation(m_pHandle, Index);

		return GetCacheObj(tHandle);
	}
	fuInt GetAnnotationCount()
	{
		return m_TechDesc.Annotations;
	}

	fBool IsValidate()
	{
		return SUCCEEDED(((ID3DXEffect*)m_pParent->GetHandle())->ValidateTechnique(m_pHandle));
	}
	fuInt GetPassCount()
	{
		return m_TechDesc.Passes;
	}
	f2dEffectPass* GetPass(fuInt Index)
	{
		return GetCachePass(((ID3DXEffect*)m_pParent->GetHandle())->GetPass(m_pHandle, Index));
	}
	f2dEffectPass* GetPass(fcStr Name)
	{
		return GetCachePass(((ID3DXEffect*)m_pParent->GetHandle())->GetPassByName(m_pHandle, Name));
	}
protected:
	f2dEffectTechniqueImpl(f2dEffect* pParent, D3DXHANDLE pThisHandle)
		: m_pParent(pParent), m_pHandle(pThisHandle)
	{
		((ID3DXEffect*)m_pParent->GetHandle())->GetTechniqueDesc(pThisHandle, &m_TechDesc);
	}
	~f2dEffectTechniqueImpl()
	{
		std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*>::iterator i = m_ObjCache.begin();
		while(i != m_ObjCache.end())
		{
			delete(i->second);
			++i;
		}

		std::unordered_map<D3DXHANDLE, f2dEffectPassImpl*>::iterator j = m_PassCache.begin();
		while(j != m_PassCache.end())
		{
			delete(j->second);
			j++;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Effect效果对象实现
////////////////////////////////////////////////////////////////////////////////
class f2dEffectImpl :
	public fcyRefObjImpl<f2dEffect>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDeviceImpl;
public:
	static f2dEffectParamImpl<f2dEffectParam>* CreateEffectParam(f2dEffect* pParent, D3DXHANDLE pThisHandle);
private:
	f2dRenderDeviceImpl* m_pParent;

	ID3DXEffect* m_pEffect;
	D3DXEFFECT_DESC m_Desc;

	f2dEffectTechnique* m_pCurTechnique;

	// 子对象缓存
	std::unordered_map<D3DXHANDLE, f2dEffectParamImpl<f2dEffectParam>*> m_ParamCache;
	std::unordered_map<D3DXHANDLE, f2dEffectFunctionImpl*> m_FuncCache;
	std::unordered_map<D3DXHANDLE, f2dEffectTechniqueImpl*> m_TechCache;

	// 缓存纹理表
	std::unordered_map<D3DXHANDLE, f2dTexture*> m_CachedTex;

	// 自动状态
	fBool m_bAutoState;

	// 保存的自动状态
	f2dEffectParamValue* m_pWorldVar;
	f2dEffectParamValue* m_pViewVar;
	f2dEffectParamValue* m_pProjVar;

protected:
	// 返回缓存的参数对象
	f2dEffectParamImpl<f2dEffectParam>* GetCacheParam(D3DXHANDLE pHandle)
	{
		if(pHandle == NULL)
			return NULL;
		if(m_ParamCache.find(pHandle) == m_ParamCache.end())
		{
			m_ParamCache[pHandle] = f2dEffectImpl::CreateEffectParam(this, pHandle);
		}

		return m_ParamCache[pHandle];
	}
	// 返回缓存的函数对象
	f2dEffectFunctionImpl* GetCacheFunction(D3DXHANDLE pHandle)
	{
		if(pHandle == NULL)
			return NULL;
		if(m_FuncCache.find(pHandle) == m_FuncCache.end())
		{
			m_FuncCache[pHandle] = new f2dEffectFunctionImpl(this, pHandle);
		}

		return m_FuncCache[pHandle];
	}
	// 返回缓存的技术对象
	f2dEffectTechniqueImpl* GetCacheTechnique(D3DXHANDLE pHandle)
	{
		if(pHandle == NULL)
			return NULL;
		if(m_TechCache.find(pHandle) == m_TechCache.end())
		{
			m_TechCache[pHandle] = new f2dEffectTechniqueImpl(this, pHandle);
		}

		return m_TechCache[pHandle];
	}
protected: // 设备丢失
	void OnRenderDeviceLost()
	{
		m_pEffect->OnLostDevice();
	}
	void OnRenderDeviceReset()
	{
		m_pEffect->OnResetDevice();

		// 刷新所有缓存的纹理
		for(std::unordered_map<D3DXHANDLE, f2dTexture*>::iterator i = m_CachedTex.begin();
			i != m_CachedTex.end(); ++i)
		{
			if (i->second->GetHandle())
				m_pEffect->SetTexture(i->first, (IDirect3DBaseTexture9*)i->second->GetHandle());
			else
				m_pEffect->SetTexture(i->first, NULL);
		}

		if(m_pCurTechnique)
			m_pEffect->SetTechnique(((f2dEffectTechniqueImpl*)m_pCurTechnique)->m_pHandle);
	}
public: // 内部公开
	// 返回缓存的纹理对象
	f2dTexture* GetCachedTex(D3DXHANDLE pHandle)
	{
		return m_CachedTex[pHandle];
	}
	// 设置缓存的纹理对象
	void SetCachedTex(D3DXHANDLE pHandle, f2dTexture* pTex)
	{
		FCYSAFEKILL(m_CachedTex[pHandle]);
		if (!pTex)
		{
			auto i = m_CachedTex.find(pHandle);
			if (i != m_CachedTex.end())
				m_CachedTex.erase(i);
		}	
		else
			m_CachedTex[pHandle] = pTex;
		if(pTex)
			pTex->AddRef();
	}

	// for Graphics3D
	void SyncState(f2dGraphics3D* pGraph);
public: // 接口实现
	void* GetHandle()
	{
		return m_pEffect;
	}

	fuInt GetParamCount()
	{
		return m_Desc.Parameters;
	}
	f2dEffectParam* GetParam(fuInt Index)
	{
		return GetCacheParam(m_pEffect->GetParameter(NULL, Index));
	}
	f2dEffectParam* GetParam(fcStr Name)
	{
		return GetCacheParam(m_pEffect->GetParameterByName(NULL, Name));
	}

	fuInt GetTechniqueCount()
	{
		return m_Desc.Techniques;
	}
	f2dEffectTechnique* GetTechnique(fuInt Index)
	{
		return GetCacheTechnique(m_pEffect->GetTechnique(Index));
	}
	f2dEffectTechnique* GetTechnique(fcStr Name)
	{
		return GetCacheTechnique(m_pEffect->GetTechniqueByName(Name));
	}

	fuInt GetFunctionCount()
	{
		return m_Desc.Functions;
	}
	f2dEffectFunction* GetFunction(fuInt Index)
	{
		return GetCacheFunction(m_pEffect->GetFunction(Index));
	}
	f2dEffectFunction* GetFunction(fcStr Name)
	{
		return GetCacheFunction(m_pEffect->GetFunctionByName(Name));
	}
	
	f2dEffectTechnique* GetCurrentTechnique()
	{
		return m_pCurTechnique;
	}
	fResult SetCurrentTechnique(f2dEffectTechnique* pTech)
	{
		if(m_pCurTechnique == pTech)
			return FCYERR_OK;

		if(FAILED(m_pEffect->SetTechnique(((f2dEffectTechniqueImpl*)pTech)->m_pHandle)))
		{
			return FCYERR_INTERNALERR;
		}
		else
		{
			m_pCurTechnique = pTech;
			return FCYERR_OK;
		}
	}

	fuInt GetCurrentPassCount()
	{
		if(!m_pCurTechnique)
			return 0;
		else
			return m_pCurTechnique->GetPassCount();
	}
protected:
	f2dEffectImpl(f2dRenderDeviceImpl* pDev, f2dStream* pStream, fBool bAutoState);
	~f2dEffectImpl();
};
