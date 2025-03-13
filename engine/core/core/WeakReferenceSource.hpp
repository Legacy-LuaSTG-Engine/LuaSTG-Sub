#pragma once
#include "WeakReference.hpp"

namespace core {
    struct CORE_NO_VIRTUAL_TABLE IWeakReferenceSource : IObject {
        virtual void getWeakReference(IWeakReference** output) = 0;
    };
}
