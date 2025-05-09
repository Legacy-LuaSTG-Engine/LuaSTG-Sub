#pragma once
#include "core/FileSystem.hpp"

namespace luastg {
    struct CORE_NO_VIRTUAL_TABLE IEmbeddedFileSystem : core::IFileSystem {
        static IEmbeddedFileSystem* getInstance();
    };
}

namespace core {
    // UUID v5
    // ns:URL
    // https://www.luastg-sub.com/luastg.IEmbeddedFileSystem
    template<> constexpr InterfaceId getInterfaceId<luastg::IEmbeddedFileSystem>() { return UUID::parse("d049a157-0c7d-55c6-87f1-84d8bc50d674"); }
}
