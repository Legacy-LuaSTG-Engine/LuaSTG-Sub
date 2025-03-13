#include "Interface.hpp"

namespace core {
    template<> InterfaceId getInterfaceId<IInterface>() { return 1; }
}
