#include "ResourceFX.hpp"
#include "AppFrame.h"
#include "Utility.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace std;

namespace LuaSTGPlus {
	ResFX::ResFX(const char* name, fcyRefPointer<f2dEffect> shader)
		: Resource(ResourceType::FX, name), m_pShader(shader)
	{
		// 扫描所有的变量注释
		for (fuInt i = 0; i < m_pShader->GetParamCount(); ++i)
		{
			f2dEffectParam* pParam = m_pShader->GetParam(i);

			if (pParam->GetType() == F2DEPTYPE_VALUE)
			{
				f2dEffectParamValue* pValue = (f2dEffectParamValue*)pParam;

				// 注释绑定
				f2dEffectParam* pAnnotation = pValue->GetAnnotation("binding");
				if (pAnnotation && pAnnotation->GetType() == F2DEPTYPE_VALUE)
				{
					f2dEffectParamValue* pAnnotationVal = (f2dEffectParamValue*)pAnnotation;
					if (pAnnotationVal->GetValueType() == F2DEPVTYPE_STRING && pAnnotationVal->GetString())
					{
						fcStr tBindingVar = pAnnotationVal->GetString();  // 被绑定的脚本变量
						m_pBindingVar[tBindingVar].push_back(pValue);
					}
				}

				// 语义绑定
				fcStr pSemantic = pValue->GetSemantic();
				if (pSemantic)
				{
					if (_stricmp(pSemantic, "POSTEFFECTTEXTURE") == 0)
					{
						if (pValue->GetValueType() != F2DEPVTYPE_TEXTURE2D)
							throw fcyException("ResFX::ResFX", "Invalid binding for 'POSTEFFECTTEXTURE'.");
						m_pBindingPostEffectTexture.push_back(pValue);
					}
					else if (_stricmp(pSemantic, "VIEWPORT") == 0)
					{
						if (pValue->GetValueType() != F2DEPVTYPE_VECTOR)
							throw fcyException("ResFX::ResFX", "Invalid binding for 'VIEWPORT'.");
						m_pBindingViewport.push_back(pValue);
					}
					else if (_stricmp(pSemantic, "SCREENSIZE") == 0)
					{
						if (pValue->GetValueType() != F2DEPVTYPE_VECTOR)
							throw fcyException("ResFX::ResFX", "Invalid binding for 'SCREENSIZE'.");
						m_pBindingScreenSize.push_back(pValue);
					}
				}
			}
		}
	}

	void ResFX::SetPostEffectTexture(f2dTexture2D* val)LNOEXCEPT
	{
		for (auto& p : m_pBindingPostEffectTexture)
			p->SetTexture(val);
	}

	void ResFX::SetViewport(fcyRect rect)LNOEXCEPT
	{
		fcyVec4 vec(rect.a.x, rect.a.y, rect.b.x, rect.b.y);
		for (auto& p : m_pBindingViewport)
			p->SetVector(vec);
	}

	void ResFX::SetScreenSize(fcyVec2 size)LNOEXCEPT
	{
		fcyVec4 vec(0.f, 0.f, size.x, size.y);
		for (auto& p : m_pBindingScreenSize)
			p->SetVector(vec);
	}

	void ResFX::SetValue(const char* key, float val)LNOEXCEPT
	{
		auto i = m_pBindingVar.find(key);
		if (i != m_pBindingVar.end())
		{
			for (auto& p : i->second)
			{
				if (p->GetValueType() == F2DEPVTYPE_FLOAT)
					p->SetFloat(val);
			}
		}
	}

	void ResFX::SetValue(const char* key, fcyColor val)LNOEXCEPT
	{
		auto i = m_pBindingVar.find(key);
		if (i != m_pBindingVar.end())
		{
			fcyVec4 tColorVec(val.r / 255.f, val.g / 255.f, val.b / 255.f, val.a / 255.f);

			for (auto& p : i->second)
			{
				if (p->GetValueType() == F2DEPVTYPE_VECTOR)
					p->SetVector(tColorVec);
			}
		}
	}

	void ResFX::SetValue(const char* key, f2dTexture2D* val)LNOEXCEPT
	{
		auto i = m_pBindingVar.find(key);
		if (i != m_pBindingVar.end())
		{
			for (auto& p : i->second)
			{
				if (p->GetValueType() == F2DEPVTYPE_TEXTURE2D)
					p->SetTexture(val);
			}
		}
	}
}
