#pragma once
#include "lua.hpp"
#include "core/Graphics/Font.hpp"

namespace luastg::binding {
    struct TextRenderer {
        static const std::string_view class_name;

        core::Graphics::ITextRenderer2* data{};
        core::Vector2F position;
        core::Vector2F scale;
        float rotation{};
        bool is_dirty{};

        static bool is(lua_State* vm, int index);
        static TextRenderer* as(lua_State* vm, int index);
        static TextRenderer* create(lua_State* vm);
        static void registerClass(lua_State* vm);
    };
}
