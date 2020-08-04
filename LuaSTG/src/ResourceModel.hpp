#pragma once
#include "ResourceBase.hpp"

namespace LuaSTGPlus {
	// 模型资源
	class ResModel :
		public Resource
	{
	private:
		void* m_Model;
	public:
		void* GetModel() { return m_Model; }
	public:
		ResModel(const char* name, void* model)
			: Resource(ResourceType::Model, name), m_Model(model) {}
	};
}
