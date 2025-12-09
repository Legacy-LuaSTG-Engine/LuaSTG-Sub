#pragma once
#include "lua.hpp"
#include "Core/Graphics/Font.hpp"

namespace luastg::binding {
    struct TextLayout {
        static std::string_view const class_name;

        [[maybe_unused]] core::ITextLayout* data{};

        static bool is(lua_State* vm, int index);
        static TextLayout* as(lua_State* vm, int index);
        static TextLayout* create(lua_State* vm);
        static void registerClass(lua_State* vm);
    };
}
