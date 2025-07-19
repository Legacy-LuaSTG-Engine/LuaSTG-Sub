#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

#define FMT_STRING "%.*s"

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
	int TextUnformatted(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const text = ctx.get_value<std::string_view>(1);
		ImGui::TextUnformatted(text.data(), text.data() + text.size());
		return 0;
	}
	int Text(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::Text(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
	int TextColored(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const col = imgui::binding::ImVec4Binding::as(vm, 1);
		auto const s = ctx.get_value<std::string_view>(2);
		ImGui::TextColored(*col, FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
	int TextDisabled(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::TextDisabled(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
	int TextWrapped(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::TextWrapped(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
	int LabelText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const s = ctx.get_value<std::string_view>(2);
		ImGui::LabelText(label.data(), FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
	int BulletText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::BulletText(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
	int SeparatorText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		ImGui::SeparatorText(label.data());
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsText(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "TextUnformatted"sv, &TextUnformatted);
		ctx.set_map_value(m, "_Text"sv, &Text);
		ctx.set_map_value(m, "TextV"sv, &notSupported);
		ctx.set_map_value(m, "_TextColored"sv, &TextColored);
		ctx.set_map_value(m, "TextColoredV"sv, &notSupported);
		ctx.set_map_value(m, "_TextDisabled"sv, &TextDisabled);
		ctx.set_map_value(m, "TextDisabledV"sv, &notSupported);
		ctx.set_map_value(m, "_TextWrapped"sv, &TextWrapped);
		ctx.set_map_value(m, "TextWrappedV"sv, &notSupported);
		ctx.set_map_value(m, "_LabelText"sv, &LabelText);
		ctx.set_map_value(m, "LabelTextV"sv, &notSupported);
		ctx.set_map_value(m, "_BulletText"sv, &BulletText);
		ctx.set_map_value(m, "BulletTextV"sv, &notSupported);
		ctx.set_map_value(m, "SeparatorText"sv, &SeparatorText);

		luaL_dostring(vm, R"(
local select = select
local ImGui = require("imgui.ImGui")
function ImGui.Text(fmt, ...)
    if select("#", ...) > 0 then
    	ImGui._Text(fmt:format(...))
	else
		ImGui._Text(fmt)
	end
end
function ImGui.TextColored(col, fmt, ...)
	if select("#", ...) > 0 then
        ImGui._TextColored(col, fmt:format(...))
	else
	    ImGui._TextColored(col, fmt)
	end
end
function ImGui.TextDisabled(fmt, ...)
    if select("#", ...) > 0 then
        ImGui._TextDisabled(fmt:format(...))
	else
	    ImGui._TextDisabled(fmt)
	end
end
function ImGui.TextWrapped(fmt, ...)
    if select("#", ...) > 0 then
        ImGui._TextWrapped(fmt:format(...))
	else
	    ImGui._TextWrapped(fmt)
	end
end
function ImGui.LabelText(label, fmt, ...)
	if select("#", ...) > 0 then
        ImGui._LabelText(label, fmt:format(...))
	else
	    ImGui._LabelText(label, fmt)
	end
end
function ImGui.BulletText(fmt, ...)
    if select("#", ...) > 0 then
        ImGui._BulletText(fmt:format(...))
	else
	    ImGui._BulletText(fmt)
	end
end
        )");
	}
}
