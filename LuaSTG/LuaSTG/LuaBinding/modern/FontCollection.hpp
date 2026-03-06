#pragma once
#include "lua.hpp"
#include "core/FontCollection.hpp"

namespace luastg::binding {
    struct FontCollection {
        static const std::string_view class_name;

        core::IFontCollection* data{};

        static bool is(lua_State* vm, int index);
        static FontCollection* as(lua_State* vm, int index);
        static FontCollection* create(lua_State* vm);
        static void registerClass(lua_State* vm);
    };
}
