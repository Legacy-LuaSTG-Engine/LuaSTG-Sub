#include "WeakReferenceSource.hpp"

namespace core {
    template<> InterfaceId getInterfaceId<IWeakReferenceSource>() { return 5; }
}
