#pragma once
#include <cstdint>

#define CORE_NO_VIRTUAL_TABLE __declspec(novtable)

namespace core {
    using InterfaceId = int64_t;

    template<typename T>
    InterfaceId getInterfaceId();

    struct CORE_NO_VIRTUAL_TABLE IInterface {
    };
}
