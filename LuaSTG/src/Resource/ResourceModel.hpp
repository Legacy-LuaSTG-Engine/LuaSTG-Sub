#pragma once
#include "ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTGPlus
{
	// 模型资源
	class ResModel : public Resource
	{
	private:
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IModel> model_;
	public:
		LuaSTG::Core::Graphics::IModel* GetModel() { return *model_; }
	public:
		ResModel(const char* name, const char* path);
		virtual ~ResModel();
	};
}
