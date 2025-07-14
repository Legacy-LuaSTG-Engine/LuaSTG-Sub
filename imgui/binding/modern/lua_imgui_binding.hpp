#pragma once
#include <string_view>
#include <string>
#include "lua.hpp"
#ifndef IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"

namespace imgui::binding {
	extern std::string_view const module_name;

	void registerGlobal(lua_State* vm);

	struct ImVec2Binding {
		static std::string_view const class_name;

		static bool is(lua_State* vm, int index);
		static ImVec2* as(lua_State* vm, int index);
		static ImVec2* create(lua_State* vm);
		static ImVec2* create(lua_State* vm, ImVec2 const& value);
		static void registerClass(lua_State* vm);
	};

	struct ImVec4Binding {
		static std::string_view const class_name;

		static bool is(lua_State* vm, int index);
		static ImVec4* as(lua_State* vm, int index);
		static ImVec4* create(lua_State* vm);
		static ImVec4* create(lua_State* vm, ImVec4 const& value);
		static void registerClass(lua_State* vm);
	};

	struct ImTextureIDUtils {
		static std::string marshal(ImTextureID texture_id);
		static ImTextureID unmarshal(std::string_view value);
	};

	struct ImTextureRefBinding {
		static std::string_view const class_name;

		static bool is(lua_State* vm, int index);
		static ImTextureRef* as(lua_State* vm, int index);
		static ImTextureRef* create(lua_State* vm);
		static ImTextureRef* create(lua_State* vm, ImTextureRef const& value);
		static void registerClass(lua_State* vm);
	};

	extern std::string_view const module_ImGui_name;
	void registerImGuiContextCreationAndAccess(lua_State* vm);
	void registerImGuiMain(lua_State* vm);
	void registerImGuiDemoDebugInformation(lua_State* vm);
	void registerImGuiStyles(lua_State* vm);
	void registerImGuiWindows(lua_State* vm);
	void registerImGuiChildWindows(lua_State* vm);
	void registerImGuiWindowsUtilities(lua_State* vm);
	void beginSetNextWindowSizeConstraintsCallbackWrapper(lua_State* vm);
	void endSetNextWindowSizeConstraintsCallbackWrapper(lua_State* vm);
	void registerImGuiWindowManipulation(lua_State* vm);
	void registerImGuiWindowsScrolling(lua_State* vm);
	void registerImGuiParametersStacksFont(lua_State* vm);
	void registerImGuiParametersStacksShared(lua_State* vm);
	void registerImGuiParametersStacksCurrentWindow(lua_State* vm);
	void registerImGuiStyleReadAccess(lua_State* vm);
	void registerImGuiLayoutCursorPositioning(lua_State* vm);
	void registerImGuiOtherLayoutFunctions(lua_State* vm);
	void registerImGuiIdStackScopes(lua_State* vm);
	void registerImGuiWidgetsText(lua_State* vm);
	void registerImGuiWidgetsMain(lua_State* vm);
	void registerImGuiWidgetsImages(lua_State* vm);
	void registerImGuiWidgetsComboBoxDropdown(lua_State* vm);

	struct ImGuiStyleBinding {
		static std::string_view const class_name;

		ImGuiStyle* data{};

		void set(ImGuiStyle* ptr, bool is_reference);
		ImGuiStyle* get();
		[[nodiscard]] bool isReference() const noexcept;

		static bool is(lua_State* vm, int index);
		static ImGuiStyle* as(lua_State* vm, int index);
		static ImGuiStyle* create(lua_State* vm);
		static ImGuiStyle* create(lua_State* vm, ImGuiStyle const& value);
		static ImGuiStyle* reference(lua_State* vm, ImGuiStyle* value);
		static void registerClass(lua_State* vm);
	};

	void registerAll(lua_State* vm);
}
