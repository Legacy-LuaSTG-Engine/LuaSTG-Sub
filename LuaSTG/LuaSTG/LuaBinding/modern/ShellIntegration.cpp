#include "ShellIntegration.hpp"
#include "core/ShellIntegration.hpp"
#include "lua/plus.hpp"

namespace luastg::binding {

	std::string_view const ShellIntegration::class_name{ "lstg.ShellIntegration" };

	struct ShellIntegrationBinding : ShellIntegration {

		// static methods

		static int openFile(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const path = ctx.get_value<std::string_view>(1);
			auto const result = core::ShellIntegration::openFile(path);
			ctx.push_value(result);
			return 1;
		}
		static int openDirectory(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const path = ctx.get_value<std::string_view>(1);
			auto const result = core::ShellIntegration::openDirectory(path);
			ctx.push_value(result);
			return 1;
		}
		static int openUrl(lua_State* const vm) {
			lua::stack_t const ctx(vm);
			auto const url = ctx.get_value<std::string_view>(1);
			auto const result = core::ShellIntegration::openUrl(url);
			ctx.push_value(result);
			return 1;
		}

	};

	void ShellIntegration::registerClass(lua_State* const vm) {
		[[maybe_unused]] lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		ctx.set_map_value(method_table, "openFile", &ShellIntegrationBinding::openFile);
		ctx.set_map_value(method_table, "openDirectory", &ShellIntegrationBinding::openDirectory);
		ctx.set_map_value(method_table, "openUrl", &ShellIntegrationBinding::openUrl);
	}

}
