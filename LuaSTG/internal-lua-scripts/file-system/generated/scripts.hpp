#pragma once
#include <string_view>
#include <span>

namespace luastg {
    struct InternalLuaScriptsFileSystemNode {
        std::string_view name;
        std::span<uint8_t const> data;
    };

    namespace generated {
        extern std::span<InternalLuaScriptsFileSystemNode const> const files;
    }
}
