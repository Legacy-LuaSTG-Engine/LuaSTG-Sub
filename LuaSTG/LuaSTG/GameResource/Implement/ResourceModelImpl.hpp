#pragma once
#include "GameResource/ResourceModel.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceModelImpl : public ResourceBaseImpl<IResourceModel>
	{
	private:
		Core::ScopeObject<Core::Graphics::IModel> model_;
	public:
		Core::Graphics::IModel* GetModel() { return *model_; }
	public:
		ResourceModelImpl(const char* name, const char* path);
	};
}
