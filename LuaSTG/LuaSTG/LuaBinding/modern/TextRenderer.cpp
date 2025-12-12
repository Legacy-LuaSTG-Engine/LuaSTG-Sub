#include "TextRenderer.hpp"
#include "TextLayout.hpp"
#include "Vector2.hpp"
#include "LuaWrapper.hpp"
#include "GameResource/LegacyBlendStateHelper.hpp"
#include "lua/plus.hpp"

namespace luastg::binding {
    const std::string_view TextRenderer::class_name{ "lstg.TextRenderer" };

    struct TextRendererBinding : TextRenderer {
        // meta methods

        // NOLINTBEGIN(*-reserved-identifier)

        static int __gc(lua_State* const vm) {
            if (const auto self = as(vm, 1); self->data) {
                self->data->release();
                self->data = nullptr;
            }
            return 0;
        }
        static int __tostring(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            [[maybe_unused]] const auto self = as(vm, 1);
            ctx.push_value(class_name);
            return 1;
        }
        static int __eq(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            const auto self = as(vm, 1);
            if (is(vm, 2)) {
                const auto other = as(vm, 2);
                ctx.push_value(self->data == other->data);
            }
            else {
                ctx.push_value(false);
            }
            return 1;
        }

        // NOLINTEND(*-reserved-identifier)

        // method

        static int setTransform(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            self->position.x = ctx.get_value<float>(1 + 1);
            self->position.y = ctx.get_value<float>(1 + 2);
            self->rotation = ctx.get_value<float>(1 + 3, 0.0f);
            self->scale.x = ctx.get_value<float>(1 + 4, 1.0f);
            self->scale.y = ctx.get_value<float>(1 + 5, self->scale.x);
            self->data->setTransform(self->position, self->scale, self->rotation);  // TODO: TBD
            self->data->setZ(0.5); // TODO: allow custom
            self->is_dirty = false;
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setPosition(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            self->position.x = ctx.get_value<float>(1 + 1);
            self->position.y = ctx.get_value<float>(1 + 2);
            self->is_dirty = true;
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setScale(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            self->scale.x = ctx.get_value<float>(1 + 1);
            self->scale.y = ctx.get_value<float>(1 + 2, self->scale.x);
            self->is_dirty = true;
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setRotation(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            self->rotation = ctx.get_value<float>(1 + 1);
            self->is_dirty = true;
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setTextLayout(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const text_layput = TextLayout::as(vm, 1 + 1);
            self->data->setTextLayout(text_layput->data);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setAnchor(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (ctx.is_number(1 + 1)) {
                auto const width = ctx.get_value<float>(1 + 1);
                auto const height = ctx.get_value<float>(1 + 2);
                self->data->setAnchor(core::Vector2F(width, height));
            }
            else {
                auto const layout_size = Vector2::as(vm, 1 + 1);
                self->data->setAnchor(core::Vector2F(
                    static_cast<float>(layout_size->data.x),
                    static_cast<float>(layout_size->data.y)
                ));
            }
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setColor(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (ctx.index_of_top() >= 4) {
                auto const c1 = *Color::Cast(vm, 1 + 1);
                auto const c2 = *Color::Cast(vm, 1 + 2);
                auto const c3 = *Color::Cast(vm, 1 + 3);
                auto const c4 = *Color::Cast(vm, 1 + 4);
                self->data->setColor(c1, c2, c3, c4);
            }
            else {
                auto const color = *Color::Cast(vm, 1 + 1);
                self->data->setColor(color);
            }
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setLegacyBlendState(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const blend = luastg::TranslateBlendMode(vm, 1 + 1);
            [[maybe_unused]] auto const [v, b] = luastg::translateLegacyBlendState(blend);
            self->data->setLegacyBlendState(v, b);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int draw(lua_State* const vm) {
            auto const self = as(vm, 1);
            if (self->is_dirty) {
                self->data->setTransform(self->position, self->scale, self->rotation);  // TODO: TBD
                self->data->setZ(0.5); // TODO: allow custom
            }
            self->data->draw(LAPP.GetAppModel()->getRenderer());
            return 0;
        }

        // static method

        static int create(lua_State* const vm) {
            if (lua::stack_t const ctx(vm); ctx.index_of_top() >= 1) {
                auto const text_layout = TextLayout::as(vm, 1);
                auto const self = TextRenderer::create(vm);
                if (!core::Graphics::ITextRenderer2::create(LAPP.GetAppModel()->getDevice(), &self->data)) {
                    return luaL_error(vm, "create TextRenderer failed");
                }
                self->data->setTextLayout(text_layout->data);
            }
            else {
                auto const self = TextRenderer::create(vm);
                if (!core::Graphics::ITextRenderer2::create(LAPP.GetAppModel()->getDevice(), &self->data)) {
                    return luaL_error(vm, "create TextRenderer failed");
                }
            }
            return 1;
        }
    };

    bool TextRenderer::is(lua_State* const vm, const int index) {
        lua::stack_t const ctx(vm);
        return ctx.is_metatable(index, class_name);
    }
    TextRenderer* TextRenderer::as(lua_State* const vm, const int index) {
        lua::stack_t const ctx(vm);
        return ctx.as_userdata<TextRenderer>(index);
    }
    TextRenderer* TextRenderer::create(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const self = ctx.create_userdata<TextRenderer>();
        auto const self_index = ctx.index_of_top();
        ctx.set_metatable(self_index, class_name);
        self->data = nullptr;
        return self;
    }
    void TextRenderer::registerClass(lua_State* const vm) {
        [[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
        lua::stack_t const ctx(vm);

        // method

        auto const method_table = ctx.create_module(class_name);
        ctx.set_map_value(method_table, "setTransform", &TextRendererBinding::setTransform);
        ctx.set_map_value(method_table, "setPosition", &TextRendererBinding::setPosition);
        ctx.set_map_value(method_table, "setScale", &TextRendererBinding::setScale);
        ctx.set_map_value(method_table, "setRotation", &TextRendererBinding::setRotation);
        ctx.set_map_value(method_table, "setTextLayout", &TextRendererBinding::setTextLayout);
        ctx.set_map_value(method_table, "setAnchor", &TextRendererBinding::setAnchor);
        ctx.set_map_value(method_table, "setColor", &TextRendererBinding::setColor);
        ctx.set_map_value(method_table, "setLegacyBlendState", &TextRendererBinding::setLegacyBlendState);
        ctx.set_map_value(method_table, "draw", &TextRendererBinding::draw);
        ctx.set_map_value(method_table, "create", &TextRendererBinding::create);

        // metatable

        auto const metatable = ctx.create_metatable(class_name);
        ctx.set_map_value(metatable, "__gc", &TextRendererBinding::__gc);
        ctx.set_map_value(metatable, "__tostring", &TextRendererBinding::__tostring);
        ctx.set_map_value(metatable, "__eq", &TextRendererBinding::__eq);
        ctx.set_map_value(metatable, "__index", method_table);
    }
}
