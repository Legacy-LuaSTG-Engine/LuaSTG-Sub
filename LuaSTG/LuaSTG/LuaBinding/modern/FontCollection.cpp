#include "FontCollection.hpp"
#include "lua/plus.hpp"

namespace luastg::binding {
    const std::string_view FontCollection::class_name{ "lstg.FontCollection" };

    struct FontCollectionBinding : FontCollection {
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
            const lua::stack_t ctx(vm);
            [[maybe_unused]] const auto self = as(vm, 1);
            ctx.push_value(class_name);
            return 1;
        }
        static int __eq(lua_State* const vm) {
            const lua::stack_t ctx(vm);
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

        static int addFile(lua_State* const vm) {
            const lua::stack_t ctx(vm);
            const auto self = as(vm, 1);
            const auto path = ctx.get_value<std::string_view>(1 + 1);
            if (self->data->addFile(path)) {
                ctx.push_value(lua::stack_index_t(1));
            }
            else {
                ctx.push_value(std::nullopt);
            }
            return 1;
        }
        static int build(lua_State* const vm) {
            const lua::stack_t ctx(vm);
            const auto self = as(vm, 1);
            if (self->data->build()) {
                ctx.push_value(lua::stack_index_t(1));
            }
            else {
                ctx.push_value(std::nullopt);
            }
            return 1;
        }

        // static method

        static int create(lua_State* const vm) {
            const auto self = FontCollection::create(vm);
            if (!core::IFontCollection::create(&self->data)) {
                return luaL_error(vm, "create FontCollection failed");
            }
            return 1;
        }
    };

    bool FontCollection::is(lua_State* const vm, const int index) {
        const lua::stack_t ctx(vm);
        return ctx.is_metatable(index, class_name);
    }
    FontCollection* FontCollection::as(lua_State* const vm, const int index) {
        const lua::stack_t ctx(vm);
        return ctx.as_userdata<FontCollection>(index);
    }
    FontCollection* FontCollection::create(lua_State* const vm) {
        const lua::stack_t ctx(vm);
        const auto self = ctx.create_userdata<FontCollection>();
        const auto self_index = ctx.index_of_top();
        ctx.set_metatable(self_index, class_name);
        self->data = nullptr;
        return self;
    }
    void FontCollection::registerClass(lua_State* const vm) {
        [[maybe_unused]] const lua::stack_balancer_t sb(vm);
        const lua::stack_t ctx(vm);

        // method

        const auto method_table = ctx.create_module(class_name);
        ctx.set_map_value(method_table, "addFile", &FontCollectionBinding::addFile);
        ctx.set_map_value(method_table, "build", &FontCollectionBinding::build);
        ctx.set_map_value(method_table, "create", &FontCollectionBinding::create);

        // metatable

        const auto metatable = ctx.create_metatable(class_name);
        ctx.set_map_value(metatable, "__gc", &FontCollectionBinding::__gc);
        ctx.set_map_value(metatable, "__tostring", &FontCollectionBinding::__tostring);
        ctx.set_map_value(metatable, "__eq", &FontCollectionBinding::__eq);
        ctx.set_map_value(metatable, "__index", method_table);
    }
}
