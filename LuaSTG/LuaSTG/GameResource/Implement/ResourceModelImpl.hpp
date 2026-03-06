#pragma once
#include "core/SmartReference.hpp"
#include "GameResource/ResourceModel.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceModelImpl : public ResourceBaseImpl<IResourceModel>
	{
	private:
		core::SmartReference<core::Graphics::IModel> model_;
	public:
		core::Graphics::IModel* GetModel() { return *model_; }
	public:
		ResourceModelImpl(const char* name, const char* path);
	};
}
