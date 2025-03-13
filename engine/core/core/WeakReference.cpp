#include "WeakReference.hpp"

namespace core {
    template<> InterfaceId getInterfaceId<IWeakReference>() { return 4; }
}
