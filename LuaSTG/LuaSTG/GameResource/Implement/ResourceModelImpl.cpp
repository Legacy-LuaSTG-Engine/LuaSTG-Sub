#include "GameResource/Implement/ResourceModelImpl.hpp"
#include "AppFrame.h"

namespace luastg
{
    ResourceModelImpl::ResourceModelImpl(const char* name, const char* path)
        : ResourceBaseImpl(ResourceType::Model, name)
    {
        if (!LAPP.GetAppModel()->getRenderer()->createModel(path, model_.put()))
            throw std::runtime_error("ResourceModelImpl::ResourceModelImpl");
    }
}
