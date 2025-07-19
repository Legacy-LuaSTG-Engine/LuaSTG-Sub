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
	void registerConstants(lua_State* vm);
	void registerConstantsManually(lua_State* vm);

	struct ImVec2Binding {
		static std::string_view const class_name;

		static bool is(lua_State* vm, int index);
		static ImVec2* as(lua_State* vm, int index);
		static ImVec2 const* as(lua_State* vm, int index, ImVec2 const& default_value);
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
	void registerImGuiWidgetsDragSliders(lua_State* vm);
	void registerImGuiWidgetsRegularSliders(lua_State* vm);
	void registerImGuiWidgetsInputWithKeyboardInputText(lua_State* vm);
	void registerImGuiWidgetsInputWithKeyboard(lua_State* vm);
	void registerImGuiWidgetsColorEditorPicker(lua_State* vm);
	void registerImGuiWidgetsTrees(lua_State* vm);
	void registerImGuiWidgetsSelectables(lua_State* vm);
	void registerImGuiMultiSelectionSystem(lua_State* vm);
	void registerImGuiWidgetsListBoxes(lua_State* vm);
	void registerImGuiWidgetsDataPlotting(lua_State* vm);
	void registerImGuiWidgetsValueHelpers(lua_State* vm);
	void registerImGuiWidgetsMenus(lua_State* vm);
	void registerImGuiTooltips(lua_State* vm);
	void registerImGuiPopupsModals(lua_State* vm);
	void registerImGuiTables(lua_State* vm);
	void registerImGuiLegacyColumns(lua_State* vm);
	void registerImGuiTabBarsTabs(lua_State* vm);
	void registerImGuiLoggingCapture(lua_State* vm);
	void registerImGuiDragAndDrop(lua_State* vm);
	void registerImGuiDisabling(lua_State* vm);
	void registerImGuiClipping(lua_State* vm);
	void registerImGuiFocusActivation(lua_State* vm);
	void registerImGuiKeyboardGamepadNavigation(lua_State* vm);
	void registerImGuiOverlappingMode(lua_State* vm);
	void registerImGuiItemWidgetsUtilitiesAndQuery(lua_State* vm);
	void registerImGuiViewports(lua_State* vm);
	void registerImGuiBackgroundForegroundDrawLists(lua_State* vm);
	void registerImGuiMiscellaneousUtilities(lua_State* vm);
	void registerImGuiTextUtilities(lua_State* vm);
	void registerImGuiColorUtilities(lua_State* vm);
	void registerImGuiInputsUtilities(lua_State* vm);
	void registerImGuiClipboardUtilities(lua_State* vm);
	void registerImGuiSettingsIniUtilities(lua_State* vm);
	void registerImGuiDebugUtilities(lua_State* vm);
	void registerImGuiMemoryAllocators(lua_State* vm);

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

	struct ImGuiTextBufferBinding {
		static std::string_view const class_name;

		ImGuiTextBuffer* data{};

		void set(ImGuiTextBuffer* ptr, bool is_reference);
		ImGuiTextBuffer* get();
		[[nodiscard]] bool isReference() const noexcept;

		static bool is(lua_State* vm, int index);
		static ImGuiTextBuffer* as(lua_State* vm, int index);
		static ImGuiTextBuffer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};

	void registerAll(lua_State* vm);
}
