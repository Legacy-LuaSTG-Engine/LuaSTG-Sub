#pragma once
#include "Interface.hpp"

namespace core {
    struct CORE_NO_VIRTUAL_TABLE IReferenceCounted : IInterface {
        virtual void reference() = 0;
        virtual void release() = 0;
    };
}
