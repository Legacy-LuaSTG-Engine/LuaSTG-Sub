#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

#define FMT_STRING "%.*s"

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
	int TreeNode(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto result = false;
		if (ctx.index_of_top() >= 2) {
			auto const s = ctx.get_value<std::string_view>(2);
			if (ctx.is_string(1)) {
				auto const str_id = ctx.get_value<std::string_view>(1);
				result = ImGui::TreeNode(str_id.data(), FMT_STRING, static_cast<int>(s.size()), s.data());
			}
			else if (ctx.is_userdata(1) || ctx.is_light_userdata(1)) {
				auto const ptr_id = lua_touserdata(vm, 1);
				result = ImGui::TreeNode(ptr_id, FMT_STRING, static_cast<int>(s.size()), s.data());
			}
		}
		else {
			auto const label = ctx.get_value<std::string_view>(1);
			result = ImGui::TreeNode(label.data());
		}
		ctx.push_value(result);
		return 1;
	}
	int TreeNodeEx(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto result = false;
		if (ctx.index_of_top() >= 3) {
			auto const flags = ctx.get_value<ImGuiTreeNodeFlags>(2);
			auto const s = ctx.get_value<std::string_view>(3);
			if (ctx.is_string(1)) {
				auto const str_id = ctx.get_value<std::string_view>(1);
				result = ImGui::TreeNodeEx(str_id.data(), flags, FMT_STRING, static_cast<int>(s.size()), s.data());
			}
			else if (ctx.is_userdata(1) || ctx.is_light_userdata(1)) {
				auto const ptr_id = lua_touserdata(vm, 1);
				result = ImGui::TreeNodeEx(ptr_id, flags, FMT_STRING, static_cast<int>(s.size()), s.data());
			}
		}
		else {
			auto const label = ctx.get_value<std::string_view>(1);
			auto const flags = ctx.get_value<ImGuiTreeNodeFlags>(2, 0);
			result = ImGui::TreeNodeEx(label.data(), flags);
		}
		ctx.push_value(result);
		return 1;
	}
	int TreePush(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_string(1)) {
			auto const str_id = ctx.get_value<std::string_view>(1);
			ImGui::TreePush(str_id.data());
		}
		else if (ctx.is_userdata(1) || ctx.is_light_userdata(1)) {
			auto const ptr_id = lua_touserdata(vm, 1);
			ImGui::TreePush(ptr_id);
		}
		return 0;
	}
	int TreePop(lua_State*) {
		ImGui::TreePop();
		return 0;
	}
	int GetTreeNodeToLabelSpacing(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetTreeNodeToLabelSpacing();
		ctx.push_value(result);
		return 1;
	}
	int CollapsingHeader(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		if (ctx.is_boolean(2)) {
			auto p_visible = ctx.get_value<bool>(2);
			auto const flags = ctx.get_value<ImGuiTreeNodeFlags>(3, 0);
			auto const result = ImGui::CollapsingHeader(label.data(), &p_visible, flags);
			ctx.push_value(result);
			ctx.push_value(p_visible);
			return 2;
		}
		auto const flags = ctx.get_value<ImGuiTreeNodeFlags>(2, 0);
		auto const result = ImGui::CollapsingHeader(label.data(), flags);
		ctx.push_value(result);
		return 1;
	}
	int SetNextItemOpen(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const is_open = ctx.get_value<bool>(1);
		auto const cond = ctx.get_value<ImGuiCond>(2, 0);
		ImGui::SetNextItemOpen(is_open, cond);
		return 0;
	}
	int SetNextItemStorageID(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const storage_id = ctx.get_value<ImGuiID>(1);
		ImGui::SetNextItemStorageID(storage_id);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsTrees(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "_TreeNode"sv, &TreeNode);
		ctx.set_map_value(m, "TreeNodeV"sv, &notSupported);
		ctx.set_map_value(m, "_TreeNodeEx"sv, &TreeNodeEx);
		ctx.set_map_value(m, "TreeNodeExV"sv, &notSupported);
		ctx.set_map_value(m, "TreePush"sv, &TreePush);
		ctx.set_map_value(m, "TreePop"sv, &TreePop);
		ctx.set_map_value(m, "GetTreeNodeToLabelSpacing"sv, &GetTreeNodeToLabelSpacing);
		ctx.set_map_value(m, "CollapsingHeader"sv, &CollapsingHeader);
		ctx.set_map_value(m, "SetNextItemOpen"sv, &SetNextItemOpen);
		ctx.set_map_value(m, "SetNextItemStorageID"sv, &SetNextItemStorageID);

		luaL_dostring(vm, R"(
local select = select
local ImGui = require("imgui.ImGui")
function ImGui.TreeNode(id, fmt, ...)
	if select("#", ...) > 0 then
		return ImGui._TreeNode(id, fmt:format(...))
	elseif fmt then
		return ImGui._TreeNode(id, fmt)
	else
		return ImGui._TreeNode(id)
	end
end
function ImGui.TreeNodeEx(id, flags, fmt, ...)
	if select("#", ...) > 0 then
		return ImGui._TreeNodeEx(id, flags, fmt:format(...))
	elseif fmt then
		return ImGui._TreeNodeEx(id, flags, fmt)
	else
		return ImGui._TreeNodeEx(id, flags)
	end
end
		)");
	}
}
