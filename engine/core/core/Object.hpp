#pragma once
#include "ReferenceCounted.hpp"

namespace core {
    struct CORE_NO_VIRTUAL_TABLE IObject : IReferenceCounted {
        virtual bool queryInterface(InterfaceId interface_id, void** output) = 0;
        template<typename Interface> bool queryInterface(Interface** output) {
            return queryInterface(getInterfaceId<Interface>(), reinterpret_cast<void**>(output));
        }
    };
}
