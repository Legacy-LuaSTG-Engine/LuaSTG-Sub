#include "ReferenceCounted.hpp"

namespace core {
    template<> InterfaceId getInterfaceId<IReferenceCounted>() { return 2; }
}
