#pragma once
#include "core/ImmutableString.hpp"

namespace core {
    CORE_INTERFACE IImmutableStringList : public IReferenceCounted {
        [[nodiscard]] virtual uint32_t getCount() const noexcept = 0;
        [[nodiscard]] virtual StringView getView(uint32_t index) const noexcept = 0;
        [[nodiscard]] virtual bool getString(uint32_t index, IImmutableString** output) const noexcept = 0;
    };
    CORE_INTERFACE_ID(IImmutableStringList, "0bc61e36-f1be-5453-9e3c-04237e3fce51");
}
