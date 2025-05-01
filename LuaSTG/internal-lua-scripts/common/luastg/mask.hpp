#pragma once
#include <cstdint>

namespace luastg {
    inline void mask(void* const data, size_t const size) {
        auto const p = static_cast<uint8_t*>(data);
        for (size_t i = 0; i < size; ++i) {
            constexpr uint8_t MASK[4]{'l', 's', 't', 'g'};
            p[i] = p[i] ^ MASK[i % 4];
        }
    }
}
