#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"

namespace core {
    CORE_INTERFACE IImage : IReferenceCounted {
        virtual Vector2U getSize() const noexcept = 0;
    };
    CORE_INTERFACE_ID(IImage, "5e4c12e0-e094-5346-b129-b9ddbb881373")
}
