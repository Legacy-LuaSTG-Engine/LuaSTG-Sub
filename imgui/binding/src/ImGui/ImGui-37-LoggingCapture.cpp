#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

#define FMT_STRING "%.*s"

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}

	int LogToTTY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const auto_open_depth = ctx.get_value<int>(1, -1);
		ImGui::LogToTTY(auto_open_depth);
		return 0;
	}
	int LogToFile(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const auto_open_depth = ctx.get_value<int>(1, -1);
		auto const filename = ctx.get_value<std::string_view>(2, {});
		ImGui::LogToFile(auto_open_depth, filename.data());
		return 0;
	}
	int LogToClipboard(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const auto_open_depth = ctx.get_value<int>(1, -1);
		ImGui::LogToClipboard(auto_open_depth);
		return 0;
	}
	int LogFinish(lua_State*) {
		ImGui::LogFinish();
		return 0;
	}
	int LogButtons(lua_State*) {
		ImGui::LogButtons();
		return 0;
	}
	int LogText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::LogText(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiLoggingCapture(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "LogToTTY"sv, &LogToTTY);
		ctx.set_map_value(m, "LogToFile"sv, &LogToFile);
		ctx.set_map_value(m, "LogToClipboard"sv, &LogToClipboard);
		ctx.set_map_value(m, "LogFinish"sv, &LogFinish);
		ctx.set_map_value(m, "LogButtons"sv, &LogButtons);
		ctx.set_map_value(m, "_LogText"sv, &LogText);
		ctx.set_map_value(m, "LogTextV"sv, &notSupported);

		luaL_dostring(vm, R"(
local select = select
local ImGui = require("imgui.ImGui")
function ImGui.LogText(fmt, ...)
	if select("#", ...) > 0 then
		ImGui._LogText(fmt:format(...))
	else
		ImGui._LogText(fmt)
	end
end
		)");
	}
}
