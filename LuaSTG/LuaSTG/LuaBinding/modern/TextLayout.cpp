#include "TextLayout.hpp"
#include "Vector2.hpp"
#include "Vector4.hpp"
#include "LuaWrapper.hpp"
#include "lua/plus.hpp"

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
        static int setFontCollection(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            // TODO
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setFontFamily(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const font_family_name = ctx.get_value<std::string_view>(1 + 1);
            self->data->setFontFamily(font_family_name);
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
        static int setFontWidth(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const font_width = ctx.get_value<core::FontWidth>(1 + 1);
            self->data->setFontWidth(font_width);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setLayoutSize(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (ctx.is_number(1 + 1)) {
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

        static int setTextColor(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (ctx.is_number(1 + 1)) {
                const auto r = ctx.get_value<float>(1 + 1);
                const auto g = ctx.get_value<float>(1 + 2);
                const auto b = ctx.get_value<float>(1 + 3);
                const auto a = ctx.get_value<float>(1 + 4, 1.0f);
                self->data->setTextColor(core::Vector4F(r, g, b, a));
            }
            else if (Vector4::is(vm, 1 + 1)) {
                const auto color = Vector4::as(vm, 1 + 1);
                self->data->setTextColor(core::Vector4F(
                    static_cast<float>(color->data.x),
                    static_cast<float>(color->data.y),
                    static_cast<float>(color->data.z),
                    static_cast<float>(color->data.w)
                ));
            }
            else {
                const auto color = Color::Cast(vm, 1 + 1);
                self->data->setTextColor(core::Vector4F(
                    color->r / 255.0f,
                    color->g / 255.0f,
                    color->b / 255.0f,
                    color->a / 255.0f
                ));
            }
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setStrokeWidth(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            auto const stroke_width = ctx.get_value<float>(1 + 1);
            self->data->setStrokeWidth(stroke_width);
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }
        static int setStrokeColor(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (ctx.is_number(1 + 1)) {
                const auto r = ctx.get_value<float>(1 + 1);
                const auto g = ctx.get_value<float>(1 + 2);
                const auto b = ctx.get_value<float>(1 + 3);
                const auto a = ctx.get_value<float>(1 + 4, 1.0f);
                self->data->setStrokeColor(core::Vector4F(r, g, b, a));
            }
            else if (Vector4::is(vm, 1 + 1)) {
                const auto color = Vector4::as(vm, 1 + 1);
                self->data->setStrokeColor(core::Vector4F(
                    static_cast<float>(color->data.x),
                    static_cast<float>(color->data.y),
                    static_cast<float>(color->data.z),
                    static_cast<float>(color->data.w)
                ));
            }
            else {
                const auto color = Color::Cast(vm, 1 + 1);
                self->data->setStrokeColor(core::Vector4F(
                    color->r / 255.0f,
                    color->g / 255.0f,
                    color->b / 255.0f,
                    color->a / 255.0f
                ));
            }
            ctx.push_value(lua::stack_index_t(1));
            return 1;
        }

        static int build(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            if (self->data->build()) {
                ctx.push_value(lua::stack_index_t(1));
            }
            else {
                ctx.push_value(std::nullopt);
            }
            return 1;
        }
        static int getVersion(lua_State* const vm) {
            lua::stack_t const ctx(vm);
            auto const self = as(vm, 1);
            const auto version = self->data->getVersion();
            ctx.push_value(version);
            return 1;
        }

        // static method

        static int create(lua_State* const vm) {
            auto const self = TextLayout::create(vm);
            if (!core::ITextLayout::create(&self->data)) {
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
        ctx.set_map_value(method_table, "setFontCollection", &TextLayoutBinding::setFontCollection);
        ctx.set_map_value(method_table, "setFontFamily", &TextLayoutBinding::setFontFamily);
        ctx.set_map_value(method_table, "setFontSize", &TextLayoutBinding::setFontSize);
        ctx.set_map_value(method_table, "setFontWeight", &TextLayoutBinding::setFontWeight);
        ctx.set_map_value(method_table, "setFontStyle", &TextLayoutBinding::setFontStyle);
        ctx.set_map_value(method_table, "setFontWidth", &TextLayoutBinding::setFontWidth);
        ctx.set_map_value(method_table, "setLayoutSize", &TextLayoutBinding::setLayoutSize);
        ctx.set_map_value(method_table, "setTextAlignment", &TextLayoutBinding::setTextAlignment);
        ctx.set_map_value(method_table, "setParagraphAlignment", &TextLayoutBinding::setParagraphAlignment);

        ctx.set_map_value(method_table, "setTextColor", &TextLayoutBinding::setTextColor);
        ctx.set_map_value(method_table, "setStrokeWidth", &TextLayoutBinding::setStrokeWidth);
        ctx.set_map_value(method_table, "setStrokeColor", &TextLayoutBinding::setStrokeColor);

        ctx.set_map_value(method_table, "build", &TextLayoutBinding::build);
        ctx.set_map_value(method_table, "getVersion", &TextLayoutBinding::getVersion);

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

        // FontWidth
        
        auto const font_width = ctx.create_module("lstg.FontWidth");
        ctx.set_map_value(font_width, "ultra_condensed" , V(core::FontWidth::ultra_condensed));
        ctx.set_map_value(font_width, "extra_condensed" , V(core::FontWidth::extra_condensed));
        ctx.set_map_value(font_width, "condensed"       , V(core::FontWidth::condensed)      );
        ctx.set_map_value(font_width, "semi_condensed"  , V(core::FontWidth::semi_condensed) );
        ctx.set_map_value(font_width, "normal"          , V(core::FontWidth::normal)         );
        ctx.set_map_value(font_width, "semi_expanded"   , V(core::FontWidth::semi_expanded)  );
        ctx.set_map_value(font_width, "expanded"        , V(core::FontWidth::expanded)       );
        ctx.set_map_value(font_width, "extra_expanded"  , V(core::FontWidth::extra_expanded) );
        ctx.set_map_value(font_width, "ultra_expanded"  , V(core::FontWidth::ultra_expanded) );

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
