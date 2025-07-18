#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int IsKeyDown(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key = ctx.get_value<ImGuiKey>(1);
		auto const result = ImGui::IsKeyDown(key);
		ctx.push_value(result);
		return 1;
	}
	int IsKeyPressed(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key = ctx.get_value<ImGuiKey>(1);
		auto const repeat = ctx.get_value<bool>(2, true);
		auto const result = ImGui::IsKeyPressed(key, repeat);
		ctx.push_value(result);
		return 1;
	}
	int IsKeyReleased(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key = ctx.get_value<ImGuiKey>(1);
		auto const result = ImGui::IsKeyReleased(key);
		ctx.push_value(result);
		return 1;
	}
	int IsKeyChordPressed(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key_chord = ctx.get_value<ImGuiKeyChord>(1);
		auto const result = ImGui::IsKeyChordPressed(key_chord);
		ctx.push_value(result);
		return 1;
	}
	int GetKeyPressedAmount(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key = ctx.get_value<ImGuiKey>(1);
		auto const repeat_delay = ctx.get_value<float>(2);
		auto const rate = ctx.get_value<float>(3);
		auto const result = ImGui::GetKeyPressedAmount(key, repeat_delay, rate);
		ctx.push_value(result);
		return 1;
	}
	int GetKeyName(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key = ctx.get_value<ImGuiKey>(1);
		auto const result = ImGui::GetKeyName(key);
		ctx.push_value(std::string_view(result));
		return 1;
	}
	int SetNextFrameWantCaptureKeyboard(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const want_capture_keyboard = ctx.get_value<bool>(1);
		ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
		return 0;
	}

	int Shortcut(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key_chord = ctx.get_value<ImGuiKeyChord>(1);
		auto const flags = ctx.get_value<ImGuiInputFlags>(2, 0);
		auto const result = ImGui::Shortcut(key_chord, flags);
		ctx.push_value(result);
		return 1;
	}
	int SetNextItemShortcut(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key_chord = ctx.get_value<ImGuiKeyChord>(1);
		auto const flags = ctx.get_value<ImGuiInputFlags>(2, 0);
		ImGui::SetNextItemShortcut(key_chord, flags);
		return 0;
	}

	int SetItemKeyOwner(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const key = ctx.get_value<ImGuiKey>(1);
		ImGui::SetItemKeyOwner(key);
		return 0;
	}

	int IsMouseDown(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const result = ImGui::IsMouseDown(button);
		ctx.push_value(result);
		return 1;
	}
	int IsMouseClicked(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const repeat = ctx.get_value<bool>(2, false);
		auto const result = ImGui::IsMouseClicked(button, repeat);
		ctx.push_value(result);
		return 1;
	}
	int IsMouseReleased(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const result = ImGui::IsMouseReleased(button);
		ctx.push_value(result);
		return 1;
	}
	int IsMouseDoubleClicked(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const result = ImGui::IsMouseDoubleClicked(button);
		ctx.push_value(result);
		return 1;
	}
	int IsMouseReleasedWithDelay(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const delay = ctx.get_value<float>(2);
		auto const result = ImGui::IsMouseReleasedWithDelay(button, delay);
		ctx.push_value(result);
		return 1;
	}
	int GetMouseClickedCount(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const result = ImGui::GetMouseClickedCount(button);
		ctx.push_value(result);
		return 1;
	}
	int IsMouseHoveringRect(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const r_min = imgui::binding::ImVec2Binding::as(vm, 1);
		auto const r_max = imgui::binding::ImVec2Binding::as(vm, 2);
		auto const clip = ctx.get_value<bool>(3, true);
		auto const result = ImGui::IsMouseHoveringRect(*r_min, *r_max, clip);
		ctx.push_value(result);
		return 1;
	}
	int IsMousePosValid(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto const mouse_pos = imgui::binding::ImVec2Binding::as(vm, 1);
			auto const result = ImGui::IsMousePosValid(mouse_pos);
			ctx.push_value(result);
		}
		else {
			auto const result = ImGui::IsMousePosValid();
			ctx.push_value(result);
		}
		return 1;
	}
	int IsAnyMouseDown(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::IsAnyMouseDown();
		ctx.push_value(result);
		return 1;
	}
	int GetMousePos(lua_State* const vm) {
		auto const result = ImGui::GetMousePos();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int GetMousePosOnOpeningCurrentPopup(lua_State* const vm) {
		auto const result = ImGui::GetMousePosOnOpeningCurrentPopup();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int IsMouseDragging(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1);
		auto const lock_threshold = ctx.get_value<float>(2, -1.0f);
		auto const result = ImGui::IsMouseDragging(button, lock_threshold);
		ctx.push_value(result);
		return 1;
	}
	int GetMouseDragDelta(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1, 0);
		auto const lock_threshold = ctx.get_value<float>(2, -1.0f);
		auto const result = ImGui::GetMouseDragDelta(button, lock_threshold);
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int ResetMouseDragDelta(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const button = ctx.get_value<ImGuiMouseButton>(1, 0);
		ImGui::ResetMouseDragDelta(button);
		return 0;
	}
	int GetMouseCursor(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetMouseCursor();
		ctx.push_value(result);
		return 1;
	}
	int SetMouseCursor(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const cursor_type = ctx.get_value<ImGuiMouseCursor>(1);
		ImGui::SetMouseCursor(cursor_type);
		return 0;
	}
	int SetNextFrameWantCaptureMouse(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const want_capture_mouse = ctx.get_value<bool>(1);
		ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiInputsUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);

		ctx.set_map_value(m, "IsKeyDown"sv, &IsKeyDown);
		ctx.set_map_value(m, "IsKeyPressed"sv, &IsKeyPressed);
		ctx.set_map_value(m, "IsKeyReleased"sv, &IsKeyReleased);
		ctx.set_map_value(m, "IsKeyChordPressed"sv, &IsKeyChordPressed);
		ctx.set_map_value(m, "GetKeyPressedAmount"sv, &GetKeyPressedAmount);
		ctx.set_map_value(m, "GetKeyName"sv, &GetKeyName);
		ctx.set_map_value(m, "SetNextFrameWantCaptureKeyboard"sv, &SetNextFrameWantCaptureKeyboard);

		ctx.set_map_value(m, "Shortcut"sv, &Shortcut);
		ctx.set_map_value(m, "SetNextItemShortcut"sv, &SetNextItemShortcut);

		ctx.set_map_value(m, "SetItemKeyOwner"sv, &SetItemKeyOwner);

		ctx.set_map_value(m, "IsMouseDown"sv, &IsMouseDown);
		ctx.set_map_value(m, "IsMouseClicked"sv, &IsMouseClicked);
		ctx.set_map_value(m, "IsMouseReleased"sv, &IsMouseReleased);
		ctx.set_map_value(m, "IsMouseDoubleClicked"sv, &IsMouseDoubleClicked);
		ctx.set_map_value(m, "IsMouseReleasedWithDelay"sv, &IsMouseReleasedWithDelay);
		ctx.set_map_value(m, "GetMouseClickedCount"sv, &GetMouseClickedCount);
		ctx.set_map_value(m, "IsMouseHoveringRect"sv, &IsMouseHoveringRect);
		ctx.set_map_value(m, "IsMousePosValid"sv, &IsMousePosValid);
		ctx.set_map_value(m, "IsAnyMouseDown"sv, &IsAnyMouseDown);
		ctx.set_map_value(m, "GetMousePos"sv, &GetMousePos);
		ctx.set_map_value(m, "GetMousePosOnOpeningCurrentPopup"sv, &GetMousePosOnOpeningCurrentPopup);
		ctx.set_map_value(m, "IsMouseDragging"sv, &IsMouseDragging);
		ctx.set_map_value(m, "GetMouseDragDelta"sv, &GetMouseDragDelta);
		ctx.set_map_value(m, "ResetMouseDragDelta"sv, &ResetMouseDragDelta);
		ctx.set_map_value(m, "GetMouseCursor"sv, &GetMouseCursor);
		ctx.set_map_value(m, "SetMouseCursor"sv, &SetMouseCursor);
		ctx.set_map_value(m, "SetNextFrameWantCaptureMouse"sv, &SetNextFrameWantCaptureMouse);
	}
}
