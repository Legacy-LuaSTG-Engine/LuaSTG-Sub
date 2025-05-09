#pragma once
#include "core/FileSystem.hpp"

namespace luastg {
    struct CORE_NO_VIRTUAL_TABLE IInternalLuaScriptsFileSystem : core::IFileSystem {
        static IInternalLuaScriptsFileSystem* getInstance();
    };
}

namespace core {
    // UUID v5
    // ns:URL
    // https://www.luastg-sub.com/luastg.IInternalLuaScriptsFileSystem
    template<> constexpr InterfaceId getInterfaceId<luastg::IInternalLuaScriptsFileSystem>() { return UUID::parse("44aa3c18-08a6-5b78-bd7a-769e66f91f94"); }
}
