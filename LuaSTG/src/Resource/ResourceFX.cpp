#include "ResourceFX.hpp"
#include "ResourceModel.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	ResFX::ResFX(const char* name, const char* path)
		: Resource(ResourceType::FX, name)
	{
		if (!LAPP.GetAppModel()->getRenderer()->createPostEffectShader(path, ~m_shader))
			throw std::runtime_error("ResFX");
	}
	ResFX::~ResFX() {}

	ResModel::ResModel(const char* name, const char* path)
		: Resource(ResourceType::Model, name)
	{
		if (!LAPP.GetAppModel()->getRenderer()->createModel(path, ~model_))
			throw std::runtime_error("ResModel");
	}
	ResModel::~ResModel() {}
}
