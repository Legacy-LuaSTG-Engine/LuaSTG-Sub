////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelLabel.h
/// @brief fcy模型文件标签定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyRefObj.h"
#include "../fcyMath.h"
#include "../fcyIO/fcyStream.h"
#include "../fcyIO/fcyBinaryHelper.h"
#include "../fcyMisc/fcyStringHelper.h"

#include <unordered_map>
#include <string>

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

/// @brief 标签名称
#define FMM_LABELNAME fuLong

/// @brief 创建标签
#define FMM_MAKE_LABELNAME8(Str) (*((fuLong*)Str))

/// @brief 创建标签_4字节
#define FMM_MAKE_LABELNAME4(Str) (*((fuInt*)Str))

/// @brief 模型数据标签
class fcyModelLabel;

////////////////////////////////////////////////////////////////////////////////
/// @brief 模型标签工厂
////////////////////////////////////////////////////////////////////////////////
class fcyModelLabelFactory
{
public:
	/// @brief 自动注册
	template<typename T, typename U = fcyModelLabel>  // FIXME!
	class AutoRegister
	{
	public:
		AutoRegister()
		{
			U::GetFactory().RegisterLabel<T>(FMM_MAKE_LABELNAME8(T::LABELNAME));
		}
	};
public:
	/// @brief 标签创建函数
	struct CreatorFunction :
		public fcyRefObj
	{
		virtual fcyRefPointer<fcyModelLabel> CreateInstance()=0;
	};
protected:
	/// @brief 标签创建函数模板实现
	template<typename T>
	class CreatorFunctionImpl :
  		public fcyRefObjImpl<CreatorFunction>
	{
	public:
		fcyRefPointer<fcyModelLabel> CreateInstance()
		{
			fcyRefPointer<fcyModelLabel> tRet; 
			(*&tRet) = new T();
			return tRet;
		}
	};
protected:
	std::unordered_map< FMM_LABELNAME, fcyRefPointer<CreatorFunction> > m_ConstructFunc;
public:
	/// @brief 获得标签对象
	CreatorFunction* GetCreator(FMM_LABELNAME Name)
	{
		return m_ConstructFunc[Name];
	}
	/// @brief 注册标签对象
	template<typename T>
	void RegisterLabel(FMM_LABELNAME Name)
	{
		if(m_ConstructFunc[Name])
			FCYSAFEKILL(m_ConstructFunc[Name]);
		(*&m_ConstructFunc[Name]) = new CreatorFunctionImpl<T>();
	}
	/// @brief 取消标签对象注册
	void UnregisterLabel(FMM_LABELNAME Name)
	{
		if(m_ConstructFunc[Name])
			m_ConstructFunc[Name] = NULL;
	}
public:
	fcyModelLabelFactory();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 模型数据标签
////////////////////////////////////////////////////////////////////////////////
class fcyModelLabel :
	public fcyRefObjImpl<fcyRefObj>
{
public:
	/// @brief 返回工厂对象
	static fcyModelLabelFactory& GetFactory()
	{
		static fcyModelLabelFactory s_Factory;
		return s_Factory;
	}

	/// @brief 字符串数据读取辅助函数
	static std::wstring ReadString(fcyStream* pStream)
	{
		fcyBinaryReader tReader(pStream);

		fuInt tStrSize = tReader.ReadUInt32();
		std::string pBuffer;
		pBuffer.resize(tStrSize);
		if(tStrSize)
			tReader.ReadChars(&pBuffer[0], tStrSize);
		
		return fcyStringHelper::MultiByteToWideChar(pBuffer, CP_UTF8);
	}
	/// @brief 字符串数据写入辅助函数
	static void WriteString(fcyStream* pStream, const std::wstring& Str)
	{
		fcyBinaryWriter tWritter(pStream);

		std::string tStr = fcyStringHelper::WideCharToMultiByte(Str, CP_UTF8);
		tWritter.Write((fuInt)tStr.size());
		tWritter.Write(&tStr[0], tStr.size());
	}
protected:
	FMM_LABELNAME m_LabelName;
public:
	/// @brief 获得标签名
	FMM_LABELNAME GetLabelName() { return m_LabelName; }

	/// @brief 从流读取数据
	/// @note  不含标签头
	virtual void ReadData(fcyStream* pStream)=0;
	/// @brief 写入数据
	/// @note  不含标签头
	virtual void WriteData(fcyStream* pStream)=0;
protected:
	fcyModelLabel(const fcyModelLabel& Org);
public:
	fcyModelLabel(FMM_LABELNAME LabelName)
		: m_LabelName(LabelName) {}
protected:
	virtual ~fcyModelLabel() {}
};
/// @}
