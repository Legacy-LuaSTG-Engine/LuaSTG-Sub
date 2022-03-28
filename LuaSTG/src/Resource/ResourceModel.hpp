#pragma once
#include "ResourceBase.hpp"
#include "Core/Renderer.hpp"

namespace LuaSTGPlus
{
	// 模型资源
	class ResModel : public Resource
	{
	private:
		LuaSTG::Core::ScopeObject<LuaSTG::Core::IModel> model_;
	public:
		LuaSTG::Core::IModel* GetModel() { return *model_; }
	public:
		ResModel(const char* name, LuaSTG::Core::ScopeObject<LuaSTG::Core::IModel> model)
			: Resource(ResourceType::Model, name)
			, model_(model)
		{}
	};
}
