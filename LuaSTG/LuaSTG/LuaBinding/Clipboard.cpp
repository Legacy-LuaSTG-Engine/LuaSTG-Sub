#include "Clipboard.hpp"
#include "core/Clipboard.hpp"
#include "lua/plus.hpp"

namespace luastg::binding {

	std::string_view const Clipboard::class_name{ "lstg.Clipboard" };

	struct ClipboardBinding : Clipboard {

		// static methods

		static int hasText(lua_State* vm) {
			lua::stack_t const ctx(vm);
			ctx.push_value(core::Clipboard::hasText());
			return 1;
		}
		static int getText(lua_State* vm) {
			lua::stack_t const ctx(vm);
			std::string buffer;
			if (core::Clipboard::getText(buffer)) {
				ctx.push_value(buffer);
				ctx.push_value(std::nullopt);
			}
			else {
				ctx.push_value(std::nullopt);
				ctx.push_value(std::nullopt);
			}
			return 2;
		}
		static int setText(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const text = ctx.get_value<std::string_view>(1);
			ctx.push_value(core::Clipboard::setText(text));
			return 1;
		}

	};

	void Clipboard::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t sb(vm);
		lua::stack_t ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "hasText", &ClipboardBinding::hasText);
		ctx.set_map_value(method_table, "getText", &ClipboardBinding::getText);
		ctx.set_map_value(method_table, "setText", &ClipboardBinding::setText);
	}

}
