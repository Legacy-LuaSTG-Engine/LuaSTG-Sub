#pragma once
#include "Object.hpp"

namespace core {
    struct CORE_NO_VIRTUAL_TABLE IWeakReference : IObject {
        virtual bool resolve(InterfaceId interface_id, void** output) = 0;
        template<typename Interface> bool resolve(Interface** output) {
            return resolve(getInterfaceId<Interface>(), reinterpret_cast<void**>(output));
        }
    };
}
