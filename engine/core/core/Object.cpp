#include "Object.hpp"

namespace core {
    template<> InterfaceId getInterfaceId<IObject>() { return 3; }
}