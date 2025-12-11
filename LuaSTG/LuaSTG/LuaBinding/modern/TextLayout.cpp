#include "TextLayout.hpp"
#include "Texture2D.hpp"
#include "Vector2.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

namespace luastg::binding {
    std::string_view const TextLayout::class_name{ "lstg.TextLayout" };

    struct TextLayoutBinding : TextLayout {
        // meta methods

        // NOLINTBEGIN(*-reserved-identifier)

        static int __gc(lua_State* const vm) {
            if (auto const self = as(vm, 1); self->data) {
                self->data->release();
                self->data = nullptr;
            }
            return 0;
        }
        static int __tostring(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            [[maybe_unused]] auto const self = as(vm, 1);
            ctx.push_value(class_name);
            return 1;
        }
        static int __eq(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (is(vm, 2)) {
                auto const other = as(vm, 2);
                ctx.push_value(self->data == other->data);
            }
            else {
                ctx.push_value(false);
            }
            return 1;
        }

        // NOLINTEND(*-reserved-identifier)

        // method

        static int setText(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const text = ctx.get_value<std::string_view>(1 + 1);
            self->data->setText(text);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setFontFamilyName(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const font_family_name = ctx.get_value<std::string_view>(1 + 1);
            self->data->setFontFamilyName(font_family_name);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setFontSize(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const font_size = ctx.get_value<float>(1 + 1);
            self->data->setFontSize(font_size);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setFontWeight(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const font_weight = ctx.get_value<core::FontWeight>(1 + 1);
            self->data->setFontWeight(font_weight);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setFontStyle(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const font_style = ctx.get_value<core::FontStyle>(1 + 1);
            self->data->setFontStyle(font_style);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setLayoutSize(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (ctx.index_of_top() >= (1 + 2)) {
                auto const width = ctx.get_value<float>(1 + 1);
                auto const height = ctx.get_value<float>(1 + 2);
                self->data->setLayoutSize(core::Vector2F(width, height));
            }
            else {
                auto const layout_size = Vector2::as(vm, 1 + 1);
                self->data->setLayoutSize(core::Vector2F(
                    static_cast<float>(layout_size->data.x),
                    static_cast<float>(layout_size->data.y)
                ));
            }
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setTextAlignment(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const text_alignment = ctx.get_value<core::TextAlignment>(1 + 1);
            self->data->setTextAlignment(text_alignment);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setParagraphAlignment(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const paragraph_alignment = ctx.get_value<core::ParagraphAlignment>(1 + 1);
            self->data->setParagraphAlignment(paragraph_alignment);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }

        static int build(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const result = self->data->build();
            ctx.push_value(result);
            return 1;
        }
        static int getTexture(lua_State* const vm) {
            auto const self = as(vm, 1);
            auto const texture = Texture2D::create(vm);
            texture->data = self->data->getTexture();
            if (texture->data) {
                texture->data->retain();
            }
            return 1;
        }

        // static method

        static int create(lua_State* const vm) {
            auto const self = TextLayout::create(vm);
            if (!core::ITextLayout::create(LAPP.GetAppModel()->getDevice(), &self->data)) {
                luaL_error(vm, "create TextLayout failed");
            }
            return 1;
        }
    };

    bool TextLayout::is(lua_State* const vm, int const index) {
        lua::stack_t const ctx(vm);
        return ctx.is_metatable(index, class_name);
    }
    TextLayout* TextLayout::as(lua_State* const vm, int const index) {
        lua::stack_t const ctx(vm);
        return ctx.as_userdata<TextLayout>(index);
    }
    TextLayout* TextLayout::create(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const self = ctx.create_userdata<TextLayout>();
        auto const self_index = ctx.index_of_top();
        ctx.set_metatable(self_index, class_name);
        self->data = nullptr;
        return self;
    }
    void TextLayout::registerClass(lua_State* vm) {
        [[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
        lua::stack_t const ctx(vm);

        // method

        auto const method_table = ctx.create_module(class_name);
        ctx.set_map_value(method_table, "setText", &TextLayoutBinding::setText);
        ctx.set_map_value(method_table, "setFontFamilyName", &TextLayoutBinding::setFontFamilyName);
        ctx.set_map_value(method_table, "setFontSize", &TextLayoutBinding::setFontSize);
        ctx.set_map_value(method_table, "setFontWeight", &TextLayoutBinding::setFontWeight);
        ctx.set_map_value(method_table, "setFontStyle", &TextLayoutBinding::setFontStyle);
        ctx.set_map_value(method_table, "setLayoutSize", &TextLayoutBinding::setLayoutSize);
        ctx.set_map_value(method_table, "setTextAlignment", &TextLayoutBinding::setTextAlignment);
        ctx.set_map_value(method_table, "setParagraphAlignment", &TextLayoutBinding::setParagraphAlignment);
        ctx.set_map_value(method_table, "build", &TextLayoutBinding::build);
        ctx.set_map_value(method_table, "getTexture", &TextLayoutBinding::getTexture);
        ctx.set_map_value(method_table, "create", &TextLayoutBinding::create);

        // metatable

        auto const metatable = ctx.create_metatable(class_name);
        ctx.set_map_value(metatable, "__gc", &TextLayoutBinding::__gc);
        ctx.set_map_value(metatable, "__tostring", &TextLayoutBinding::__tostring);
        ctx.set_map_value(metatable, "__eq", &TextLayoutBinding::__eq);
        ctx.set_map_value(metatable, "__index", method_table);

    #ifdef V
    #undef V
    #endif
    #define V(E) static_cast<std::underlying_type_t<decltype(E)>>(E)

        // FontWeight
        
        auto const font_weight = ctx.create_module("lstg.FontWeight");
        ctx.set_map_value(font_weight, "thin"       , V(core::FontWeight::thin)       );
        ctx.set_map_value(font_weight, "extra_light", V(core::FontWeight::extra_light));
        ctx.set_map_value(font_weight, "light"      , V(core::FontWeight::light)      );
        ctx.set_map_value(font_weight, "normal"     , V(core::FontWeight::normal)     );
        ctx.set_map_value(font_weight, "medium"     , V(core::FontWeight::medium)     );
        ctx.set_map_value(font_weight, "semi_bold"  , V(core::FontWeight::semi_bold)  );
        ctx.set_map_value(font_weight, "bold"       , V(core::FontWeight::bold)       );
        ctx.set_map_value(font_weight, "extra_bold" , V(core::FontWeight::extra_bold) );
        ctx.set_map_value(font_weight, "black"      , V(core::FontWeight::black)      );

        // FontStyle
        
        auto const font_style = ctx.create_module("lstg.FontStyle");
        ctx.set_map_value(font_style, "normal" , V(core::FontStyle::normal) );
        ctx.set_map_value(font_style, "oblique", V(core::FontStyle::oblique));
        ctx.set_map_value(font_style, "italic" , V(core::FontStyle::italic) );

        // TextAlignment
        
        auto const text_alignment = ctx.create_module("lstg.TextAlignment");
        ctx.set_map_value(text_alignment, "start" , V(core::TextAlignment::start) );
        ctx.set_map_value(text_alignment, "center", V(core::TextAlignment::center));
        ctx.set_map_value(text_alignment, "end_"  , V(core::TextAlignment::end)   );

        // ParagraphAlignment

        auto const paragraph_alignment = ctx.create_module("lstg.ParagraphAlignment");
        ctx.set_map_value(paragraph_alignment, "start" , V(core::ParagraphAlignment::start) );
        ctx.set_map_value(paragraph_alignment, "center", V(core::ParagraphAlignment::center));
        ctx.set_map_value(paragraph_alignment, "end_"  , V(core::ParagraphAlignment::end)   );
    }
}
